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

in_port_t port1, port2;
in_addr_t server_ip;
char * file_name;
ndn_name_t name_prefix;
uint8_t buf[4096];
bool running;

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

  face = ndn_udp_unicast_face_construct(INADDR_ANY, port1, server_ip, port2);
  running = true;
  encoder_init(&encoder, buf, 4096);
  ndn_name_tlv_encode(&encoder, &name_prefix);
  ndn_forwarder_add_route(&face->intf, buf, encoder.offset);

  char interest_buf[4096];
  int interest_off;
  tlv_make_interest(interest_buf,4096,&interest_off,3,TLV_INTARG_NAME_PTR,&name_prefix,TLV_INTARG_PARAMS_BUF,(uint8_t*)file_name,TLV_INTARG_PARAMS_SIZE,strlen(file_name));
  ndn_forwarder_express_interest(interest_buf, interest_off, on_data, on_timeout, NULL);

  while(running){
    ndn_forwarder_process();
    usleep(10000);
  }

  ndn_face_destroy(&face->intf);

  return 0;
}
