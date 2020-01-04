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
 * This file-tranfer-server works with file-transfer-client. 
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
#include "ndn-lite/app-support/ndn-sig-verifier.h"

uint8_t secp256r1_prv_key_str[32] = {
0xA7, 0x58, 0x4C, 0xAB, 0xD3, 0x82, 0x82, 0x5B, 0x38, 0x9F, 0xA5, 0x45, 0x73, 0x00, 0x0A, 0x32,
0x42, 0x7C, 0x12, 0x2F, 0x42, 0x4D, 0xB2, 0xAD, 0x49, 0x8C, 0x8D, 0xBF, 0x80, 0xC9, 0x36, 0xB5
};
uint8_t secp256r1_pub_key_str[64] = {
0x99, 0x26, 0xD6, 0xCE, 0xF8, 0x39, 0x0A, 0x05, 0xD1, 0x8C, 0x10, 0xAE, 0xEF, 0x3C, 0x2A, 0x3C,
0x56, 0x06, 0xC4, 0x46, 0x0C, 0xE9, 0xE5, 0xE7, 0xE6, 0x04, 0x26, 0x43, 0x13, 0x8A, 0x3E, 0xD4,
0x6E, 0xBE, 0x0F, 0xD2, 0xA2, 0x05, 0x0F, 0x00, 0xAC, 0x6F, 0x5D, 0x4B, 0x29, 0x77, 0x2D, 0x54,
0x32, 0x27, 0xDC, 0x05, 0x77, 0xA7, 0xDC, 0xE0, 0xA2, 0x69, 0xC8, 0x8B, 0x4C, 0xBF, 0x25, 0xF2
};

in_port_t port1, port2;
in_addr_t client_ip;
ndn_name_t name_prefix;
uint8_t buf[4096];
uint8_t anchor_bytes[2048];
uint32_t anchor_bytes_size;
ndn_udp_face_t *face;
bool running;

int parseArgs(int argc, char *argv[]){
  char *sz_port1, *sz_port2, *sz_addr;
  uint32_t ul_port;
  struct hostent * host_addr;
  struct in_addr ** paddrs;

  if(argc < 5){
    fprintf(stderr, "ERROR: wrong arguments.\n");
    printf("Usage: <local-port> <client-ip> <client-port> <name-prefix>\n");
    return 1;
  }
  sz_port1 = argv[1];
  sz_addr = argv[2];
  sz_port2 = argv[3];
  //sz_prefix = argv[4];
  //data_need = argv[5];

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
  client_ip = paddrs[0]->s_addr;

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
  return 0;
}

void
on_success(ndn_interest_t* interest)
{
  printf("verify succeed");
  char* file_name = interest->parameters.value;
  int param_size = interest->parameters.size;
  // tlv_parse_interest(interest,interest_size,3,
  //                    TLV_INTARG_NAME_PTR,&ek_name,TLV_INTARG_PARAMS_BUF,(uint8_t**)&file_name,
  //                    TLV_INTARG_PARAMS_SIZE,&param_size);
  file_name[param_size] = '\0';

  char temp_buffer[1024];
  FILE *fp = fopen(file_name,"r");
  printf("The requested file name is: %s\nlength is %d\n",file_name,param_size);
  if(fp == NULL){
    fprintf(stderr, "ERROR: fail to open file.\n");
    return;
  }
  if(fgets(temp_buffer,1024,fp) == NULL){
    fprintf(stderr, "ERROR: fail to read file.\n");
    return;
  }
  //printf("The content of the file is: %s, %lu\n",temp_buffer,strlen(temp_buffer) );

  uint8_t data_buf[4096];
  int data_off;
  tlv_make_data(data_buf,4096,&data_off,3,TLV_DATAARG_NAME_PTR,&interest->name,TLV_DATAARG_CONTENT_BUF,(uint8_t*)temp_buffer,TLV_DATAARG_CONTENT_SIZE,strlen(temp_buffer));
  ndn_forwarder_put_data(data_buf,data_off);
  return;
}

void
on_failure(ndn_interest_t* interest)
{
  printf("Cannot verify");
}

int on_interest(const uint8_t* interest, uint32_t interest_size, void* userdata){
  ndn_data_t data;
  ndn_encoder_t encoder;

  printf("On interest\n");
  ndn_sig_verifier_verify_int(interest, interest_size, on_success, NULL, on_failure, NULL);
}


