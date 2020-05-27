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

#include "ndn-lite/app-support/repo.h"

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

int
load_bootstrapping_info()
{
  FILE * fp;
  char buf[255];
  char* buf_ptr;
  fp = fopen("../devices/tutorial_shared_info-398.txt", "r");
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
on_interest(const uint8_t* interest, uint32_t interest_size, void* userdata)
{
  ndn_interest_t interest_pkt;
  ndn_interest_from_block(&interest_pkt, interest, interest_size);
  ndn_data_t data;
  ndn_encoder_t encoder;
  char * str = "I'm a Data packet.";

  printf("On interest: \n");
  ndn_name_print(&interest_pkt.name);
  data.name = interest_pkt.name;
  ndn_data_set_content(&data, (uint8_t*)str, strlen(str));
  ndn_metainfo_init(&data.metainfo);
  ndn_metainfo_set_content_type(&data.metainfo, NDN_CONTENT_TYPE_BLOB);
  ndn_metainfo_set_freshness_period(&data.metainfo, 10000);
  encoder_init(&encoder, buf, 4096);
  ndn_data_tlv_encode_digest_sign(&encoder, &data);
  ndn_forwarder_put_data(encoder.output_value, encoder.offset);

  return NDN_FWD_STRATEGY_SUPPRESS;
}


void
periodic_insert()
{ 
  static ndn_time_ms_t last;

  ndn_name_t name;
  ndn_name_from_string(&name, "/ndn-iot/1/test-new/rng", strlen("/ndn-iot/1/test-new/rng"));

  if (ndn_time_now_ms() - last >= 10000) {
    uint8_t rng[4];
    ndn_rng(rng, sizeof(rng));
    ndn_name_append_bytes_component(&name, rng, sizeof(rng));
    ndn_repo_publish_cmd_param(&name, NDN_SD_LED);
    last = ndn_time_now_ms();
  }
  ndn_msgqueue_post(NULL, periodic_insert, 0, NULL);
}

void
after_bootstrapping()
{
  ndn_repo_init();

  ndn_name_t name;
  ndn_name_from_string(&name, "/ndn-iot/1/test-new", strlen("/ndn-iot/1/test-new"));
  ndn_forwarder_register_name_prefix(&name, on_interest, NULL);
  ndn_time_delay(10000);
  periodic_insert();
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
  int ret = NDN_SUCCESS;
  if ((ret = load_bootstrapping_info()) != 0) {
    return ret;
  }
  ndn_lite_startup();

  // CREAT A MULTICAST FACE
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);

  // LOAD SERVICES PROVIDED BY SELF DEVICE
  uint8_t capability[1];
  capability[0] = NDN_SD_LED;

  // SET UP SERVICE DISCOVERY
  sd_add_or_update_self_service(NDN_SD_LED, true, 1); // state code 1 means normal
  ndn_ac_register_encryption_key_request(NDN_SD_LED);
  //ndn_ac_register_access_request(NDN_SD_LED);

  // START BOOTSTRAPPING
  ndn_bootstrapping_info_t booststrapping_info = {
    .pre_installed_prv_key_bytes = secp256r1_prv_key_bytes,
    .pre_installed_pub_key_bytes = secp256r1_pub_key_bytes,
    .pre_shared_hmac_key_bytes = hmac_key_bytes,
  };
  ndn_device_info_t device_info = {
    .device_identifier = device_identifier,
    .service_list = capability,
    .service_list_size = sizeof(capability),
  };
  ndn_security_bootstrapping(&face->intf, &booststrapping_info, &device_info, after_bootstrapping);

  // START MAIN LOOP
  running = true;
  while(running) {
    ndn_forwarder_process();
    usleep(1000);
  }

  // DESTROY FACE
  ndn_face_destroy(&face->intf);
  return 0;
}
