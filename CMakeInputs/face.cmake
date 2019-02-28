set(DIR_FACE "${DIR_NDN_LITE}/face")
target_sources(ndn-lite PUBLIC
  ${DIR_FACE}/direct-face.h
  ${DIR_FACE}/dummy-face.h
)
target_sources(ndn-lite PRIVATE
  ${DIR_FACE}/direct-face.c
  ${DIR_FACE}/dummy-face.c
)
unset(DIR_FACE)
