#ifndef VV_GC_BLOCK_LIST
#define VV_GC_BLOCK_LIST

#include <array>
#include <bitset>
#include <map>
#include <vector>

namespace vv {

namespace gc {

class block_list {
public:
  block_list();

  bool contains(void* ptr) const;
  bool is_marked(void* ptr) const;

  void mark(void* ptr);
  void unmark_all();

  void* allocate(size_t size);
  void reclaim(void* ptr, size_t size);

  void expand();

private:

  void add_new_block();

  struct free_block {
    size_t size;
    char* data;
  };

  struct block {
    std::array<char, 65'536> block;
    std::bitset<8'192> markings;

    std::vector<free_block> free_list;
    std::vector<free_block>::iterator free_pos;
  };

  std::map<char*, std::unique_ptr<block>> m_list;
  std::map<char*, std::unique_ptr<block>>::iterator m_cur_pos;
};

}

}

#endif
