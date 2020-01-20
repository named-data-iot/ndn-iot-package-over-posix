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

#define ENABLE_NDN_LOG_INFO 1
#define ENABLE_NDN_LOG_DEBUG 1
#define ENABLE_NDN_LOG_ERROR 1
#include "ndn-lite/util/logger.h"

#include "ndn-lite/encode/interest.h"
#include "ndn-lite/app-support/pub-sub.h"
#include "ndn-lite/app-support/access-control.h"
#include "services.h"
#include "bootstrap-helper.h"


bool running;
static uint8_t buffer[1024];

typedef struct {
  uint8_t lastStatus;
  uint64_t motionStopTime;
} state_t;

state_t state = {1, 0};

/* variables should provided by users*/
uint32_t delayMinutes = 0;

/* Translation from SmartApp Logic 
 *
 * Descriptions: 
 *   Turns on lights when it's dark and motion is detected. 
 *   Turns lights off when it becomes light or some time after motion ceases.
 * 
 * Translation Details:
 *   1. Motion sensor Data have two types: "inactive" and "active". the Data are differentiated with uint32_t threshold.
 *      uint32_t < 50 is "inactive", uint32_t  > 50 is "active".
 *   2. Illuminance sensor should publish uint32_t content to indicate the illuminance. uint32_t < 30 turning on, uint32_t > 50 turning off. 
 *   3. User should input delayMinutes to set the time gap between motion ceases and light turning off. By default it's set to 0,
 *      Also the part to perform the scheduled turning off is absent, so the turning off happend immediately when motion ceasing detected.
 *   4. lastStatus = 1 indicates lastStatus = "on", while 0 indicating "off".
 *
 */
void on_illuminance_publish(uint8_t service, bool is_cmd, const name_component_t* identifiers, uint32_t identifiers_size,
               const uint8_t* suffix, uint32_t suffix_len, const uint8_t* content, uint32_t content_len,
               void* userdata)
{
  NDN_LOG_DEBUG("state.motionStopTime = %lu ms, status.lastStatus = %d", state.motionStopTime, state.lastStatus);
  uint32_t illuminance = *(uint32_t*)content;
  if (state.lastStatus && illuminance > 50) {
    uint8_t command_id = 112;
    /* turnning off all lights if possible, let user policy decide which acutally to be turned on */
    ps_publish_command(NDN_SD_LED, &command_id, sizeof(command_id), NULL, 0, &command_id, sizeof(command_id)); 
    state.lastStatus = 0;
  }
  else if (state.motionStopTime) {
    if (state.lastStatus) {
      uint64_t elapsed = ndn_time_now_ms() - state.motionStopTime;
      NDN_LOG_DEBUG("%lu ms elapsed time from motionStopTime", elapsed);
      if (elapsed > delayMinutes * 60000L) {
        NDN_LOG_DEBUG("turning off lights");
        uint8_t command_id = 112;
        /* turnning off all lights if possible, let user policy decide which acutally to be turned on */
        ps_publish_command(NDN_SD_LED, &command_id, sizeof(command_id), NULL, 0, &command_id, sizeof(command_id));
        state.lastStatus = 0;
       }
    }
  }
  else if (state.lastStatus != 1 && illuminance < 30) {
    uint8_t command_id = 111;
    /* turnning on all lights if possible, let user policy decide which acutally to be turned on */
    ps_publish_command(NDN_SD_LED, &command_id, sizeof(command_id), NULL, 0, &command_id, sizeof(command_id));
    state.lastStatus = 1;
  }
}

void turnOffMotion() 
{
  NDN_LOG_DEBUG("turn off motion at %lu ms, status.lastStatus = %d", state.motionStopTime, state.lastStatus);
  if (state.motionStopTime && state.lastStatus) {
      uint64_t elapsed = ndn_time_now_ms() - state.motionStopTime;
      NDN_LOG_DEBUG("%lu ms elapsed time from motionStopTime", elapsed);
      if (elapsed > delayMinutes * 60000L) {
        NDN_LOG_DEBUG("turning off lights");
        uint8_t command_id = 112;
        /* turnning off all lights if possible, let user policy decide which acutally to be turned on */
        ps_publish_command(NDN_SD_LED, &command_id, sizeof(command_id), NULL, 0, &command_id, sizeof(command_id));
        state.lastStatus = 0;
      }
  }
}

