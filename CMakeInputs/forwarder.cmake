set(DIR_FORWARDER "${DIR_NDN_LITE}/forwarder")
target_sources(ndn-lite PUBLIC
  ${DIR_FORWARDER}/face.h
  ${DIR_FORWARDER}/fib.h
  ${DIR_FORWARDER}/forwarder.h
  ${DIR_FORWARDER}/memory-pool.h
  ${DIR_FORWARDER}/msg-queue.h
  ${DIR_FORWARDER}/pit.h
)
target_sources(ndn-lite PRIVATE
  ${DIR_FORWARDER}/face.c
  ${DIR_FORWARDER}/forwarder.c
  ${DIR_FORWARDER}/memory-pool.c
  ${DIR_FORWARDER}/msg-queue.c
  ${DIR_FORWARDER}/pit.c
)
unset(DIR_FORWARDER)
