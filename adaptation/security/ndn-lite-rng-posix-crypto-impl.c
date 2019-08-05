/*
 * Copyright (C) 2018-2019
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
ndn_lite_rng_posix_crypto(uint8_t *dest, unsigned size)
{
  arc4random_buf((void*)dest, size);
  return 0;
}

void
ndn_lite_posix_crypto_rng_load_backend(void)
{
  ndn_rng_backend_t* backend = ndn_rng_get_backend();
  backend->rng = ndn_lite_rng_posix_crypto;
}