void on_motion_publish(uint8_t service, bool is_cmd, const name_component_t* identifiers, uint32_t identifiers_size,
               const uint8_t* suffix, uint32_t suffix_len, const uint8_t* content, uint32_t content_len,
               void* userdata)
{
  NDN_LOG_DEBUG("motion Data received\n");
  /* if above the threshold */
  uint32_t threshold = 50;
  uint32_t value = *(uint32_t*)content;
  if (value > threshold) {
    NDN_LOG_DEBUG("turnnig on lights due to motion\n");
    uint8_t command_id = 111;
    /* turnning on all lights if possible, let user policy decide which acutally to be turned on */
    ps_publish_command(NDN_SD_LED, &command_id, sizeof(command_id), NULL, 0, &value, sizeof(value));
    state.lastStatus = 1; /* on */
    state.motionStopTime = 0; /* should work like null */ 
  }
  else {
    /* motion stop now */
    state.motionStopTime = ndn_time_now_ms();
    if (delayMinutes) {
      /* should have a scheduler to handle the delay events */
    }
    else {
      turnOffMotion();
    }
  }
}

void initialize()
{
  ndn_ac_register_access_request(NDN_SD_MOTION);
  ndn_ac_register_access_request(NDN_SD_ILLUMINANCE);
  ndn_ac_register_encryption_key_request(NDN_SD_LED);
  // ndn_ac_after_bootstrapping();
  
  /* inject the access keys */
  ndn_access_control_t* ac_state = ndn_ac_get_state();
  uint8_t value[16] = {8};
  ndn_aes_key_t* key = NULL;
  ndn_time_ms_t now = ndn_time_now_ms();
  uint32_t keyid;
  for (int i = 0; i < 10; i++) {
    if (ac_state->access_services[i] == NDN_SD_MOTION ||
        ac_state->access_services[i] == NDN_SD_ILLUMINANCE) {
      ndn_rng((uint8_t*)&keyid, 4);
      ac_state->access_keys[i].key_id = keyid;
      ac_state->access_keys[i].expires_at = 40000 + now;
      key = ndn_key_storage_get_empty_aes_key();
      ndn_aes_key_init(key, value, 16, keyid);
    }
  }

  for (int i = 0; i < 10; i++) {
    if (ac_state->self_services[i] == NDN_SD_LED) {
      ndn_rng((uint8_t*)&keyid, 4);
      ac_state->ekeys[i].key_id = keyid;
      ac_state->ekeys[i].expires_at = 40000 + now;
      key = ndn_key_storage_get_empty_aes_key();
      ndn_aes_key_init(key, value, 16, keyid);
    }
  }

  ps_subscribe_to_content(NDN_SD_MOTION, NULL, 0, 5000, on_motion_publish, NULL);
  ps_subscribe_to_content(NDN_SD_ILLUMINANCE, NULL, 0, 5000, on_illuminance_publish, NULL);
  ps_after_bootstrapping();
}


int main(int argc, char *argv[])
{
  int ret = 0;
  ndn_lite_startup();

  /* Simulate Bootstrapping
   *
   * 1. Setting TrustAnchor /ndn-iot/controller/KEY/123/self/456
   * 2. Install identity certificate named /ndn-iot/Hub/KEY/234/home/567
   * 3. Add route /ndn-iot to the given UDP face
   * 4. Install KeyID-10002 Key as default AES-128 Key.
   */
  ndn_udp_face_t* face = ndn_udp_unicast_face_construct(INADDR_ANY, 6666, inet_addr("127.0.0.1"), 2333);

  if (!face) {
    NDN_LOG_ERROR("Face construction failed\n");
  }
  name_component_t id;
  name_component_from_string(&id, "Hub", strlen("Hub"));
  simulate_bootstrap(&face->intf, &id, 1);

  // initialize
  initialize();

  running = true;
  while(running) {
    usleep(1000);
    ndn_forwarder_process();
  }
  ndn_face_destroy(&face->intf);
  return 0;
}
