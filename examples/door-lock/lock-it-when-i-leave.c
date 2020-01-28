#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <ndn-lite.h>
#include <ndn-lite/app-support/pub-sub.h>
#include <ndn-lite/app-support/service-discovery.h>
#include "../smart-nightlight/bootstrap-helper.h"

const int NDN_SD_PRESENCE = 25;
const int NDN_SD_LOCK = 26;
const int NDN_CMD_LOCK = 1;
const int NDN_CMD_UNLOCK = 2;
const bool unlock = true;

ndn_unix_face_t *face;
bool running;
uint8_t buffer[4096];

void presence(uint8_t service, bool is_cmd, const ps_identifier_t* identifier, ps_content_t content, void* userdata)
{
  if (is_cmd){
    return;
  }

  ps_content_t cmd_content = {
    .payload = NULL,
    .payload_len = 0
  };

  if(content.payload_len >= 7 && strcmp((char*)content.payload, "present") == 0){
    if(unlock){
      // TODO: How to keep all records of locks. SD is not usable.
      printf("Unlock.\n");
      uint8_t command_id = NDN_CMD_UNLOCK;
      cmd_content.content_id = &command_id;
      cmd_content.content_id_len = sizeof(command_id);
      ps_publish_command(NDN_SD_LOCK, NULL, cmd_content);
    }
    // TODO: No response to verify whether the command is successful.
  }else{
    printf("Lock.\n");
    // TODO: How to query how many people are present. Query is not supported by pub-sub.
    uint8_t command_id = NDN_CMD_LOCK;
    cmd_content.content_id = &command_id;
    cmd_content.content_id_len = sizeof(command_id);
    ps_publish_command(NDN_SD_LOCK, NULL, cmd_content);
  }
}

int main(){
  ndn_lite_startup();
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
  name_component_t id;
  name_component_from_string(&id, "Hub", strlen("Hub"));
  simulate_bootstrap(&face->intf, &id, 1, 1);

  ps_subscribe_to_content(NDN_SD_PRESENCE, NULL, 5000, presence, NULL);
  // ndn_sd_after_bootstrapping(&face->intf);
  // sd_add_interested_service(NDN_SD_LOCK);

  printf("Started.\n");
  running = true;
  while(running) {
    ndn_forwarder_process();
    usleep(10000);
  }

  ndn_face_destroy(&face->intf);
  return 0;
}