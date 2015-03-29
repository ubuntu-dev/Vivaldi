#ifndef VV_GC_MANAGED_PTR_H
#define VV_GC_MANAGED_PTR_H

// Forward declarations {{{

namespace vv {

namespace value {

struct object;

}

namespace gc {

template <typename T>
class managed_ptr;

namespace internal {

struct block;

managed_ptr<value::object> get_next_empty();

}

}

}

// }}}

namespace vv {

namespace gc {

}

}

#endif
