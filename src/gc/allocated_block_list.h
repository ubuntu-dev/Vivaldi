#ifndef VV_GC_ALLOCATED_BLOCK_LIST_H
#define VV_GC_ALLOCATED_BLOCK_LIST_H

#include "value.h"

#include <bitset>
#include <map>

namespace vv {

namespace gc {

class allocated_block_list {
public:
  bool contains(void* pos);
  bool marked(void* pos);

  void mark(void* pos);
  void unmark();

  void insert_block(void* block_start);

private:
  std::map<void*, std::bitset<8'192>> m_blocks;
};

}

}

#endif
