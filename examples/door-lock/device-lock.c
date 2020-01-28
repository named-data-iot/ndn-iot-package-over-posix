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
int state;

void on_lock_command(uint8_t service, bool is_cmd, const ps_identifier_t* identifier, ps_content_t content, void* userdata)
{
  NDN_LOG_DEBUG((content.content_id[0] == NDN_CMD_LOCK) ? "on command\n" : "on unlock command\n");
  /* change the lock state */
  state = content.content_id[0] == NDN_CMD_LOCK;
  // service_data_publishing();
}

int main(){
  ndn_lite_startup();
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
  name_component_t id[2];
  name_component_from_string(&id[0], "living", strlen("living"));
  name_component_from_string(&id[1], "lock1", strlen("lock1"));
  simulate_bootstrap(&face->intf, &id[0], 2, 0);
  sd_add_or_update_self_service(NDN_SD_LOCK, true, 1);
  ndn_sd_after_bootstrapping(&face->intf);

  ps_subscribe_to_command(NDN_SD_LOCK, NULL, on_lock_command, NULL);
  printf("Started.\n");

  running = true;
  while(running) {
    ndn_forwarder_process();
    usleep(10000);
  }

  ndn_face_destroy(&face->intf);
  return 0;
}