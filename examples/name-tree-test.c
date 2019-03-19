#include "ndn-lite/forwarder/name-tree.h"
#include <stdbool.h>

#define test_assert(cond) if(!(cond)) return false

uint8_t nametree_buf[NDN_NAMETREE_RESERVE_SIZE(10)];
ndn_nametree_t *nametree = (ndn_nametree_t*)nametree_buf;

bool test(){
  ndn_nametree_init(nametree, 10);

  // Functional Test
  // Insert
  uint8_t name1[] = "\x07\x17\x08\x03ndn\x08\x09name-tree\x08\x05test1";
  nametree_entry_t *ptr1 = ndn_nametree_find_or_insert(nametree, name1, strlen(name1));
  test_assert(ptr1 != NULL);

  uint8_t name2[] = "\x07\x17\x08\x03ndn\x08\x09name-tree\x08\x05test2";
  nametree_entry_t *ptr2 = ndn_nametree_find_or_insert(nametree, name2, strlen(name2));
  test_assert(ptr2 != NULL);

  uint8_t name3[] = "\x07\x17\x08\x03ndn\x08\x09name-tree\x08\x05test3";
  nametree_entry_t *ptr3 = ndn_nametree_find_or_insert(nametree, name3, strlen(name3));
  test_assert(ptr3 != NULL);

  nametree_entry_t *ptr4 = ndn_nametree_find_or_insert(nametree, name2, strlen(name2));
  test_assert(ptr4 == ptr2);

  uint8_t name5[] = "\x07\x10\x08\x03ndn\x08\x09name-tree";
  nametree_entry_t *ptr5 = ndn_nametree_find_or_insert(nametree, name5, strlen(name5));
  test_assert(ptr5 != NULL);
  test_assert(ptr5->left_child != NDN_INVALID_ID);

  // Match
  nametree_entry_t *ptr6 = ndn_nametree_prefix_match(nametree, name2, strlen(name2), NDN_NAMETREE_FIB_TYPE);
  test_assert(ptr6 == NULL);

  ptr5->fib_id = 1;
  ptr6 = ndn_nametree_prefix_match(nametree, name2, strlen(name2), NDN_NAMETREE_FIB_TYPE);
  test_assert(ptr6 == ptr5);

  ptr6 = ndn_nametree_prefix_match(nametree, name2, strlen(name2), NDN_NAMETREE_PIT_TYPE);
  test_assert(ptr6 == NULL);

  ptr5->pit_id = 1;
  ptr2->pit_id = 2;
  ptr6 = ndn_nametree_prefix_match(nametree, name2, strlen(name2), NDN_NAMETREE_PIT_TYPE);
  test_assert(ptr6 == ptr2);

  // Autoclear test
  int i;
  uint8_t name20[] = "\x07\x03\x08\x01\x00";
  ndn_nametree_init(nametree, 10);
  for(i = 0; i < 10 - 4; i ++){
    name20[4] = i;
    ptr1 = ndn_nametree_find_or_insert(nametree, name20, strlen(name20));
    ptr1->fib_id = 0;
  }
  uint8_t name21[] = "\x07\x10\x08\x03ndn\x08\x09name-tree";
  ptr1 = ndn_nametree_find_or_insert(nametree, name21, strlen(name21));
  test_assert(ptr1 != NULL);
  ptr1->fib_id = 1;

  uint8_t name22[] = "\x07\x10\x08\x03nbn\x08\x09name-tree";
  ptr1 = ndn_nametree_find_or_insert(nametree, name22, strlen(name22));
  test_assert(ptr1 == NULL);

  ptr1 = ndn_nametree_find_or_insert(nametree, name21, strlen(name21));
  test_assert(ptr1 != NULL);
  ptr1->fib_id = NDN_INVALID_ID;

  ptr1 = ndn_nametree_find_or_insert(nametree, name22, strlen(name22));
  test_assert(ptr1 != NULL);

  return true;
}

int main(){
  if(!test())
    return -1;
  return 0;
}