int main(int argc, char *argv[]){
  int ret;
  ndn_encoder_t encoder;

  if((ret = parseArgs(argc, argv)) != 0){
    return ret;
  }

  ndn_lite_startup();
  face = ndn_udp_unicast_face_construct(INADDR_ANY, port1, client_ip, port2);

  // simulate bootstrapping process
  ndn_ecc_prv_t anchor_prv_key;
  ndn_ecc_prv_init(&anchor_prv_key, secp256r1_prv_key_str, sizeof(secp256r1_prv_key_str),
                   NDN_ECDSA_CURVE_SECP256R1, 123);
  ndn_ecc_pub_t anchor_pub_key;
  ndn_ecc_pub_init(&anchor_pub_key, secp256r1_pub_key_str, sizeof(secp256r1_pub_key_str), NDN_ECDSA_CURVE_SECP256R1, 123);

  // test
  // ndn_ecc_make_key(&anchor_pub_key, &anchor_prv_key, NDN_ECDSA_CURVE_SECP256R1, 123);
  // uint8_t* starting = ndn_ecc_get_pub_key_value(&anchor_pub_key);
  // for (int i = 0; i < ndn_ecc_get_pub_key_size(&anchor_pub_key); i++) {
  //   // printf()
  //   fprintf(stdout, "0x%02X%s",
  //   *(starting + i),
  //   ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
  // }
  // printf("\n\n");
  // starting = &anchor_prv_key.abs_key.key_value;
  // for (int i = 0; i < ndn_ecc_get_prv_key_size(&anchor_prv_key); i++) {
  //   // printf()
  //   fprintf(stdout, "0x%02X%s",
  //   *(starting + i),
  //   ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
  // }

  ndn_data_t anchor;
  ndn_data_init(&anchor);
  ndn_name_from_string(&anchor.name, "/ndn-iot/controller/KEY", strlen("/ndn-iot/controller/KEY"));
  ndn_name_t anchor_id;
  memcpy(&anchor_id, &anchor.name, sizeof(ndn_name_t));
  anchor_id.components_size -= 1;
  ndn_name_append_keyid(&anchor.name, 123);
  ndn_name_append_string_component(&anchor.name, "self", strlen("self"));
  ndn_name_append_keyid(&anchor.name, 456);
  ndn_data_set_content(&anchor, secp256r1_pub_key_str, sizeof(secp256r1_pub_key_str));
  encoder_init(&encoder, anchor_bytes, sizeof(anchor_bytes));
  ndn_data_tlv_encode_ecdsa_sign(&encoder, &anchor, &anchor_id, &anchor_prv_key);
  anchor_bytes_size = encoder.offset;
  ndn_data_tlv_decode_no_verify(&anchor, encoder.output_value, encoder.offset, NULL, NULL);
  ndn_key_storage_set_trust_anchor(&anchor);

  // test key pair
  // ndn_encoder_t encoder2;
  // encoder_init(&encoder2, buf, sizeof(buf));
  // ndn_data_tlv_encode(&encoder2, &anchor);
  // ndn_data_t temp_data;
  // int suc = ndn_data_tlv_decode_ecdsa_verify(&temp_data, encoder2.output_value, encoder2.offset, &anchor_pub_key);
  // if (suc == 0) {
  //   printf("key pair works fine");
  // }

  // ndn_ecc_prv_t self_prv_key
  ndn_ecc_pub_t* self_pub = NULL;
  ndn_ecc_prv_t* self_prv = NULL;
  ndn_key_storage_get_empty_ecc_key(&self_pub, &self_prv);
  ndn_ecc_make_key(self_pub, self_prv, NDN_ECDSA_CURVE_SECP256R1, 234);

  // self cert
  ndn_data_t self_cert;
  ndn_data_init(&self_cert);
  ndn_name_from_string(&self_cert.name, "/ndn-iot/bedroom/file-server/KEY", strlen("/ndn-iot/bedroom/file-server/KEY"));
  ndn_name_append_keyid(&self_cert.name, 234);
  ndn_name_append_string_component(&self_cert.name, "home", strlen("home"));
  ndn_name_append_keyid(&self_cert.name, 567);
  ndn_data_set_content(&self_cert, ndn_ecc_get_pub_key_value(self_pub),
                       ndn_ecc_get_pub_key_size(self_pub));
  encoder_init(&encoder, anchor_bytes, sizeof(anchor_bytes));
  ndn_data_tlv_encode_ecdsa_sign(&encoder, &self_cert, &anchor_id, &anchor_prv_key);
  ndn_data_tlv_decode_no_verify(&self_cert, encoder.output_value, encoder.offset, NULL, NULL);
  ndn_key_storage_set_self_identity(&self_cert, self_prv);

  // set up sig verifier
  ndn_sig_verifier_after_bootstrapping(&face->intf);

  running = true;
  encoder_init(&encoder, buf, sizeof(buf));
  ndn_name_tlv_encode(&encoder, &name_prefix);
  ndn_forwarder_register_prefix(encoder.output_value, encoder.offset, on_interest, NULL);
  while(running){
    ndn_forwarder_process();
    usleep(10000);
  }

  ndn_face_destroy(&face->intf);

  return 0;
}
