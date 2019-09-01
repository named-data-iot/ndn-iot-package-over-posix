set(DIR_APP_SUPPORT "${DIR_NDN_LITE}/app-support")
target_sources(ndn-lite PUBLIC
  ${DIR_APP_SUPPORT}/access-control.h
  ${DIR_APP_SUPPORT}/service-discovery.h
  ${DIR_APP_SUPPORT}/security-bootstrapping.h
  ${DIR_APP_SUPPORT}/ndn-sig-verifier.h
)
target_sources(ndn-lite PRIVATE
  ${DIR_APP_SUPPORT}/access-control.c
  ${DIR_APP_SUPPORT}/service-discovery.c
  ${DIR_APP_SUPPORT}/security-bootstrapping.c
  ${DIR_APP_SUPPORT}/ndn-sig-verifier.c
)
unset(DIR_APP_SUPPORT)
