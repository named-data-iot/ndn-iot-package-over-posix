/*
 * Copyright (C) 2019 Yiran Lei
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3.0. See the file LICENSE in the top level
 * directory for more details.
 *
 * See AUTHORS.md for complete list of NDN IOT PKG authors and contributors.
 */
/*
 * This file-tranfer-client works with file-transfer-server.
 * Launch the file-transfer-server, input local port, client ip, client port and name.
 * Launch the file-transfer-client, input local port, server ip, server port, name and the file name.
 * The server will then return the requested file to the client. (if the file exists in the directory)
 */

#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ndn-lite.h>
#include "ndn-lite/encode/name.h"
#include "ndn-lite/encode/data.h"
#include "ndn-lite/encode/interest.h"
#include "ndn-lite/encode/signed-interest.h"
#include "ndn-lite/app-support/ndn-sig-verifier.h"

in_port_t port1, port2;
in_addr_t server_ip;
char * file_name;
ndn_name_t name_prefix;
uint8_t buf[4096];
uint8_t anchor_bytes[2048];
uint32_t anchor_bytes_size;
bool running;

uint8_t secp256r1_prv_key_str[] = {
  0xf9,'.',0xbc,0xc5,0xb1,0xcb,0x8c,0x08,
  'G','b','c','-','L',0xf1,0x96,0xcc,
  'r',0xed,0x91,0xaf,0x9e,'!',0x02,0xf2,
  0xef,'h','L',0xbc,'r',0xf5,'l',0x01
};

uint8_t secp256r1_pub_key_str[64] = {
  0x90,0xa6,0xbc,0xe8,0x00,'W',0xc0,'e',
  0xe9,0x8a,'\\',0x05,'(','d',0x9a,0x99,
  'y',0xc1,0x10,0x0f,0xf8,0x8a,0xd0,'I',
  'U',0xaa,0xbf,0xbb,0x1b,'\\',0xe2,0xab,
  '9','W',0x89,0x96,0xb5,0xee,':',0xf9,
  '_',0xd3,0x89,0x15,0xdc,3,0x7f,'g',
  0xca,'R','b','\t',0xbe,0x88,'Y',0xe2,
  0xbc,0xcf,0xbd,0xd4,0x18,0xdd,'8',0x01
};

int parseArgs(int argc, char *argv[]){
  char *sz_port1, *sz_port2, *sz_addr;
  uint32_t ul_port;
  struct hostent * host_addr;
  struct in_addr ** paddrs;

  if(argc < 6){
    fprintf(stderr, "ERROR: wrong arguments.\n");
    printf("Usage: <local-port> <remote-ip> <remote-port> <name-prefix> <file-name>\n");
    return 1;
  }
  sz_port1 = argv[1];
  sz_addr = argv[2];
  sz_port2 = argv[3];

  if(strlen(sz_port1) <= 0 || strlen(sz_addr) <= 0 || strlen(sz_port2) <= 0){
    fprintf(stderr, "ERROR: wrong arguments.\n");
    return 1;
  }

  host_addr = gethostbyname(sz_addr);
  if(host_addr == NULL){
    fprintf(stderr, "ERROR: wrong hostname.\n");
    return 2;
  }

  paddrs = (struct in_addr **)host_addr->h_addr_list;
  if(paddrs[0] == NULL){
    fprintf(stderr, "ERROR: wrong hostname.\n");
    return 2;
  }
  server_ip = paddrs[0]->s_addr;

  ul_port = strtoul(sz_port1, NULL, 10);
  if(ul_port < 1024 || ul_port >= 65536){
    fprintf(stderr, "ERROR: wrong port number.\n");
    return 3;
  }
  port1 = htons((uint16_t) ul_port);

  ul_port = strtoul(sz_port2, NULL, 10);
  if(ul_port < 1024 || ul_port >= 65536){
    fprintf(stderr, "ERROR: wrong port number.\n");
    return 3;
  }
  port2 = htons((uint16_t) ul_port);

  if(ndn_name_from_string(&name_prefix, argv[4], strlen(argv[4])) != NDN_SUCCESS){
    fprintf(stderr, "ERROR: wrong name.\n");
    return 4;
  }

  file_name = argv[5];
  return 0;
}


void on_data(const uint8_t* rawdata, uint32_t data_size, void* userdata){
  ndn_data_t data;
  printf("Receiving data\n");
  // char data_buf[1024];
  char* data_buf;
  int data_off;
  tlv_parse_data(rawdata,data_size,2,TLV_DATAARG_CONTENT_BUF,(uint8_t**)&data_buf,TLV_DATAARG_CONTENT_SIZE,&data_off);
  //printf("data\n%s\n",data_buf);
  save_file(data_buf);
}

int save_file (uint8_t* file_data){
  FILE * fp = fopen(file_name,"w");
  if(fp == NULL){
    fprintf(stderr, "ERROR: fail to open a file when writing.\n");
    return 1;
  }
  if(fputs((char*)file_data,fp) == EOF){
    fprintf(stderr, "ERROR: fail to write data.\n");
    return 1;
  }
  fclose(fp);
  return 0;
}

