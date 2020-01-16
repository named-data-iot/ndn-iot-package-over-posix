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
#include "ndn-lite/app-support/access-control.h"
#include "ndn-lite/app-support/security-bootstrapping.h"
#include "ndn-lite/app-support/ndn-sig-verifier.h"
#include "ndn-lite/app-support/pub-sub.h"
#include "ndn-lite/encode/key-storage.h"
#include "ndn-lite/encode/ndn-rule-storage.h"

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
uint8_t light_brightness = 0;

static ndn_trust_schema_rule_t same_room;
static ndn_trust_schema_rule_t controller_only;

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

void
light_service(uint8_t service, bool is_cmd,
              const name_component_t* identifiers, uint32_t identifiers_size,
              uint8_t* suffix, uint32_t suffix_len,
              const uint8_t* content, uint32_t content_len,
              void* userdata)
{
  uint8_t *param, *name, new_val;
  ndn_name_t name_check;

  printf("RECEIVED NEW COMMAND\n");

  // Execute the function
  if (content) {
    // new_val = *real_payload;
    char content_str[128] = {0};
    memcpy(content_str, content, content_len);
    content_str[content_len] = '\0';
    new_val = atoi(content_str);
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
        ps_publish_content(NDN_SD_LED, "a", strlen("a"), &light_brightness, 1);
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
}

void
after_bootstrapping()
{
  ps_subscribe_to_command(NDN_SD_LED, NULL, 0, light_service, NULL);
  ps_publish_content(NDN_SD_LED, "a", strlen("a"), "hello", strlen("hello"));
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
  ndn_hmac_key_t* hmac_key = ndn_key_storage_get_empty_hmac_key();
  ndn_hmac_key_init(hmac_key, hmac_key_bytes, sizeof(hmac_key_bytes), 2);

  // LOAD SERVICES PROVIDED BY SELF DEVICE
  uint8_t capability[1];
  capability[0] = NDN_SD_LED;

  // SET UP SERVICE DISCOVERY
  sd_add_or_update_self_service(NDN_SD_LED, true, 1); // state code 1 means normal
  ndn_ac_register_encryption_key_request(NDN_SD_LED);
  // ndn_ac_register_access_request(NDN_SD_TEMP);

  // START BOOTSTRAPPING
  ndn_security_bootstrapping(&face->intf, ecc_secp256r1_prv_key, hmac_key,
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
