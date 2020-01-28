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

void service_data_publishing(void *self, size_t param_length, void *param)
{
  static int state = 0;
  static ndn_time_ms_t last = 0;

  ndn_time_ms_t now = ndn_time_now_ms();
  if (now - last > 2000){
    last = now;
    state = !state;
    /* publish presence state */
    printf("Publish presence: %d\n", state);
    fflush(stdout);

    ps_content_t content = {
      .payload = (uint8_t*)&state,
      .payload_len = sizeof(state)
    };
    if(state != 0){
      content.content_id = (uint8_t*)"present";
      content.content_id_len = strlen("present");
    }else{
      content.content_id = (uint8_t*)"absent";
      content.content_id_len = strlen("absent");
    }

    ps_publish_content(NDN_SD_PRESENCE, content);
  }

  ndn_msgqueue_post(NULL, service_data_publishing, 0, NULL);
}

int main(){
  ndn_lite_startup();
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
  name_component_t id[2];
  name_component_from_string(&id[0], "living", strlen("living"));
  name_component_from_string(&id[1], "presence1", strlen("presence1"));
  simulate_bootstrap(&face->intf, &id[0], 2, 0);
  // sd_add_or_update_self_service(NDN_SD_PRESENCE, true, 1);
  // ndn_sd_after_bootstrapping(&face->intf);

  printf("Started.\n");

  ndn_msgqueue_post(NULL, service_data_publishing, 0, NULL);

  running = true;
  while(running) {
    ndn_forwarder_process();
    usleep(10000);
  }

  ndn_face_destroy(&face->intf);
  return 0;
}