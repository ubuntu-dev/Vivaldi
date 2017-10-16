#ifndef VV_GC_BLOCK_LIST
#define VV_GC_BLOCK_LIST

#include <array>
#include <bitset>
#include <vector>

namespace vv {

namespace gc {

class managed_ptr;

// This class serves a dual purpose. On one hand, it keeps track of all
// available free space, and provides interfaces for allocating and freeing
// memory. On the other hand, it handles marking for any allocated object. It
// doesn't handle actual marking and sweeping--- that's dealt with in gc.cpp.
// There's no particular reason for that separation, though (it just so happens
// that the various functions this class handles are really interconnected, so
// it's more convenient to glom them together). TODO: Move more GC functionality
// to this class, so we can get rid of some of the remaining GC-related globals.
class block_list {
public:
  block_list();

  // Returns true if heap-allocated pointer ptr is marked, and false otherwise.
  // Behavior is undefined if ptr wasn't allocated by this class.
  bool is_marked(gc::managed_ptr ptr) const;

  // After calling this function, is_marked(ptr) will return true. As above, ptr
  // *must* be allocated by this class (i.e. contains(ptr) must return true).
  void mark(gc::managed_ptr ptr);
  // Unmarks all allocated objects.
  void unmark_all();

  // Provides a pointer to a block of memory of the given size, or nullptr if
  // none can be found (in that case, destruct and call reclaim on unused
  // objects, or expand).
  gc::managed_ptr allocate(size_t size);
  // Re-adds the given block of memory to the free list. The provided memory
  // *must* have originallly been obtained via allocate.
  void reclaim(gc::managed_ptr ptr, size_t size);

  // Expands available memory by ~50% (including currently allocated memory),
  // providing free space if none exists.
  void expand();

  // Tries to release unused memory.
  void shrink_to_fit();

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

  std::vector<std::unique_ptr<block>> m_list;
  std::vector<std::unique_ptr<block>>::iterator m_cur_pos;

  friend class gc::managed_ptr;
};

}

}

#endif
