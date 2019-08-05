/*
 * Copyright (C) 2018-2019 Zhiyi Zhang
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * See AUTHORS.md for complete list of NDN IOT PKG authors and contributors.
 */

#include "ndn-lite-rng-posix-crypto-impl.h"
#include <ndn-lite/security/ndn-lite-rng.h>
#if defined(__linux__)
  #include <bsd/stdlib.h>
#elif defined(__APPLE__)
  #include <stdlib.h>
#endif

int
ndn_lite_posix_rng(uint8_t *dest, unsigned size)
{
  arc4random_buf((void*)dest, size);
  return 1;
}

void
ndn_lite_posix_rng_load_backend(void)
{
  ndn_rng_backend_t* backend = ndn_rng_get_backend();
  backend->rng = ndn_lite_posix_rng;
}