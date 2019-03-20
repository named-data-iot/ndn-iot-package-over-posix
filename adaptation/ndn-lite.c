#include "ndn-lite.h"

// Temporarily put the helper func here
void
ndn_lite_startup(){
  ndn_security_init();
  ndn_forwarder_init();
}
