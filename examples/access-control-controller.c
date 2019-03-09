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
#include "../ndn-riot-tests/print-helpers.h"
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
ndn_name_t controller_identity;
ndn_interest_t ek_interest;

void
print_error(const char *test_name, const char *fnct_name, const char *funct_failed, int err_code) {
  printf("In %s test, within call to %s, call to %s failed, error code: %d\n",
         test_name, fnct_name, funct_failed, err_code);
}

int
on_interest(const uint8_t* interest, uint32_t interest_size)
{
  // parse incoming Interest
  printf("Get EK/DK Interest\n");
  int ret_val = ndn_interest_from_block(&ek_interest, interest, interest_size);
  if (ret_val != 0) {
    print_error("controller", "on_EKInterest", "ndn_interest_from_block", ret_val);
  }
  ret_val = ndn_signed_interest_ecdsa_verify(&ek_interest, pub_key);
  if (ret_val != 0) {
    print_error("controller", "on_EKInterest", "ndn_signed_interest_ecdsa_verify", ret_val);
  }

  // react on the Interest
  ndn_data_t response;
  ret_val = ndn_ac_on_interest_process(&response, &ek_interest);
  if (ret_val != 0) {
    print_error("controller", "on_EKInterest", "ndn_ac_on_interest_process", ret_val);
  }

  // reply the Data packet
  ndn_encoder_t encoder;
  ret_val = ndn_data_tlv_encode_ecdsa_sign(&encoder, &response, &controller_identity,
                                           prv_key);


  return 0;
}

int
main(void)
{
  // init security
  ndn_security_init();

  // set home prefix
  ndn_name_t home_prefix;
  char* home_prefix_str = "/ndn";
  int ret_val = ndn_name_from_string(&home_prefix, home_prefix_str, sizeof(home_prefix_str));
  if (ret_val != 0) {
    print_error("controller", "set home prefix", "ndn_name_from_string", ret_val);
    return -1;
  }

  // set identity name
  char comp_controller[] = "controller";
  name_component_t component_controller;
  ret_val = name_component_from_string(&component_controller, comp_controller, sizeof(comp_controller));
  if (ret_val != 0) {
    print_error("controller", "set identity name", "name_component_from_string", ret_val);
    return -1;
  }
  controller_identity = home_prefix;
  ret_val = ndn_name_append_component(&controller_identity, &component_controller);
  if (ret_val != 0) {
    print_error("controller", "set identity name", "ndn_name_append_component", ret_val);
    return -1;
  }

  // generate controller keys
  ndn_key_storage_init();
  ndn_key_storage_get_empty_ecc_key(&pub_key, &prv_key);
  ret_val = ndn_ecc_prv_init(prv_key, prv, sizeof(prv),
                             NDN_ECDSA_CURVE_SECP256R1, 123);
  if (ret_val != 0) {
    print_error("controller", "init keys", "ndn_ecc_prv_init", ret_val);
    return -1;
  }
  ret_val = ndn_ecc_pub_init(pub_key, pub, sizeof(pub),
                             NDN_ECDSA_CURVE_SECP256R1, 123);
  if (ret_val != 0) {
    print_error("controller", "init keys", "ndn_ecc_pub_init", ret_val);
    return -1;
  }

  // init ac state
  ndn_ac_state_init(&controller_identity, pub_key, prv_key);

  // set up direct face and forwarder
  ndn_forwarder_t* forwarder = ndn_forwarder_init();
  ndn_direct_face_t* direct_face = ndn_direct_face_construct(666);

  // register prefix
  char prefix_string[] = "/ndn/AC";
  ndn_name_t prefix;
  ret_val = ndn_name_from_string(&prefix, prefix_string, sizeof(prefix_string));
  if (ret_val != 0) {
    print_error("controller", "register prefix", "ndn_name_from_string", ret_val);
  }
  ret_val = ndn_direct_face_register_prefix(&prefix, on_interest);
  if (ret_val != 0) {
    print_error("controller", "register prefix", "ndn_direct_face_register_prefix", ret_val);
  }

  return 0;
}
