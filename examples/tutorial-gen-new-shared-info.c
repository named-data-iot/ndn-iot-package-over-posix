/*
 * Copyright (C) 2019
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3.0. See the file LICENSE in the top level
 * directory for more details.
 *
 * See AUTHORS.md for complete list of NDN IOT PKG authors and contributors.
 */
#include <stdio.h>
#include <ndn-lite.h>
#include "ndn-lite/security/ndn-lite-rng.h"

int
main(int argc, char *argv[])
{
  ndn_lite_startup();
  ndn_ecc_pub_t pre_installed_pub;
  ndn_ecc_prv_t pre_installed_prv;
  ndn_ecc_make_key(&pre_installed_pub, &pre_installed_prv, NDN_ECDSA_CURVE_SECP256R1, 1);
  uint16_t device_id;
  ndn_rng((uint8_t*)&device_id, 2);
  uint8_t hmacKey[16];
  ndn_rng(hmacKey, 16);

  FILE *fp;
  fp = fopen("tutorial_shared_info.txt", "w+");
  // prv key
  for (int i = 0; i < 32; i++) {
    fprintf(fp, "%02X", pre_installed_prv.abs_key.key_value[i]);
  }
  fprintf(fp, "\n");
  // device id
  fprintf(fp, "device-%d\n", (int)device_id);
  // pub key
  for (int i = 0; i < 64; i++) {
    fprintf(fp, "%02X", pre_installed_pub.abs_key.key_value[i]);
  }
  fprintf(fp, "\n");
  // hmac key
  for (int i = 0; i < 16; i++) {
    fprintf(fp, "%02X", hmacKey[i]);
  }
  fclose(fp);
  return 0;
}