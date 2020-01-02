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
#include "ndn-lite/app-support/security-bootstrapping.h"
#include "ndn-lite/encode/key-storage.h"

char * device_identifier;
uint8_t buf[4096];

//HERE TO SET key-pair
// secp192r1_prv_key: '\x005\x12\xbc\xae50\x7f\xee\x1by?v\xd5\x97\xdc0.\xf6\x9c[)5\r'
uint8_t secp192r1_prv_key_str[] = {
  0x00,'5',0x12,0xbc,0xae,'5','0',0x7f,
  0xee,0x1b,'y','?','v',0xd5,0x97,0xdc,
  '0','.',0xf6,0x9c,'[',')','5','\r'
};

//secp192r1_pub_key: '\x05"\x118\xd1\xe2\xed\x1c\x14\x80\xc3\x08\x00\x03\xba\xda\xe2\x89\x95\x19(2\xa5Ygk\x8e\x047\xad]\xdf\xc7O\x95\x18l\xd0\xa7\xa3X4\x88\xe7\x1f\x00\x12/'
uint8_t secp192r1_pub_key_str[] = {
  0x05,'"',0x11,8,0xd1,0xe2,0xed,0x1c,
  0x14,0x80,0xc3,0x08,0x00,0x03,0xba,0xda,
  0xe2,0x89,0x95,0x19,'(','2',0xa5,'Y',
  'g','k',0x8e,0x04,'7',0xad,']',0xdf,
  0xc7,'O',0x95,0x18,'l',0xd0,0xa7,0xa3,
  'X','4',0x88,0xe7,0x1f,0x00,0x12,'/'
};

//secp256r1_prv_key: '\xf9.\xbc\xc5\xb1\xcb\x8c\x08Gbc-L\xf1\x96\xccr\xed\x91\xaf\x9e!\x02\xf2\xefhL\xbcr\xf5l\x01'
uint8_t secp256r1_prv_key_str[] = {
  0xf9,'.',0xbc,0xc5,0xb1,0xcb,0x8c,0x08,
  'G','b','c','-','L',0xf1,0x96,0xcc,
  'r',0xed,0x91,0xaf,0x9e,'!',0x02,0xf2,
  0xef,'h','L',0xbc,'r',0xf5,'l',0x01
};

//secp256r1_pub_key: '\x90\xa6\xbc\xe8\x00W\xc0e\xe9\x8a\\\x05(d\x9a\x99y\xc1\x10\x0f\xf8\x8a\xd0IU\xaa\xbf\xbb\x1b\\\xe2\xab9W\x89\x96\xb5\xee:\xf9_\xd3\x89\x15\xdc3\x7fg\xcaRb\t\xbe\x88Y\xe2\xbc\xcf\xbd\xd4\x18\xdd8\x01'

uint8_t secp256r1_pub_key_str[64] = {
  0x90,0xa6,0xbc,0xe8,0x00,'W',0xc0,'e',
  0xe9,0x8a,'\\',0x05,'(','d',0x9a,0x99,
  'y',0xc1,0x10,0x0f,0xf8,0x8a,0xd0,'I',
  'U',0xaa,0xbf,0xbb,0x1b,'\\',0xe2,0xab,
  '9','W',0x89,0x96,0xb5,0xee,':',0xf9,
  '_',0xd3,0x89,0x15,0xdc,3,0x7f,'g',
  0xca,'R','b','\t',0xbe,0x88,'Y',0xe2,
  0xbc,0xcf,0xbd,0xd4,0x18,0xdd,'8',0x01
};

//HERE TO SET pre-shared secrets
uint8_t hmac_key_str[] = {
    0x54,0x69,0xB8,0xC0,0xB6,0x28,0x77,0x70,
    0x1C,0xDD,0xE8,0x89,0x92,0x03,0xFD,0xDE
};

// HERE TO SET capability
uint8_t * capability;

bool running;

int parseArgs(int argc, char *argv[]){
  if(argc < 2){
    fprintf(stderr, "ERROR: wrong arguments.\n");
    printf("<device-identifier>\n");
    return 1;
  }
  device_identifier = argv[1];
  return 0;
}

void
after_bootstrapping()
{
  printf("Bootstrapping Ends");
  return;
}

int main(int argc, char *argv[]){
  ndn_unix_face_t *face;
  // ndn_udp_face_t *face;
  ndn_encoder_t encoder;
  int ret;

  if((ret = parseArgs(argc, argv)) != 0){
    return ret;
  }

  ndn_lite_startup();

  //set up routes
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR,true);
  //face = ndn_udp_unicast_face_construct(INADDR_ANY, htons((uint16_t) 2000), inet_addr("224.0.23.170"), htons((uint16_t) 56363));
  in_port_t multicast_port =  htons((uint16_t) 56363);
  in_addr_t multicast_ip = inet_addr("224.0.23.170");
  // face = ndn_udp_multicast_face_construct(INADDR_ANY, multicast_ip, multicast_port);
  running = true;

  //bootstrapping
  capability = (uint8_t *) malloc(sizeof(uint8_t) * 2);
  capability[0] = 0xaa;
  capability[1] = 0xbb;

  //set up keys
  ndn_ecc_prv_t * ecc_secp256r1_prv_key;
  ndn_ecc_pub_t * ecc_secp256r1_pub_key;
  ndn_key_storage_get_empty_ecc_key(&ecc_secp256r1_pub_key, &ecc_secp256r1_prv_key);
  //ecc prv key
  ndn_ecc_prv_init(ecc_secp256r1_prv_key,
                   secp256r1_prv_key_str,
                   sizeof(secp256r1_prv_key_str),
                   NDN_ECDSA_CURVE_SECP256R1,
                   0);

  ndn_hmac_key_t * hmac_key = ndn_key_storage_get_empty_hmac_key();
  ndn_hmac_key_init(hmac_key,hmac_key_str,sizeof(hmac_key_str),0);

  ndn_security_bootstrapping(&face->intf, ecc_secp256r1_prv_key, hmac_key,
                             device_identifier,strlen(device_identifier),
                             capability,strlen(capability), after_bootstrapping);

  while(running) {
    ndn_forwarder_process();
    usleep(10000);
  }
  ndn_face_destroy(&face->intf);
  return 0;
}
