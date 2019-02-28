set(DIR_APP_SUPPORT "${DIR_NDN_LITE}/app-support")
set(DIR_BOOTSTRAPPING "${DIR_APP_SUPPORT}/bootstrapping")
set(DIR_SECURE_SIGN_ON "${DIR_BOOTSTRAPPING}/secure-sign-on")
set(DIR_SECURITY "${DIR_SECURE_SIGN_ON}/security")
set(DIR_VARIANTS "${DIR_SECURE_SIGN_ON}/variants")
set(DIR_ECC256 "${DIR_VARIANTS}/ecc_256")
target_sources(ndn-lite PUBLIC
  ${DIR_APP_SUPPORT}/access-control.h
  ${DIR_APP_SUPPORT}/bootstrapping.h
  ${DIR_APP_SUPPORT}/service-discovery.h
)
target_sources(ndn-lite PRIVATE
  ${DIR_APP_SUPPORT}/access-control.c
  ${DIR_APP_SUPPORT}/service-discovery.c
  ${DIR_SECURE_SIGN_ON}/sign-on-basic-client-consts.h
  ${DIR_SECURE_SIGN_ON}/sign-on-basic-client.c
  ${DIR_SECURE_SIGN_ON}/sign-on-basic-client.h
  ${DIR_SECURE_SIGN_ON}/sign-on-basic-consts.h
  ${DIR_SECURE_SIGN_ON}/sign-on-basic-impl-consts.h
  ${DIR_SECURE_SIGN_ON}/sign-on-basic-sec-intf-setter.c
  ${DIR_SECURE_SIGN_ON}/sign-on-basic-sec-intf-setter.h
  ${DIR_SECURITY}/sign-on-basic-sec-consts.h
  ${DIR_SECURITY}/sign-on-basic-sec-impl.c
  ${DIR_SECURITY}/sign-on-basic-sec-impl.h
  ${DIR_ECC256}/sign-on-basic-ecc-256-consts.h
  ${DIR_ECC256}/sign-on-basic-ecc-256-sec.c
  ${DIR_ECC256}/sign-on-basic-ecc-256-sec.h
)
unset(DIR_APP_SUPPORT)
unset(DIR_BOOTSTRAPPING)
unset(DIR_SECURE_SIGN_ON)
unset(DIR_SECURITY)
unset(DIR_VARIANTS)
unset(DIR_ECC256)
