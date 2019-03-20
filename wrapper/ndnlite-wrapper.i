%module(directors="1") pyndnlite
%{
#include "ndn-lite.h"
%}

%include "carrays.i"

%include "ndn-lite.h"
%include "ndn-lite/ndn-constants.h"
%include "ndn-lite/ndn-enums.h"
%include "ndn-lite/ndn-error-code.h"
%include "ndn-lite/ndn-services.h"
%include "ndn-lite/forwarder/callback-funcs.h"
%include "ndn-lite/forwarder/forwarder.h"
%include "adaptation/adapt-consts.h"
%include "adaptation/udp/udp-face.h"
%include "ndn-lite/security/ndn-lite-sec-config.h"

typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;

%array_class(unsigned char, byteArray);

%feature("director") OnInterestFunc;
%inline %{
struct OnInterestFunc {
  virtual int handle(const uint8_t* interest, uint32_t interest_size) = 0;
  virtual ~OnInterestFunc() {}
};
%}
%{
static int __on_interest_helper(const uint8_t* interest, uint32_t interest_size, void* userdata){
  OnInterestFunc* __on_interest_handler = reinterpret_cast<OnInterestFunc*>(userdata);
  return __on_interest_handler->handle(interest, interest_size);
}
%}

%inline %{
int
ndn_forwarder_register_prefix_wrapper(uint8_t* prefix,
                                      size_t length,
                                      OnInterestFunc* on_interest)
{
  return ndn_forwarder_register_prefix(prefix, length, __on_interest_helper, on_interest);
}
%}
