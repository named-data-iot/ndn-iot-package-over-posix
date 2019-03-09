/*
 * Copyright (C) 2019 Xinyu Ma
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3.0. See the file LICENSE in the top level
 * directory for more details.
 */

#include "ndn-udp-unicast-face.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int
ndn_udp_unicast_face_up(struct ndn_face_intf* self);

int
ndn_udp_unicast_face_down(struct ndn_face_intf* self);

void
ndn_udp_unicast_face_destroy(ndn_face_intf_t* self);

int
ndn_udp_unicast_face_send(ndn_face_intf_t* self, const ndn_name_t* name,
                          const uint8_t* packet, uint32_t size);

ndn_udp_unicast_face_t*
ndn_udp_unicast_face_construct(
  uint16_t face_id,
  in_addr_t local_addr,
  in_port_t local_port,
  in_addr_t remote_addr,
  in_port_t remote_port)
{
  ndn_udp_unicast_face_t* ret;

  ret = (ndn_udp_unicast_face_t*)malloc(sizeof(ndn_udp_unicast_face_t));
  if(!ret){
    return NULL;
  }

  ret->intf.face_id = face_id;
  ret->intf.type = NDN_FACE_TYPE_NET;
  ret->intf.state = NDN_FACE_STATE_DOWN;
  ret->intf.up = ndn_udp_unicast_face_up;
  ret->intf.down = ndn_udp_unicast_face_down;
  ret->intf.send = ndn_udp_unicast_face_send;
  ret->intf.destroy = ndn_udp_unicast_face_destroy;

  ret->local_addr.sin_family = AF_INET;
  ret->local_addr.sin_port = local_port;
  ret->local_addr.sin_addr.s_addr = local_addr;
  memset(ret->local_addr.sin_zero, 0, sizeof(ret->local_addr.sin_zero));

  ret->remote_addr.sin_family = AF_INET;
  ret->remote_addr.sin_port = remote_port;
  ret->remote_addr.sin_addr.s_addr = remote_addr;
  memset(ret->remote_addr.sin_zero, 0, sizeof(ret->remote_addr.sin_zero));

  ret->sock = -1;
  ndn_face_up(&ret->intf);

  return ret;
}

int
ndn_udp_unicast_face_up(struct ndn_face_intf* self)
{
  ndn_udp_unicast_face_t* ptr = (ndn_udp_unicast_face_t*)self;
  int iyes = 1;

  if(self->state == NDN_FACE_STATE_UP){
    return NDN_SUCCESS;
  }
  ptr->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(ptr->sock == -1){
    return -1; //TODO: Error code
  }
  setsockopt(ptr->sock, SOL_SOCKET, SO_REUSEADDR, &iyes, sizeof(int));
  if(ioctl(ptr->sock, FIONBIO, (char *)&iyes) == -1){
    ndn_face_down(self);
    return -1; //TODO: Error code
  }

  if(bind(ptr->sock, (struct sockaddr*)&ptr->local_addr, sizeof(ptr->local_addr)) == -1){
    ndn_face_down(self);
    return -1; //TODO: Error code
  }

  self->state = NDN_FACE_STATE_UP;
  return NDN_SUCCESS;
}

int
ndn_udp_unicast_face_down(struct ndn_face_intf* self)
{
  ndn_udp_unicast_face_t* ptr = (ndn_udp_unicast_face_t*)self;
  self->state = NDN_FACE_STATE_DOWN;
  close(ptr->sock);
  return NDN_SUCCESS;
}

void
ndn_udp_unicast_face_destroy(ndn_face_intf_t* self)
{
  ndn_face_down(self);
  free(self);
}

int
ndn_udp_unicast_face_send(ndn_face_intf_t* self, const ndn_name_t* name,
                          const uint8_t* packet, uint32_t size)
{
  ndn_udp_unicast_face_t* ptr = (ndn_udp_unicast_face_t*)self;
  ssize_t ret;
  ret = sendto(ptr->sock, packet, size, 0, 
               (struct sockaddr*)&ptr->remote_addr, sizeof(ptr->remote_addr));
  if(ret != size){
    return -1; //TODO: Error code
  }else{
    return NDN_SUCCESS;
  }
}

int
ndn_udp_unicast_face_recv(ndn_udp_unicast_face_t* self)
{
  struct sockaddr_in client_addr;
  socklen_t addr_len;
  ssize_t size;
  int ret;

  while(true){
    size = recvfrom(self->sock, self->buf, sizeof(self->buf), MSG_DONTWAIT,
                  (struct sockaddr*)&client_addr, &addr_len);
    if(size >= 0){
      // A packet recved
      ret = ndn_face_receive(&self->intf, self->buf, size);
      if(ret != NDN_SUCCESS){
        return ret;
      }
    }else if(size == -1 && errno == EWOULDBLOCK){
      // No more packet
      return NDN_SUCCESS;
    }else{
      return -1; //TODO: Error code
    }
  }
}
