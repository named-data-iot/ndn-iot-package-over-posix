/*
 * Copyright (C) 2019 Zhiyi Zhang
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3.0. See the file LICENSE in the top level
 * directory for more details.
 */

#include "ndn-lite/app-support/access-control.h"
#include "ndn-lite/encode/signed-interest.h"
#include "ndn-lite/encode/key-storage.h"
#include "ndn-lite/forwarder/forwarder.h"
#include "ndn-lite/face/direct-face.h"
#include "adaptation/udp-multicast/ndn-udp-multicast-face.h"
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

const uint8_t prv[] = {
  0x5D, 0xC7, 0x6B, 0xAB, 0xEE, 0xD4, 0xEB, 0xB7, 0xBA, 0xFC,
  0x64, 0xE7, 0x8B, 0xDB, 0x22, 0xE1, 0xF4, 0x37, 0x10, 0xC2,
  0xEA, 0xE9, 0xDD, 0xAF, 0xF4, 0x74, 0xB3, 0x18, 0x08, 0x56,
  0x5E, 0x4C
};

const uint8_t pub[] = {
  0x36, 0xF7, 0xEF, 0x7C, 0x05, 0x10, 0x68, 0xC4, 0x6C, 0x67,
  0x63, 0x2A, 0xF5, 0x82, 0x1D, 0x14, 0xBA, 0xCC, 0x50, 0x12,
  0x73, 0x73, 0xED, 0xDE, 0x7D, 0x23, 0x5D, 0x20, 0xA8, 0x5E,
  0xD1, 0x83, 0x3C, 0x0F, 0xB7, 0xD2, 0x6E, 0xB2, 0x0F, 0x8B,
  0x09, 0x1D, 0xD0, 0xF3, 0xB9, 0xAA, 0x56, 0x11, 0x1D, 0x15,
  0x0C, 0xAC, 0xE4, 0xFA, 0x9F, 0x6C, 0x61, 0xB4, 0xFF, 0x41,
  0xE8, 0xBA, 0x21, 0x89
};

ndn_ecc_pub_t* pub_key = NULL;
ndn_ecc_prv_t* prv_key = NULL;
in_addr_t multicast_ip;

int
parseArgs(int argc, char *argv[]) {
  char *sz_addr;
  struct hostent *host_addr;
  struct in_addr **paddrs;

  if (argc < 1) {
    char defaultaddr[] = "225.0.0.37";
    sz_addr = defaultaddr;
    return 1;
  }
  else
    sz_addr = argv[1];

  if (strlen(sz_addr) <= 0) {
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
  multicast_ip = paddrs[0]->s_addr;
  return 0;
}

void
print_error(const char *test_name, const char *fnct_name, const char *funct_failed, int err_code) {
  printf("In %s test, within call to %s, call to %s failed, error code: %d\n",
         test_name, fnct_name, funct_failed, err_code);
}

int
on_data(const uint8_t* data, uint32_t data_size)
{
  printf("Get EK Data\n");

  // parse response Data
  ndn_data_t response;
  int ret_val = ndn_data_tlv_decode_ecdsa_verify(&response, data, data_size, pub_key);
  if (ret_val != 0) {
    print_error("producer", "on_data", "ndn_data_tlv_decode_ecdsa_verify", ret_val);
  }

  // update ac state
  ret_val = ndn_ac_on_ek_response_process(&response);
  if (ret_val != 0) {
    print_error("producer", "on_data", "ndn_ac_on_ek_response", ret_val);
  }

  return 0;
}

int
main(int argc, char *argv[])
{
  int ret_val = -1;
  if ((ret_val = parseArgs(argc, argv)) != 0) {
    return ret_val;
  }

  // init security
  ndn_security_init();

  // set home prefix
  ndn_name_t home_prefix;
  char* home_prefix_str = "/ndn";
  ret_val = ndn_name_from_string(&home_prefix, home_prefix_str, sizeof(home_prefix_str));
  if (ret_val != 0) {
    print_error("producer", "set home prefix", "ndn_name_from_string", ret_val);
    return -1;
  }

  // set identity name
  char comp_producer[] = "producer";
  name_component_t component_producer;
  ret_val = name_component_from_string(&component_producer, comp_producer, sizeof(comp_producer));
  if (ret_val != 0) {
    print_error("producer", "set identity name", "name_component_from_string", ret_val);
    return -1;
  }
  ndn_name_t producer_identity = home_prefix;
  ret_val = ndn_name_append_component(&producer_identity, &component_producer);
  if (ret_val != 0) {
    print_error("producer", "set identity name", "ndn_name_append_component", ret_val);
    return -1;
  }

  // init keys
  ndn_key_storage_init();
  ndn_key_storage_get_empty_ecc_key(&pub_key, &prv_key);
  ret_val = ndn_ecc_prv_init(prv_key, prv, sizeof(prv),
                             NDN_ECDSA_CURVE_SECP256R1, 123);
  if (ret_val != 0) {
    print_error("producer", "init keys", "ndn_ecc_prv_init", ret_val);
    return -1;
  }
  ret_val = ndn_ecc_pub_init(pub_key, pub, sizeof(pub),
                             NDN_ECDSA_CURVE_SECP256R1, 123);
  if (ret_val != 0) {
    print_error("producer", "init keys", "ndn_ecc_pub_init", ret_val);
    return -1;
  }

  // init ac state
  ndn_ac_state_init(&producer_identity, pub_key, prv_key);

  // prepare EK Interest
  uint8_t buffer[1024];
  ndn_encoder_t interest_encoder;
  encoder_init(&interest_encoder, buffer, sizeof(buffer));
  ret_val = ndn_ac_prepare_key_request_interest(&interest_encoder, &home_prefix, &component_producer,
                                                100, prv_key, 1);
  if (ret_val != 0) {
    print_error("producer", "prepare EK Interest", "ndn_ac_prepare_key_request", ret_val);
  }

  // set up direct face and forwarder
  ndn_forwarder_init();
  ndn_direct_face_construct(666);
  ndn_udp_muticast_face_t* udp_face;
  udp_face = ndn_udp_muticast_face_construct(667, INADDR_ANY, 6363, multicast_ip);

  // add route
  char prefix_string[] = "/ndn/AC";
  ndn_name_t controller_prefix;
  ret_val = ndn_name_from_string(&controller_prefix, prefix_string, sizeof(prefix_string));
  if (ret_val != 0) {
    print_error("consumer", "add route", "ndn_name_from_string", ret_val);
  }
  ndn_forwarder_fib_insert(&controller_prefix, udp_face, 0);

  while (running) {
    ndn_udp_multicast_face_recv(consumer_udp_face);
    usleep(10);
  }

  return 0;
}
