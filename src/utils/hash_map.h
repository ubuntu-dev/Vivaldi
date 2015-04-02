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
    size_t size;
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
    : m_buckets ( std::max(init.size() / 6, size_t{6}) )
  {
    for (const auto& i : init)
      insert(i.first, i.second);
  }

  size_t size() const
  {
    return accumulate(std::begin(m_buckets), std::end(m_buckets), size_t{},
                      [](auto sz, const auto& b) { return sz + b.size; });
  }

  // Returns 1 if hash_map contains an element with key item, and 0 otherwise.
  size_t count(const K& item) const
  {
    if (!m_buckets.size())
      return false;
    const auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];
    return any_of(std::begin(bucket.slots), std::end(bucket.slots),
                  [&item](const auto& i) { return i.first == item; });
  }

  // Returns an iterator pointing to the element with key item, or end() if no
  // such element exists.
  iterator find(const K& item)
  {
    if (!m_buckets.size())
      return end();
    auto bucket = std::begin(m_buckets) + s_hash(item) % m_buckets.size();
    auto iter = std::find_if(std::begin(bucket->slots), std::end(bucket->slots),
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
    if (m_buckets.size()) {
      auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];
      auto slot = find_if(std::begin(bucket.slots), std::end(bucket.slots),
                          [&](const auto& i) { return i.first == item; });
      if (slot != std::end(bucket.slots)) {
        slot->second = std::move(val);
        return;
      }

      if (bucket.size < 6) {
        ++bucket.size;
        bucket.slots.emplace_front(std::move(item), std::move(val));
        return;
      }
    }

    rehash();
    auto& nbucket = m_buckets[s_hash(item) % m_buckets.size()];
    ++nbucket.size;
    nbucket.slots.emplace_front(std::move(item), std::move(val));
  }

  // Returns (and, if needed, constructs) the value at key `item`.
  // Since this method will insert elements if necessary, it can invalidated
  // all iterators.
  V& operator[](const K& item)
  {
    if (m_buckets.size()) {
      auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];

      auto slot = find_if(std::begin(bucket.slots), std::end(bucket.slots),
                          [&](const auto& i) { return i.first == item; });
      if (slot != std::end(bucket.slots))
        return slot->second;

      if (bucket.size < 6) {
        ++bucket.size;
        bucket.slots.emplace_front(item, V{});
        return bucket.slots.front().second;
      }
    }

    rehash();
    auto& nbucket = m_buckets[s_hash(item) % m_buckets.size()];
    ++nbucket.size;
    nbucket.slots.emplace_front(item, V{});
    return nbucket.slots.front().second;
  }

  // Returns the value at key item. If no item exists at key item, an
  // std::out_of_range error is thrown.
  const V& at(const K& item) const
  {
    if (m_buckets.size()) {
      const auto& bucket = m_buckets[s_hash(item) % m_buckets.size()];
      auto slot = find_if(std::begin(bucket.slots), std::end(bucket.slots),
                          [&item](const auto& i) { return i.first == item; });
      if (slot != std::end(bucket.slots))
        return slot->second;
    }

    throw std::out_of_range{"no such item in hash_map"};
  }

  iterator begin()
  {
    auto first_nonempty = find_if(std::begin(m_buckets), std::end(m_buckets),
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
      i.size = 0;
    }
    m_buckets.resize(m_buckets.size() + 6);

    for (auto& i : members) {
      auto& bucket = m_buckets[s_hash(i.first) % m_buckets.size()];
      ++bucket.size;
      bucket.slots.emplace_front(std::move(i));
    }
  }
};

template <typename K, typename V>
const std::hash<K> hash_map<K, V>::s_hash{};

}

#endif
