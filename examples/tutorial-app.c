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
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ndn-lite.h>
#include "ndn-lite/encode/name.h"
#include "ndn-lite/encode/data.h"
#include "ndn-lite/encode/interest.h"
#include "ndn-lite/app-support/service-discovery.h"
#include "ndn-lite/app-support/security-bootstrapping.h"
#include "ndn-lite/encode/key-storage.h"

// DEVICE manufacture-created private key
uint8_t secp256r1_prv_key_bytes[32] = {0};

// HERE TO SET pre-shared public key
uint8_t secp256r1_pub_key_bytes[64] = {0};

//HERE TO SET pre-shared secrets
uint8_t hmac_key_bytes[16] = {0};

// Device identifer
char device_identifier[30];
size_t device_len;

// Face Declare
// ndn_udp_face_t *face;
ndn_unix_face_t *face;
// Buf used in this program
uint8_t buf[4096];
// Wether the program is running or not
bool running;
// A global var to keep the brightness
uint8_t light_brightness;
// The locator of the device, e.g., /bedroom/sensor1
ndn_name_t locator;

int
parseArgs(int argc, char *argv[])
{
  FILE * fp;
  char buf[255];
  char* buf_ptr;
  fp = fopen("tutorial_shared_info.txt", "r");
  if (fp == NULL) exit(1);
  size_t i = 0;
  for (size_t lineindex = 0; lineindex < 4; lineindex++) {
    memset(buf, 0, sizeof(buf));
    buf_ptr = buf;
    fgets(buf, sizeof(buf), fp);
    if (lineindex == 0) {
      for (i = 0; i < 32; i++) {
        sscanf(buf_ptr, "%2hhx", &secp256r1_prv_key_bytes[i]);
        buf_ptr += 2;
      }
    }
    else if (lineindex == 1) {
      buf[strlen(buf) - 1] = '\0';
      strcpy(device_identifier, buf);
    }
    else if (lineindex == 2) {
      for (i = 0; i < 64; i++) {
        sscanf(buf_ptr, "%2hhx", &secp256r1_pub_key_bytes[i]);
        buf_ptr += 2;
      }
    }
    else {
      for (i = 0; i < 16; i++) {
        sscanf(buf_ptr, "%2hhx", &hmac_key_bytes[i]);
        buf_ptr += 2;
      }
    }
  }
  fclose(fp);

  // prv key
  printf("Pre-installed ECC Private Key:");
  for (int i = 0; i < 32; i++) {
    printf("%02X", secp256r1_prv_key_bytes[i]);
  }
  printf("\nPre-installed Device Identifier: ");
  // device id
  printf("%s\nPre-installed ECC Pub Key: ", device_identifier);
  // pub key
  for (int i = 0; i < 64; i++) {
    printf("%02X", secp256r1_pub_key_bytes[i]);
  }
  printf("\nPre-installed Shared Secret: ");
  // hmac key
  for (int i = 0; i < 16; i++) {
    printf("%02X", hmac_key_bytes[i]);
  }
  printf("\n");
  return 0;
}

int
light_service(const uint8_t* interest, uint32_t interest_size, void* userdata)
{
  uint8_t *param, *name, new_val;
  ndn_name_t name_check;
  size_t param_size, ret_size;
  int ret;

  printf("RECEIVED INTEREST\n");

  ret = tlv_parse_interest(interest, interest_size, 4,
                           TLV_INTARG_NAME_BUF, &name,
                           TLV_INTARG_NAME_PTR, &name_check,
                           TLV_INTARG_PARAMS_BUF, &param,
                           TLV_INTARG_PARAMS_SIZE, &param_size);
  if (ret != NDN_SUCCESS) {
    return NDN_FWD_STRATEGY_SUPPRESS;
  }
  if (param_size <= 10) {
    printf("No signature. Ignore the command.");
    return -1;
  }

  // Remove parameter digest
  if(name_check.components[name_check.components_size - 1].type != TLV_GenericNameComponent){
    name_check.components_size --;
  }

  // Check the function ID (=0)
  if(name_check.components[name_check.components_size - 1].size != 1 ||
     name_check.components[name_check.components_size - 1].value[0] != 0) {
    return NDN_FWD_STRATEGY_SUPPRESS;
  }

  // Check the locator (can become API)
  if(name_check.components_size - 3 > locator.components_size){
    return NDN_FWD_STRATEGY_SUPPRESS;
  }
  if(ndn_name_compare_sub_names(&locator, 0, name_check.components_size - 3,
                                &name_check, 2, name_check.components_size - 1) != 0) {
    return NDN_FWD_STRATEGY_SUPPRESS;
  }

  // check signature
  ndn_key_storage_t* storage = ndn_key_storage_get_instance();
  ndn_data_t signature_carrier;
  ret = ndn_data_tlv_decode_ecdsa_verify(&signature_carrier, param + 1, param_size - 1, &storage->trust_anchor_key);
  if (ret != NDN_SUCCESS) {
    printf("Cannot verify signature. Ignore the command.");
    return NDN_FWD_STRATEGY_SUPPRESS;
  }
  printf("Signature verify success. Execute the command.");

  // Execute the function
  if (param) {
    new_val = *param;
  }
  else {
    new_val = 0xFF;
  }
  if (new_val != 0xFF) {
    if ((new_val > 0) != (light_brightness > 0)) {
      if (new_val > 0) {
        printf("Switch on the light.\n");
      }
      else {
        printf("Turn off the light.\n");
      }
    }
    if (new_val < 10) {
      light_brightness = new_val;
      if (light_brightness > 0) {
        printf("Successfully set the brightness = %u\n", light_brightness);
      }
    }
    else {
      light_brightness = 10;
      printf("Exceeding range. Set the brightness = %u\n", light_brightness);
    }
  }
  else {
    printf("Query the brightness = %u\n", light_brightness);
  }

  tlv_make_data(buf, sizeof(buf), &ret_size, 4,
                TLV_DATAARG_NAME_BUF, name,
                TLV_DATAARG_CONTENT_BUF, &light_brightness,
                TLV_DATAARG_CONTENT_SIZE, sizeof(light_brightness),
                TLV_DATAARG_FRESHNESSPERIOD_U64, 1000);
  ndn_forwarder_put_data(buf, ret_size);
  return NDN_FWD_STRATEGY_SUPPRESS;
}

