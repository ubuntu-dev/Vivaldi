#ifndef VV_GC_FREE_BLOCK_LIST_H
#define VV_GC_FREE_BLOCK_LIST_H

#include <vector>

namespace vv {

namespace gc {

class free_block_list {
public:
  // Returns a pointer to the first found contiguous block of memory of the
  // provided size, or nullptr if no such block exists.
  void* allocate(size_t size);
  // Reclaims a block of memory returned by allocate. Ensure that size is the
  // same size as was originally passed to allocate; if it's too small, it'll
  // result in fragmentation, and if it's too large the result is basically
  // undefined.
  void reclaim(void* ptr, size_t size);

  // Insert a new block of memory into the free block list
  void insert(void* ptr, size_t size);

private:
  struct free_block {
    size_t size;
    char* ptr;
  };

  std::vector<free_block> m_list;
};

}

}

#endif
