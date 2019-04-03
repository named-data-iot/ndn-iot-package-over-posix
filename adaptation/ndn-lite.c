#include "ndn-lite.h"
#include <ndn-lite/security/ndn-lite-sec-config.h>

// Temporarily put the helper func here
void
ndn_lite_startup(){
  ndn_security_init();
  ndn_forwarder_init();
}
