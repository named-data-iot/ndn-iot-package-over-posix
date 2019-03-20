%module pyndnlite
%{
#include "ndn-lite.h"
%}
#include "ndn-lite.h"

#define TESTVAR 103

typedef int (*ndn_on_interest_func)(const uint8_t* interest,
                                    uint32_t interest_size,
                                    void* userdata);
typedef void (*ndn_on_data_func)(const uint8_t* data, uint32_t data_size, void* userdata);
typedef void (*ndn_on_timeout_func)(void* userdata);

extern void ndn_forwarder_init(void);
extern void ndn_forwarder_process(void);
extern int ndn_forwarder_add_route(ndn_face_intf_t* face, uint8_t* prefix, size_t length);
extern int ndn_forwarder_remove_route(ndn_face_intf_t* face, uint8_t* prefix, size_t length);
extern int ndn_forwarder_remove_all_routes(uint8_t* prefix, size_t length);
extern int
ndn_forwarder_register_prefix(uint8_t* prefix,
                              size_t length,
                              ndn_on_interest_func on_interest,
                              void* userdata);
extern int
ndn_forwarder_unregister_prefix(uint8_t* prefix, size_t length);
extern int
ndn_forwarder_express_interest(uint8_t* interest,
                               size_t length,
                               ndn_on_data_func on_data,
                               ndn_on_timeout_func on_timeout,
                               void* userdata);
extern int
ndn_forwarder_put_data(uint8_t* data, size_t length);
