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
#include "ndn-lite/app-support/access-control.h"
#include "ndn-lite/app-support/pub-sub.h"
#include "ndn-lite/security/ndn-lite-rng.h"

#define ENABLE_NDN_LOG_INFO 1
#define ENABLE_NDN_LOG_DEBUG 1
#define ENABLE_NDN_LOG_ERROR 1
#include "ndn-lite/util/logger.h"
#include "services.h"
#include "bootstrap-helper.h"

bool running;
static uint8_t buffer[1024];

typedef struct {
  uint64_t last_battery;
  uint32_t last_motion;
  uint64_t timestamp; /* to avoid overflow on mac */
} state_t;

state_t state = {100, 0, 0};

/* Motion Sensor
 *
 * Certificate: /ndn-iot/motion_sensor9/KEY
 * Locator: /ndn-iot/living/motion_sensor9
 * Services: 
 *   Battery:
 *      uint32_t [1, 100]
 *   Motion: 
 *      uint32_t [1, 100] 
 * 
 * Signing Key:
 *   Battery: /ndn-iot/motion_sensor9/KEY/<keyID>/battery
 *   Motion: /ndn-iot/motion_sensor9/KEY/<keyID>/motion
 * 
 * Implementation: 
 *   Signing Key:
 *     Different KeyName but same key bits
 *   Pub Interval:
 *     Battery: publish only if drop one percent
 *     Motion: publish only if Value - lastValue > 10 
 */

void service_data_publishing()
{
  /* simulate the battery usage */
  usleep(1000000);
  uint64_t now = ndn_time_now_ms();
  uint64_t elapsed = now - state.timestamp;
  uint32_t battery = 100 - elapsed / 600;
  if (battery < state.last_battery) {
    state.last_battery =  battery;
    state.timestamp = now;
    NDN_LOG_DEBUG("battery info changes, now is %u percent\n", battery);
    ps_publish_content(NDN_SD_BATTERY, "content-id", strlen("content-id"), (uint8_t*)&battery, sizeof(battery));
  }
  
  /* simulate the motion */
  uint8_t dice;
  ndn_rng((uint8_t*)&dice, sizeof(dice));
  uint32_t motion = dice / 3;
  if (motion - state.last_motion > 10) {
    NDN_LOG_DEBUG("motion info changes, now is %d percent\n", motion);
    ps_publish_content(NDN_SD_MOTION, "content-id", strlen("content-id"), (uint8_t*)&motion, sizeof(motion));
  }
}


void on_motion_command(uint8_t service, bool is_cmd, const name_component_t* identifiers, uint32_t identifiers_size,
                       const uint8_t* suffix, uint32_t suffix_len, const uint8_t* content, uint32_t content_len,
                       void* userdata)
{
  NDN_LOG_DEBUG("on motion command");

}


/* subscribe to the command namespace
 * /ndn-iot/Motion/CMD/<...>/OFF
 * /ndn-iot/Motion/CMD/<...>/ON
 *
 *
 */
void initialize()
{
  ndn_ac_register_encryption_key_request(NDN_SD_MOTION);
  ndn_ac_register_encryption_key_request(NDN_SD_BATTERY);

  /* inject the access keys */
  ndn_access_control_t* ac_state = ndn_ac_get_state();
  uint8_t value[30] = {8};
  ndn_aes_key_t* key = NULL;
  ndn_time_ms_t now = ndn_time_now_ms();
  uint32_t keyid;
  for (int i = 0; i < 10; i++) {
    if (ac_state->self_services[i] == NDN_SD_MOTION ||
        ac_state->self_services[i] == NDN_SD_BATTERY) {
      ndn_rng((uint8_t*)&keyid, 4);
      ac_state->ekeys[i].key_id = keyid;
      ac_state->ekeys[i].expires_at = 40000 + now;
      key = ndn_key_storage_get_empty_aes_key();
      ndn_aes_key_init(key, value, NDN_AES_BLOCK_SIZE, keyid);
    }
  }

  /* subscribe to ON and OFF */
  //ps_subscribe_to_command(NDN_SD_MOTION, NULL, 0, on_motion_command, NULL);
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

  state.timestamp = ndn_time_now_ms();
  running = true;

  initialize();

  while(running) {
    service_data_publishing();
    ndn_forwarder_process();
    usleep(1000);
  }
  ndn_face_destroy(&face->intf);
  return 0;
}