void on_timeout(void* userdata){
  printf("On timeout\n");
  running = false;
}

int main(int argc, char *argv[]){
  ndn_udp_face_t *face;
  ndn_interest_t interest;
  ndn_encoder_t encoder;
  int ret;

  if((ret = parseArgs(argc, argv)) != 0){
    return ret;
  }

  ndn_lite_startup();

  // simulate bootstrapping process
  ndn_ecc_prv_t anchor_prv_key;
  ndn_ecc_prv_init(&anchor_prv_key, secp256r1_prv_key_str, sizeof(secp256r1_prv_key_str),
                   NDN_ECDSA_CURVE_SECP256R1, 123);
  ndn_data_t anchor;
  ndn_data_init(&anchor);
  ndn_name_from_string(&anchor.name, "/ndn-iot/controller/KEY", strlen("/ndn-iot/controller/KEY"));
  ndn_name_append_keyid(&anchor.name, 123);
  ndn_name_t key_name;
  memcpy(&key_name, &anchor.name, sizeof(ndn_name_t));
  ndn_name_append_string_component(&anchor.name, "self", strlen("self"));
  ndn_name_append_keyid(&anchor.name, 456);
  ndn_data_set_content(&anchor, secp256r1_pub_key_str, sizeof(secp256r1_pub_key_str));
  encoder_init(&encoder, anchor_bytes, sizeof(anchor_bytes));
  ndn_data_tlv_encode_ecdsa_sign(&encoder, &anchor, &key_name, &anchor_prv_key);
  anchor_bytes_size = encoder.offset;
  ndn_data_tlv_decode_no_verify(&anchor, encoder.output_value, encoder.offset, NULL, NULL);
  ndn_key_storage_set_trust_anchor(&anchor);

  // ndn_ecc_prv_t self_prv_key
  ndn_ecc_pub_t* self_pub;
  ndn_ecc_prv_t* self_prv;
  ndn_key_storage_get_empty_ecc_key(&self_pub, &self_prv);
  ndn_ecc_make_key(self_pub, self_prv, NDN_ECDSA_CURVE_SECP256R1, 234);

  // self cert
  ndn_data_t self_cert;
  ndn_data_init(&self_cert);
  ndn_name_from_string(&self_cert.name, "/ndn-iot/bedroom/file-client/KEY", strlen("/ndn-iot/bedroom/file-server/KEY"));
  ndn_name_append_keyid(&self_cert.name, 234);
  ndn_name_append_string_component(&self_cert.name, "home", strlen("home"));
  ndn_name_append_keyid(&self_cert.name, 567);
  ndn_data_set_content(&self_cert, ndn_ecc_get_pub_key_value(self_pub),
                       ndn_ecc_get_pub_key_size(self_pub));
  encoder_init(&encoder, anchor_bytes, sizeof(anchor_bytes));
  ndn_data_tlv_encode_ecdsa_sign(&encoder, &self_cert, &key_name, &anchor_prv_key);
  ndn_data_tlv_decode_no_verify(&self_cert, encoder.output_value, encoder.offset, NULL, NULL);
  ndn_key_storage_set_self_identity(&self_cert, self_prv);

  // set up route
  face = ndn_udp_unicast_face_construct(INADDR_ANY, port1, server_ip, port2);
  running = true;
  encoder_init(&encoder, buf, 4096);
  ndn_name_tlv_encode(&encoder, &name_prefix);
  ndn_forwarder_add_route(&face->intf, buf, encoder.offset);

  // set up sig verifier
  ndn_sig_verifier_init(&face->intf);

  char interest_buf[4096];
  int interest_off;
  ndn_key_storage_t* storage = ndn_key_storage_get_instance();
  ndn_interest_t request;
  ndn_interest_from_name(&request, &name_prefix);
  ndn_interest_set_Parameters(&request, (uint8_t*)file_name, strlen(file_name));
  ndn_signed_interest_ecdsa_sign(&request, &storage->self_identity, self_prv);
  // tlv_make_interest(interest_buf,4096,&interest_off,6,TLV_INTARG_NAME_PTR,&name_prefix,
  //                   TLV_INTARG_PARAMS_BUF,(uint8_t*)file_name,TLV_INTARG_PARAMS_SIZE,strlen(file_name),
  //                   TLV_INTARG_SIGTYPE_U8, NDN_SIG_TYPE_ECDSA_SHA256, TLV_INTARG_SIGKEY_PTR, self_prv,
  //                   TLV_INTARG_IDENTITYNAME_PTR, &storage->self_identity);
  encoder_init(&encoder, interest_buf, 4096);
  ndn_interest_tlv_encode(&encoder, &request);
  ndn_forwarder_express_interest(interest_buf, encoder.offset, on_data, on_timeout, NULL);

  while(running){
    ndn_forwarder_process();
    usleep(10000);
  }

  ndn_face_destroy(&face->intf);

  return 0;
}
