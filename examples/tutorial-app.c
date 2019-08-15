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
#include <ndn-lite.h>
#include "ndn-lite/encode/name.h"
#include "ndn-lite/encode/data.h"
#include "ndn-lite/encode/interest.h"
#include "ndn-lite/app-support/service-discovery.h"
#include "ndn-lite/app-support/security-bootstrapping.h"
#include "ndn-lite/encode/key-storage.h"

// DEVICE manufacture-created private key
uint8_t secp256r1_prv_key_str[] = {
  0xf9,'.',0xbc,0xc5,0xb1,0xcb,0x8c,0x08,'G','b','c','-','L',0xf1,0x96,0xcc,
  'r',0xed,0x91,0xaf,0x9e,'!',0x02,0xf2,0xef,'h','L',0xbc,'r',0xf5,'l',0x01
};

// HERE TO SET pre-shared public key
uint8_t secp256r1_pub_key_str[64] = {
  0x90,0xa6,0xbc,0xe8,0x00,'W',0xc0,'e', 0xe9,0x8a,'\\',0x05,'(','d',0x9a,0x99,
  'y',0xc1,0x10,0x0f,0xf8,0x8a,0xd0,'I','U',0xaa,0xbf,0xbb,0x1b,'\\',0xe2,0xab,
  '9','W',0x89,0x96,0xb5,0xee,':',0xf9,'_',0xd3,0x89,0x15,0xdc,3,0x7f,'g',
  0xca,'R','b','\t',0xbe,0x88,'Y',0xe2,0xbc,0xcf,0xbd,0xd4,0x18,0xdd,'8',0x01
};

//HERE TO SET pre-shared secrets
uint8_t hmac_key_str[] = {
    0x54,0x69,0xB8,0xC0,0xB6,0x28,0x77,0x70,
    0x1C,0xDD,0xE8,0x89,0x92,0x03,0xFD,0xDE
};

// Device identifer
char* device_identifier;
// Buf used in this program
uint8_t buf[4096];
// Device services
uint8_t * capability;
// Wether the program is running or not
bool running;

int
parseArgs(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "ERROR: wrong arguments.\n");
    printf("<device-identifier>\n");
    return 1;
  }
  device_identifier = argv[1];
  return 0;
}

int
main(int argc, char *argv[])
{
  ndn_unix_face_t *face;
  ndn_encoder_t encoder;
  int ret;
  if ((ret = parseArgs(argc, argv)) != 0) {
    return ret;
  }
  ndn_lite_startup();

  // CREAT A MULTICAST FACE
  // face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR,true);
  face = ndn_udp_unicast_face_construct(INADDR_ANY, htons((uint16_t) 2000), inet_addr("224.0.23.170"), htons((uint16_t) 56363));
  in_port_t multicast_port = htons((uint16_t) 56363);
  in_addr_t multicast_ip = inet_addr("224.0.23.170");
  face = ndn_udp_multicast_face_construct(INADDR_ANY, multicast_port, multicast_ip);
  // SET UP ROUTE
  running = true;
  ndn_forwarder_add_route_by_str(&face->intf, "/ndn/sign-on", strlen("/ndn/sign-on"));

  // BOOTSTRAPPING PROCESS
  capability = (uint8_t *) malloc(sizeof(uint8_t) * 2);
  capability[0] = NDN_SD_LED;
  capability[1] = NDN_SD_TEMP;
  // PARSE PRE-INSTALLED PRV KEY AND PUB KEY
  ndn_key_storage_t* key_storage =  ndn_key_storage_init();
  ndn_ecc_prv_t* ecc_secp256r1_prv_key;
  ndn_ecc_pub_t* ecc_secp256r1_pub_key;
  ndn_key_storage_get_empty_ecc_key(&ecc_secp256r1_prv_key,&ecc_secp256r1_pub_key);
  ndn_ecc_prv_init(ecc_secp256r1_prv_key, secp256r1_prv_key_str, sizeof(secp256r1_prv_key_str),
                   NDN_ECDSA_CURVE_SECP256R1, 1);
  ndn_ecc_pub_init(ecc_secp256r1_pub_key, secp256r1_pub_key_str, sizeof(secp256r1_pub_key_str),
                   NDN_ECDSA_CURVE_SECP256R1, 1);
  ndn_hmac_key_t* hmac_key;
  ndn_key_storage_get_empty_hmac_key(&hmac_key);
  ndn_hmac_key_init(hmac_key,hmac_key_str, sizeof(hmac_key_str), 2);

  // SET UP BOOTSTRAPPING
  ndn_security_bootstrapping(&face->intf, ecc_secp256r1_prv_key,hmac_key,device_identifier,strlen(device_identifier),capability,strlen(capability));

  // SET UP SERVICE DISCOVERY
  ndn_sd_init();
  sd_add_or_update_self_service(NDN_SD_LED, true, 1); // state code 1 means normal
  sd_add_or_update_self_service(NDN_SD_TEMP, true, 1); // state code 1 means normal

  // START MAIN LOOP
  while(running){
    ndn_forwarder_process();
    usleep(10000);
  }

  // DESTROY FACE
  ndn_face_destroy(&face->intf);
  return 0;
}
