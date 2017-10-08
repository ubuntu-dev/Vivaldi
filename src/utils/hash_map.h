#ifndef VV_UTILS_HASH_MAP_H
#define VV_UTILS_HASH_MAP_H

#include <vector>
#include <forward_list>
#include <functional>
#include <numeric>

namespace vv {

// Simple-minded hash map.
// This class is a functional subset of std::unordered_map (with some methods
// renamed), and has substantially worse performance for larger maps. However,
// since most maps created in a running program are either empty or very small
// (e.g. most value::base::member hashes will contain at most one or two items),
// this class's extremely simple rehashing heuristic ends up being a net win.
template <typename K, typename V>
class hash_map {
  struct bucket {
    std::forward_list<std::pair<K, V>> slots;
  };

public:
  class iterator {
  public:

    iterator& operator++()
    {
      ++m_minor;
      if (m_minor == std::end(m_major->slots)) {
        m_major = std::find_if(m_major + 1, m_end,
                               [](const auto& i) { return !i.slots.empty(); });
        if (m_major == m_end)
          m_minor = {};
        else
          m_minor = std::begin(m_major->slots);
      }
      return *this;
    }

    std::pair<K, V>& operator*() const { return *m_minor; }

    std::pair<K, V>* operator->() const { return &*m_minor; }

    bool operator==(const iterator& other) const
    {
      return m_minor == other.m_minor;
    }

    bool operator!=(const iterator& other) const
    {
      return !(other == *this);
    }

  private:
    typename std::vector<bucket>::iterator m_major;
    typename std::forward_list<std::pair<K, V>>::iterator m_minor;
    typename std::vector<bucket>::iterator m_end;

    iterator(decltype(m_major) major, decltype(m_minor) minor, decltype(m_end) end)
      : m_major {major},
        m_minor {minor},
        m_end   {end}
    { }
    friend class hash_map;
  };

  // Constructs an empty hash map.
  hash_map() : m_buckets( 0 ) { }

  // Constructs a hash map with the members of init.
  hash_map(std::initializer_list<std::pair<K, V>> init)
    : m_buckets ( std::max(init.size() / 3, size_t{6}) )
  {
    for (const auto& i : init)
      insert(i.first, i.second);
  }

  bool empty() const
  {
    return m_buckets.empty();
  }

  size_t size() const
  {
    return accumulate(std::begin(m_buckets), std::end(m_buckets), size_t{},
                      [](auto sz, const auto& b)
                        { return sz + distance(std::begin(b.slots), std::end(b.slots)); });
  }

  // Returns 1 if hash_map contains an element with key item, and 0 otherwise.
  size_t count(const K& item) const
  {
    if (empty())
      return 0;
    const auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];
    return any_of(std::begin(bucket.slots), std::end(bucket.slots),
                  [&item](const auto& i) { return i.first == item; });
  }

  // Returns an iterator pointing to the element with key item, or end() if no
  // such element exists.
  iterator find(const K& item)
  {
    if (empty())
      return end();
    const auto bucket = std::begin(m_buckets) + s_hash(item) % m_buckets.size();
    const auto iter = std::find_if(std::begin(bucket->slots), std::end(bucket->slots),
                                   [&item](const auto& i) { return i.first == item; });
    return iter == std::end(bucket->slots) ? end()
                                           : iterator{bucket, iter, std::end(m_buckets)};
  }

  void insert(const K& item, const V& val)
  {
    return insert(K{item}, V{val});
  }

  void insert(const K& item, V&& val)
  {
    return insert(K{item}, std::forward<V>(val));
  }

  void insert(K&& item, const V& val)
  {
    return insert(std::forward<K>(item), V{val});
  }

