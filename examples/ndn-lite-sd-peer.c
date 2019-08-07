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
#include <arpa/inet.h>
#include "../adaptation/udp/udp-face.h"
#include "ndn-lite/forwarder/forwarder.h"
#include "ndn-lite/encode/data.h"
#include "ndn-lite/app-support/service-discovery.h"
#include "ndn-lite/ndn-services.h"

ndn_name_t self_identity;
uint8_t buf[4096];
ndn_udp_face_t *face;
bool running;

int
parseArgs(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "ERROR: wrong arguments.\n");
    printf("Usage: <self-prefix>\n");
    return 1;
  }
  if (ndn_name_from_string(&self_identity, argv[1], strlen(argv[1])) != NDN_SUCCESS) {
    fprintf(stderr, "ERROR: wrong name.\n");
    return 4;
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  // default params
  in_port_t multicast_port = 56363;
  in_addr_t multicast_ip = inet_addr("224.0.23.170");
  int ret;
  if ((ret = parseArgs(argc, argv)) != 0) {
    return ret;
  }

  ndn_lite_startup();
  face = ndn_udp_multicast_face_construct(INADDR_ANY, multicast_ip, multicast_port);
  ndn_sd_init(&self_identity);
  sd_add_or_update_self_service(NDN_SD_LED, true, 1);
  sd_start_adv_self_services();

  running = true;
  while(running){
    ndn_forwarder_process();
    usleep(10000);
  }
  ndn_face_destroy(&face->intf);
  return 0;
}