void
after_bootstrapping()
{
  uint8_t temp_byte = 0;
  ndn_name_t temp_name;
  ndn_key_storage_t* storage = ndn_key_storage_get_instance();

  // set locator
  ndn_name_init(&locator);
  for (int i = 1; i < storage->self_identity.components_size; i ++) {
    ndn_name_append_component(&locator, &storage->self_identity.components[i]);
  }

  // Register light service
  ndn_name_init(&temp_name);
  ndn_name_append_component(&temp_name, &storage->self_identity.components[0]);
  temp_byte = NDN_SD_LED;
  ndn_name_append_bytes_component(&temp_name, &temp_byte, sizeof(temp_byte));
  ndn_forwarder_register_name_prefix(&temp_name, light_service, NULL);
}

void SignalHandler(int signum){
  running = false;
}

int
main(int argc, char *argv[])
{
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  signal(SIGQUIT, SignalHandler);

  // PARSE COMMAND LINE PARAMETERS
  int ret;
  if ((ret = parseArgs(argc, argv)) != 0) {
    return ret;
  }
  ndn_lite_startup();

  // CREAT A MULTICAST FACE
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
  // face = ndn_udp_unicast_face_construct(INADDR_ANY, htons((uint16_t) 2000), inet_addr("224.0.23.170"), htons((uint16_t) 56363));
  // in_port_t multicast_port = htons((uint16_t) 56363);
  // in_addr_t multicast_ip = inet_addr("224.0.23.170");
  // face = ndn_udp_multicast_face_construct(INADDR_ANY, multicast_ip, multicast_port);

  // LOAD PRE-INSTALLED PRV KEY AND PUB KEY
  ndn_ecc_prv_t* ecc_secp256r1_prv_key;
  ndn_ecc_pub_t* ecc_secp256r1_pub_key;
  ndn_key_storage_get_empty_ecc_key(&ecc_secp256r1_pub_key, &ecc_secp256r1_prv_key);
  ndn_ecc_prv_init(ecc_secp256r1_prv_key, secp256r1_prv_key_bytes, sizeof(secp256r1_prv_key_bytes),
                   NDN_ECDSA_CURVE_SECP256R1, 1);
  ndn_ecc_pub_init(ecc_secp256r1_pub_key, secp256r1_pub_key_bytes, sizeof(secp256r1_pub_key_bytes),
                   NDN_ECDSA_CURVE_SECP256R1, 1);
  ndn_hmac_key_t* hmac_key;
  ndn_key_storage_get_empty_hmac_key(&hmac_key);
  ndn_hmac_key_init(hmac_key, hmac_key_bytes, sizeof(hmac_key_bytes), 2);

  // LOAD SERVICES PROVIDED BY SELF DEVICE
  uint8_t capability[2];
  capability[0] = NDN_SD_LED;
  capability[1] = NDN_SD_TEMP;

  // SET UP SERVICE DISCOVERY
  ndn_sd_init();
  sd_add_or_update_self_service(NDN_SD_LED, true, 1); // state code 1 means normal
  sd_add_or_update_self_service(NDN_SD_TEMP, true, 1); // state code 1 means normal

  // START BOOTSTRAPPING
  ndn_security_bootstrapping(&face->intf, ecc_secp256r1_prv_key,hmac_key,
                             device_identifier, strlen(device_identifier),
                             capability, sizeof(capability), after_bootstrapping);

  // START MAIN LOOP
  running = true;
  while(running) {
    ndn_forwarder_process();
    usleep(10000);
  }

  // DESTROY FACE
  ndn_face_destroy(&face->intf);
  return 0;
}