  // Inserts val at key item.
  // If an element with key item already exists, it's overwritten. This function
  // should run in O(1), but can run in O(n) if using a pathologically bad
  // hashing algorithm or if a rehash is needed.  If an element with key item is
  // overwritten, all iterators are preserved; otherwise, all iterators are
  // invalidated by this method.
  void insert(K&& item, V&& val)
  {
    // Four cases:
    // 1. nonempty, slot already exists; need to replace value
    // 2. nonempty, bucket isn't full; need to append to bucket
    // 3. nonempty, bucket is full; need to rehash
    // 4. empty; need to allocate space
    if (!empty()) {
      auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];
      auto sz = 1;
      auto slot = std::begin(bucket.slots);
      for (;slot != std::end(bucket.slots) && slot->first != item; ++slot)
        ++sz;
      if (slot != std::end(bucket.slots)) {
        // Case 1
        slot->second = std::move(val);
      }
      else if (sz < 6) {
        // Case 2
        bucket.slots.emplace_front(std::move(item), std::move(val));
      }
      else {
        // Case 3
        rehash();
        auto& nbucket = m_buckets[s_hash(item) % m_buckets.size()];
        nbucket.slots.emplace_front(std::move(item), std::move(val));
      }
    }
    else {
      // Case 4
      m_buckets.resize(3);
      auto& nbucket = m_buckets[s_hash(item) % m_buckets.size()];
      nbucket.slots.emplace_front(std::move(item), std::move(val));
    }
  }

  // Returns (and, if needed, constructs) the value at key `item`.
  // Since this method will insert elements if necessary, it can invalidated
  // all iterators.
  V& operator[](const K& item)
  {
    // Four cases:
    // 1. nonempty, slot already exists; need to return value
    // 2. nonempty, bucket isn't full; need to append and return V{} to bucket
    // 3. nonempty, bucket is full; need to rehash and add V{} as appropriate
    // 4. empty; need to allocate space for V{}
    if (!empty()) {
      auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];

      auto sz = 0;
      for (auto&& i : bucket.slots) {
        // Case 1
        if (i.first == item)
          return i.second;
        ++sz;
      }
      if (sz < 6) {
        // Case 2
        bucket.slots.emplace_front(item, V{});
        return bucket.slots.front().second;
      }
      else {
        // Case 3
        rehash();
        auto& nbucket = m_buckets[s_hash(item) % m_buckets.size()];
        nbucket.slots.emplace_front(item, V{});
        return nbucket.slots.front().second;
      }
    }
    else {
      // Case 4
      m_buckets.resize(3);
      auto& nbucket = m_buckets[s_hash(item) % m_buckets.size()];
      nbucket.slots.emplace_front(item, V{});
      return nbucket.slots.front().second;
    }
  }

  // Returns the value at key item. If no item exists at key item, an
  // std::out_of_range error is thrown.
  const V& at(const K& item) const
  {
    if (!empty()) {
      const auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];
      const auto slot = find_if(std::begin(bucket.slots), std::end(bucket.slots),
                                [&item](const auto& i) { return i.first == item; });
      if (slot != std::end(bucket.slots))
        return slot->second;
    }

    throw std::out_of_range{"no such item in hash_map"};
  }

  iterator begin()
  {
    const auto first_nonempty = find_if(std::begin(m_buckets), std::end(m_buckets),
                                        [](const auto& i) { return !i.slots.empty(); });
    if (first_nonempty == std::end(m_buckets))
      return end();
    return {first_nonempty,
            std::begin(first_nonempty->slots),
            std::end(m_buckets)};
  }

  iterator end()
  {
    return {std::end(m_buckets), {}, std::end(m_buckets)};
  }

private:
  std::vector<bucket> m_buckets;
  const static std::hash<K> s_hash;

  void rehash()
  {
    std::forward_list<std::pair<K, V>> members;
    for (auto& i : m_buckets) {
      members.splice_after(members.before_begin(), std::move(i.slots));
    }
    m_buckets.resize(m_buckets.size() + 3);

    for (auto& i : members) {
      auto& bucket = m_buckets[s_hash(i.first) % m_buckets.size()];
      bucket.slots.emplace_front(std::move(i));
    }
  }
};

template <typename K, typename V>
const std::hash<K> hash_map<K, V>::s_hash{};

}

#endif
