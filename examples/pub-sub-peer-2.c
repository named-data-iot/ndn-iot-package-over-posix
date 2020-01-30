/*
 * Copyright (C) 2019 Zhiyi Zhang, Tianyuan Yu
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
#include <ndn-lite.h>
#include "ndn-lite/encode/name.h"
#include "ndn-lite/encode/data.h"
#include "ndn-lite/encode/interest.h"
#include "ndn-lite/app-support/pub-sub.h"

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

ndn_unix_face_t *face;
uint8_t anchor_bytes[2048];
uint32_t anchor_bytes_size;
bool running;
uint8_t buffer[4096];

void on_command_publish(const ps_event_context_t* context, const ps_event_t* event, void* userdata)
{
  printf("\n\nNew Command publish!! Content: %llu\n\n", *(uint64_t*)event->payload);
}

void on_content_publish(const ps_event_context_t* context, const ps_event_t* event, void* userdata)
{
    printf("\n\nNew Content publish!! Content: %llu\n\n", *(uint64_t*)event->payload);
}

void
simulate_bootstrap()
{
  ndn_encoder_t encoder;
  // simulate bootstrapping process
  ndn_ecc_prv_t anchor_prv_key;
  ndn_ecc_prv_init(&anchor_prv_key, secp256r1_prv_key_str, sizeof(secp256r1_prv_key_str),
                   NDN_ECDSA_CURVE_SECP256R1, 123);
  ndn_ecc_pub_t anchor_pub_key;
  ndn_ecc_pub_init(&anchor_pub_key, secp256r1_pub_key_str, sizeof(secp256r1_pub_key_str), NDN_ECDSA_CURVE_SECP256R1, 123);

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

  // ndn_ecc_prv_t self_prv_key
  ndn_ecc_pub_t* self_pub = NULL;
  ndn_ecc_prv_t* self_prv = NULL;
  ndn_key_storage_get_empty_ecc_key(&self_pub, &self_prv);
  ndn_ecc_make_key(self_pub, self_prv, NDN_ECDSA_CURVE_SECP256R1, 234);

  // self cert
  ndn_data_t self_cert;
  ndn_data_init(&self_cert);
  ndn_name_from_string(&self_cert.name, "/ndn-iot/bedroom/peer-2/KEY", strlen("/ndn-iot/bedroom/peer-2/KEY"));
  ndn_name_append_keyid(&self_cert.name, 234);
  ndn_name_append_string_component(&self_cert.name, "home", strlen("home"));
  ndn_name_append_keyid(&self_cert.name, 567);
  ndn_data_set_content(&self_cert, ndn_ecc_get_pub_key_value(self_pub),
                       ndn_ecc_get_pub_key_size(self_pub));
  encoder_init(&encoder, anchor_bytes, sizeof(anchor_bytes));
  ndn_data_tlv_encode_ecdsa_sign(&encoder, &self_cert, &anchor_id, &anchor_prv_key);
  ndn_data_tlv_decode_no_verify(&self_cert, encoder.output_value, encoder.offset, NULL, NULL);
  ndn_key_storage_set_self_identity(&self_cert, self_prv);

  // register prefix
  ndn_forwarder_add_route_by_str(face, "/ndn-iot", strlen("/ndn-iot"));
}

int main(int argc, char *argv[])
{
  int ret = 0;
  ndn_lite_startup();
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
  simulate_bootstrap();

  ps_subscribe_to_content(20, "/bedroom/peer-1", 5000, on_content_publish, NULL);
  ps_subscribe_to_command(NDN_SD_TEMP, "", on_command_publish, NULL);

  ps_after_bootstrapping();
  ndn_forwarder_process();

  uint64_t content = 0;
  running = true;
  char* content_id = "current-temp";
  uint8_t command_id = 100;
  while(running) {
    ndn_forwarder_process();

    // pub content
    ps_event_t content_event = {
      .data_id = (uint8_t*)content_id,
      .data_id_len = strlen(content_id),
      .payload = &content,
      .payload_len = 8
    };
    ps_publish_content(NDN_SD_TEMP, &content_event);

    content++;

    // pub command
    ps_event_t cmd_event = {
      .data_id = (uint8_t*)command_id,
      .data_id_len = strlen(command_id),
      .payload = &content,
      .payload_len = 8
    };
    ps_publish_command(20, "/bedroom/peer-1", &cmd_event);

    content++;

    usleep(1000000);
  }
  ndn_face_destroy(&face->intf);
  return 0;
}
