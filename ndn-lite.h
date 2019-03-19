/*
 * Copyright (C) 2019 Xinyu Ma
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3.0. See the file LICENSE in the top level
 * directory for more details.
 */
#ifndef NDN_LITE_H
#define NDN_LITE_H

#include "ndn-lite/ndn-constants.h"
#include "ndn-lite/ndn-enums.h"
#include "ndn-lite/ndn-error-code.h"
#include "ndn-lite/ndn-services.h"
#include "ndn-lite/forwarder/forwarder.h"
#include "adaptation/adapt-consts.h"
#include "adaptation/udp/udp-face.h"
#include "ndn-lite/security/ndn-lite-sec-config.h"

// TODO
#include "ndn-lite/encode/new-interest.h"

static void
ndn_lite_startup(){
  ndn_security_init();
  ndn_forwarder_init();
}

#endif // NDN_LITE_H
