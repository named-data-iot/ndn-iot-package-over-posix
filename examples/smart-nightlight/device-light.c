/*
 * Copyright (C) 2020 Tianyuan Yu
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
#include "ndn-lite/util/logger.h"
#include "ndn-lite/security/ndn-lite-rng.h"

#define ENABLE_NDN_LOG_INFO 1
#define ENABLE_NDN_LOG_DEBUG 1
#define ENABLE_NDN_LOG_ERROR 1

#include "services.h"

bool running;
static uint8_t buffer[1024];

typedef struct {
  uint32_t last_switich;
  uint32_t last_color;
} state_t;

state_t state = {111, 201};

/* Light
 *
 * Certificate: /ndn-iot/light9/KEY
 * Locator: /ndn-iot/living/light9
 * Services: 
 *   Switich:
 *      uint8_t {On, Off} = {111, 112}
 *   Color: 
 *      uint8_t {Orange, Red, Daylight} = {201, 202, 203} 
 * 
 * Signing Key:
 *   Battery: /ndn-iot/light9/KEY/<keyID>/switich
 *   Motion: /ndn-iot/light9/KEY/<keyID>/color
 * 
 * Implementation: 
 *   Signing Key:
 *     Different KeyName but same key bits
 *   Pub Interval:
 *     Swtich: publish only when state change
 *     Color: publish only when state change
 */

void service_data_publishing()
{
  /* publish switich state */
  NDN_LOG_DEBUG("publishing switich state, now is %d", state.last_switch);
  ps_publish_content(NDN_SD_LED, "state", strlen("state"), (uint8_t*)&state.last_switch, sizeof(state.last_switch));

  /* publish color motion */
  NDN_LOG_DEBUG("publishing color state, now is %d", state.last_color);
  ps_publish_content(NDN_SD_COLOR, "state", strlen("state"), (uint8_t*)&state.last_color, sizeof(state.last_color));
}


void on_switich_command(uint8_t service, bool is_cmd, const name_component_t* identifiers, uint32_t identifiers_size,
                       const uint8_t* suffix, uint32_t suffix_len, const uint8_t* content, uint32_t content_len,
                       void* userdata)
{
  NDN_LOG_DEBUG("on switch command");
  /* flip the switch state */
  uint32_t value = *(uint32_t*)content;
  state.last_switch = (value == 111)? 112 : 111;
  service_data_publishing();
}


/* subscribe to the command namespace
 * /ndn-iot/Motion/CMD/<...>/OFF
 * /ndn-iot/Motion/CMD/<...>/ON
 *
 *
 */
void initialize()
{
  /* subscribe to ON and OFF */
  ps_subscribe_to_command(NDN_SD_MOTION, NULL, 0, on_motion_command, NULL);
}



int main(int argc, char *argv[])
{
  int ret = 0;
  ndn_lite_startup();

  /* Simulate Bootstrapping
   *
   * 1. Setting TrustAnchor /ndn-iot/controller/KEY/123/self/456
   * 2. Install identity certificate named /ndn-iot/living/motion_sensor9/KEY/234/home/567
   * 3. Add route /ndn-iot to the given UDP face
   * 4. Install KeyID-10002 Key as default AES-128 Key.
   */
  ndn_udp_face_t* face = ndn_udp_unicast_face_construct(INADDR_ANY, 2333, inet_addr("127.0.0.1"), 6666);
  if (!face) {
    NDN_LOG_ERROR("Face construction failed\n");
  }
  name_component_t id[2];
  name_component_from_string(&id[0], "living", strlen("living"));
  name_component_from_string(&id[1], "motion_sensor9", strlen("motion_sensor9"));
  simulate_bootstrap(&face->intf, &id, 2);

  running = true;
  while(running) {
    service_data_publishing();
    ndn_forwarder_process();
    usleep(100000);
  }
  ndn_face_destroy(&face->intf);
  return 0;
}