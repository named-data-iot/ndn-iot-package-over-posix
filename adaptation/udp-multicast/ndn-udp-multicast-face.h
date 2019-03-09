/*
 * Copyright (C) 2019 Xinyu Ma
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3.0. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef NDN_UDP_MULTICAST_FACE_H_
#define NDN_UDP_MULTICAST_FACE_H_

#include <netinet/in.h>
#include "ndn-lite/forwarder/forwarder.h"

#ifdef __cplusplus
extern "C" {
#endif

// This face is different because we can create multiple faces safely

// Generally MTU < 2048
// Given that we don't cache
#define NDN_UDP_BUFFER_SIZE 4096

/**
 * Udp unicast face
 */
typedef struct ndn_udp_multicast_face {
  /**
   * The inherited interface.
   */
  ndn_face_intf_t intf;

  int sock;
  struct sockaddr_in local_addr;
  struct sockaddr_in group_addr;
  uint8_t buf[4096];
} ndn_udp_multicast_face_t;

ndn_udp_multicast_face_t*
ndn_udp_multicast_face_construct(
  uint16_t face_id,
  in_addr_t local_addr,
  in_port_t port,
  in_addr_t group_addr);

// TODO: Exploit msg-queue to do it.
int
ndn_udp_multicast_face_recv(ndn_udp_multicast_face_t* self);

#ifdef __cplusplus
}
#endif

#endif