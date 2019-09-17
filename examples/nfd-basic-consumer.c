/*
 * Copyright (C) 2019 Xinyu Ma, Zhiyi Zhang
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
#include <time.h>
#include <ndn-lite.h>
#include "ndn-lite/encode/name.h"
#include "ndn-lite/encode/data.h"
#include "ndn-lite/encode/interest.h"

ndn_name_t name_prefix;
bool running;

int
parseArgs(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(stderr, "ERROR: wrong arguments.\n");
    printf("Usage: <name-prefix>\n");
    return 1;
  }

  if(ndn_name_from_string(&name_prefix, argv[1], strlen(argv[1])) != NDN_SUCCESS){
    fprintf(stderr, "ERROR: wrong name.\n");
    return 4;
  }

  return 0;
}

void
on_data(const uint8_t* rawdata, uint32_t data_size, void* userdata)
{
  ndn_data_t data;
  printf("On data\n");
  if(ndn_data_tlv_decode_digest_verify(&data, rawdata, data_size)){
    printf("Decoding failed.\n");
  }

  data.content_value[data.content_size] = 0;
  printf("It says: %s\n", data.content_value);
  running = false;
}

void
on_timeout(void* userdata)
{
  printf("On timeout\n");
  running = false;
}

int
main(int argc, char *argv[])
{
  ndn_unix_face_t *face;
  ndn_interest_t interest;
  int ret;

  if ((ret = parseArgs(argc, argv)) != 0) {
    return ret;
  }

  ndn_lite_startup();
  srandom(time(0));
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
  ndn_forwarder_add_route_by_name(&face->intf, &name_prefix);

  ndn_interest_from_name(&interest, &name_prefix);
  ndn_interest_set_MustBeFresh(&interest, true);
  ndn_interest_set_CanBePrefix(&interest, true);
  interest.nonce = random();
  ndn_forwarder_express_interest_struct(&interest, on_data, on_timeout, NULL);

  running = true;
  while(running) {
    ndn_forwarder_process();
    usleep(10000);
  }

  ndn_face_destroy(&face->intf);
  return 0;
}
