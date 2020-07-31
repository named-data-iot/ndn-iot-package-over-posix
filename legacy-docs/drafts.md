Design Drafts of the NDN-Lite Forwarder (**Discarded**)
=======================================

Occasional Dead Nonce List
--------------------------
Nonce is used to eliminate Interest loop when the looping time is longer than
its lifetime and no corresponding Data. However this is a very rare case, since
the default lifetime is 4s, and the access time from LA to Shanghai is about
200ms (non official data by S.K.S). Especially, our work is used for local network
of constrained devices. It's reasonable to ignore nonces.

But there's another solution, to exploit the garbage collecting of Name Tree,
which is an occasional event. When a PIT entry is freed, put the nonce into the
Name Tree entry, which is freed but not collected. Then if an Interest carrying
the same name comes, new PIT entry will be created at the same place. We can
get the nonce back and tell if it's a looped Interest.

Currently I gave a more simplified implementation---randomly pick a PIT and
check the nonce. (Randomised Nonce List) I don't think it works but no harm.

Software-Defined Strategy
-------------------------
One of the biggest difference between NDN-Lite and NFD is the position of
applications. In NDN-Lite, applications connect to the forwarder directly
without a face, and the OnInterest callback is executed before forwarding
the Interest. So the application who registered the prefix can control the
forwarding of Interests under it. Regardless of the security, a software-
defined forwarding strategy can be implemented.

Currently the OnInterest can only control whether to suppress or multicast it.

Differential Name Tree
----------------------
Current name tree is not scalable due to the limitation on the length of name
components. This can be overcome by adding an `overlap` field into the entry.
The `overlap` means the length of the common prefix shared with its left
brother. The use `overlap` enables a differential coding.
```C++
struct entry{
  uint8_t val[COMPONENT_LEN];
  uint8_t overlap;
  entry* left_child;
  entry* right_bro;
};
string entry::real_name(){
  return left_bro()->real_name()[0:overlap] + string(val);
}
```
For example, we have two names `/ndn/3C15C2B80252` and `/ndn/3C15C2B80264`, and
let `COMPONENT_LEN=5`. The tree will be:
```
("\x08\x03ndn", 0)
    |
    |
("\x08\x0b3C1", 0) -> ("5C2B8", 5) -> ("0252", 10) -> ("64", 12)
```
The last two nodes in the graph are what we want.\
This also accelerate string comparing.

Content Store Draft
-------------------
Though the entries in current NameTree is hard-coded, we can make it flexable:
```C++
struct nametree_entry{
  // NDN_NAMETREE_ENTRY_TYPE_CNT is the last entry in NDN_NAMETREE_ENTRY_TYPE.
  ndn_table_id_t entries[NDN_NAMETREE_ENTRY_TYPE_CNT];
};
```
We can use compiler flags to control `NDN_NAMETREE_ENTRY_TYPE`.
I don't have a very clear idea on how to implement the content store.
Maybe we can only implement it on platforms that support malloc().
Or do it only in FIFO, since a queue can support variable emelent sizes.
CS is important when we want to extend NDN-Lite to mobile scenarios, though
currently we only focus on Home IoT.

Thread Safety of MsgQueue
-------------------------
Current MsgQueue is not thread-safe. A mutex can give the thread safety but
still not reentrancy, which may be more important. The only solution for
reentrancy is to only allow one task writing and the other reading. So we need
a different queue for faces. It seems that no atomic operation is needed.
```C++
struct OneWayQueue{
  uint8_t buf[BUF_SIZE];
  int32_t roar;
  int32_t head;
}

void PutDataToNormalSide(uint8_t* data, uint32_t len){
  // Only called from ISR side, won't be interrupted.
  // Codes dealing with rewind are omitted
  memcpy(buf + roar, data, len);
  roar += len;
}

uint32_t ReadDataFromISRSide(uint8_t* data, uint32_t len){
  int32_t datasize = int32_t(roar) - int32_t(head); // Safe, read only
  if(datasize == 0) return 0;
  if(datasize < 0) datasize += BUF_SIZE;
  memcpy(data, buf + head, datasize); // Safe, data won't be changed
  return datasize;
}
```
