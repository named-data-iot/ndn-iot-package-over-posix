set(DIR_ENCODE "${DIR_NDN_LITE}/encode")
target_sources(ndn-lite PUBLIC
  ${DIR_ENCODE}/data.h
  ${DIR_ENCODE}/decoder.h
  ${DIR_ENCODE}/encoder.h
  ${DIR_ENCODE}/fragmentation-support.h
  ${DIR_ENCODE}/interest.h
  ${DIR_ENCODE}/key-storage.h
  ${DIR_ENCODE}/metainfo.h
  ${DIR_ENCODE}/name-component.h
  ${DIR_ENCODE}/name.h
  ${DIR_ENCODE}/signature.h
  ${DIR_ENCODE}/signed-interest.h
  ${DIR_ENCODE}/tlv.h
)
target_sources(ndn-lite PRIVATE
  ${DIR_ENCODE}/data.c
  ${DIR_ENCODE}/interest.c
  ${DIR_ENCODE}/key-storage.c
  ${DIR_ENCODE}/metainfo.c
  ${DIR_ENCODE}/name-component.c
  ${DIR_ENCODE}/name.c
  ${DIR_ENCODE}/signature.c
  ${DIR_ENCODE}/signed-interest.c
)
unset(DIR_ENCODE)
