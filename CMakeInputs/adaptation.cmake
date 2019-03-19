target_sources(ndn-lite PUBLIC
  ${DIR_ADAPTATION}/adapt-consts.h
  ${DIR_ADAPTATION}/udp/udp-face.h
)
target_sources(ndn-lite PRIVATE
  ${DIR_ADAPTATION}/uniform-time.c
  ${DIR_ADAPTATION}/udp/udp-face.c
)
