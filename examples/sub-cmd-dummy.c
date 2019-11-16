
/*
 * Copyright (C) 2019 Tianyuan Yu, Zhiyi Zhang
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
#include "ndn-lite/app-support/ndn-sig-verifier.h"
#include "ndn-lite/app-support/pub-sub.h"
#include "ndn-lite/face/dummy-face.h"

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
ndn_dummy_face_t *face;
bool running;

uint8_t buffer[200];

int on_publish(uint8_t service, uint16_t type_action,
                const name_component_t* identifier, uint32_t component_size,
                const uint8_t* content, uint32_t content_len)
{
  ndn_data_t data;
  ndn_encoder_t encoder;

  printf("On content publish\n");
  return NDN_SUCCESS;
}


void
simlutae_bootstrap()
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
    ndn_sig_verifier_init(&face->intf);

    running = true;
}

void
prepare_notify_interest(uint8_t service, uint8_t* value, uint32_t value_size,
                        uint32_t* output_size)
{
  ndn_key_storage_t* storage = ndn_key_storage_get_instance();
  name_component_t* home_prefix = &storage->self_identity.components[0];

  int ret = 0;
  ndn_name_t name;
  ndn_name_init(&name);
  ndn_name_append_component(&name, home_prefix);
  ndn_name_append_bytes_component(&name, &service, sizeof(service));
  ndn_name_append_string_component(&name, "NOTIFY", strlen("NOTIFY"));
  ndn_name_append_string_component(&name, "aaa", strlen("aaa"));
  ndn_name_append_string_component(&name, "bbb", strlen("bbb"));
  // represent a cmd
  uint8_t cmd = 123;
  ndn_name_append_bytes_component(&name, &cmd, sizeof(cmd));
  tlv_make_interest(value, value_size, output_size, 4,
                      TLV_INTARG_NAME_PTR, &name,
                      TLV_INTARG_CANBEPREFIX_BOOL, true,
                      TLV_INTARG_MUSTBEFRESH_BOOL, true,
                      TLV_INTARG_LIFETIME_U64, (uint64_t)600);
}



int main(int argc, char *argv[]){
    int ret;

    ndn_lite_startup();
    face = ndn_dummy_face_construct();

    simlutae_bootstrap();
    ndn_key_storage_t* storage = ndn_key_storage_get_instance();
    name_component_t* home_prefix = &storage->self_identity.components[0];
    printf("add route: ");
    ndn_name_t home; ndn_name_init(&home);ndn_name_append_component(&home, home_prefix);
    ndn_name_print(&home);putchar('\n');
    ndn_forwarder_add_route_by_name(&face->intf, &home);

    // sub
    name_component_t id[2];
    char* id_1 = "aaa";
    name_component_from_string(&id[0], id_1, strlen(id_1));
    char* id_2 = "bbb";
    name_component_from_string(&id[1], id_2, strlen(id_2));

    // pub
    uint8_t content[8] = {1, 9, 2, 6, 0, 8, 1, 7};
    //ps_publish_content(NDN_SD_TEMP, id, 2, content, sizeof(content));
    //ps_subscribe_to(NDN_SD_TEMP, DATA, NULL, 0, 30000, on_publish);
    ps_subscribe_to(NDN_SD_TEMP, true, NULL, 0, 30000, on_publish, NULL);
    printf("one-time process\n");
    ndn_forwarder_process();

    uint32_t output_size = 0;
    prepare_notify_interest(NDN_SD_TEMP, buffer, sizeof(buffer), &output_size);
    ndn_forwarder_receive(face, buffer, output_size);

    printf("one-time process\n");
    ndn_forwarder_process();

    ndn_face_destroy(&face->intf);

    return 0;
}
