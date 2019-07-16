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

in_port_t port1, port2;
in_addr_t client_ip;
ndn_name_t name_prefix;
uint8_t buf[4096];
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


int on_interest(const uint8_t* interest, uint32_t interest_size, void* userdata){
  ndn_data_t data;
  ndn_encoder_t encoder;

  printf("On interest\n");

  ndn_interest_t ek_interest;

  ndn_name_t ek_name;
  // char file_name[1024];
  char* file_name;
  int param_size;
  tlv_parse_interest(interest,interest_size,3,TLV_INTARG_NAME_PTR,&ek_name,TLV_INTARG_PARAMS_BUF,(uint8_t**)&file_name,TLV_INTARG_PARAMS_SIZE,&param_size);
  file_name[param_size] = '\0';

  char temp_buffer[1024];
  FILE *fp = fopen(file_name,"r");
  printf("The requested file name is: %s\n",file_name);
  if(fp == NULL){
    fprintf(stderr, "ERROR: fail to open file.\n");
    return 1;
  }
  if(fgets(temp_buffer,1024,fp) == NULL){
    fprintf(stderr, "ERROR: fail to read file.\n");
    return 2;
  }
  //printf("The content of the file is: %s, %lu\n",temp_buffer,strlen(temp_buffer) );

  uint8_t data_buf[4096];
  int data_off;
  tlv_make_data(data_buf,4096,&data_off,3,TLV_DATAARG_NAME_PTR,&ek_name,TLV_DATAARG_CONTENT_BUF,(uint8_t*)temp_buffer,TLV_DATAARG_CONTENT_SIZE,strlen(temp_buffer));
  ndn_forwarder_put_data(data_buf,data_off);
  return 0;
}


int main(int argc, char *argv[]){
  int ret;
  ndn_encoder_t encoder;

  if((ret = parseArgs(argc, argv)) != 0){
    return ret;
  }

  ndn_lite_startup();
  face = ndn_udp_unicast_face_construct(INADDR_ANY, port1, client_ip, port2);

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
