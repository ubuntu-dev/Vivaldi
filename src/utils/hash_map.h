#ifndef VV_UTILS_HASH_MAP_H
#define VV_UTILS_HASH_MAP_H

#include <vector>
#include <forward_list>
#include <functional>

namespace vv {

template <typename K, typename V>
class hash_map {
public:
  class iterator {
  public:

    iterator& operator++()
    {
      ++m_minor;
      if (m_minor == std::end(*m_major)) {
        m_major = std::find_if(m_major + 1, m_end,
                               [](const auto& i) { return !i.empty(); });
        if (m_major == m_end)
          m_minor = {};
        else
          m_minor = std::begin(*m_major);
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
    typename std::vector<std::forward_list<std::pair<K, V>>>::iterator m_major;
    typename std::forward_list<std::pair<K, V>>::iterator m_minor;
    typename std::vector<std::forward_list<std::pair<K, V>>>::iterator m_end;

    iterator(decltype(m_major) major, decltype(m_minor) minor, decltype(m_end) end)
      : m_major {major},
        m_minor {minor},
        m_end   {end}
    { }
    friend class hash_map;
  };

  hash_map() : m_buckets( 0 ), m_sz{0} { }
  hash_map(std::initializer_list<std::pair<K, V>> init)
    : m_buckets ( std::max(init.size() / 6, size_t{6}) ),
      m_sz      {0}
  {
    for (const auto& i : init)
      insert(i.first, i.second);
  }

  size_t size() const { return m_sz; }
  bool contains(const K& item) const
  {
    if (!m_buckets.size())
      return false;
    const auto& bucket = m_buckets[m_hash(item) % m_buckets.size()];
    return any_of(std::begin(bucket), std::end(bucket),
                  [&item](const auto& i) { return i.first == item; });
  }

  iterator find(const K& item)
  {
    if (!m_buckets.size())
      return end();
    auto bucket = std::begin(m_buckets) + m_hash(item) % m_buckets.size();
    auto iter = std::find_if(std::begin(*bucket), std::end(*bucket),
                             [&item](const auto& i) { return i.first == item; });
    return iter == std::end(*bucket) ? end() : iterator{bucket, iter, std::end(m_buckets)};
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

  void insert(K&& item, V&& val)
  {
    if (m_buckets.size()) {
      auto& bucket = m_buckets[m_hash(item) % m_buckets.size()];
      size_t slot_sz{};
      auto slot = find_if(std::begin(bucket), std::end(bucket),
                          [&](const auto& i) { ++slot_sz; return i.first == item; });
      if (slot != std::end(bucket)) {
        slot->second = std::move(val);
        return;
      }

      ++m_sz;

      if (slot_sz < 6) {
        bucket.emplace_front(std::move(item), std::move(val));
        return;
      }
    } else {
      ++m_sz;
    }

    rehash();
    auto& nbucket = m_buckets[m_hash(item) % m_buckets.size()];
    nbucket.emplace_front(std::move(item), std::move(val));
  }

  V& operator[](const K& item)
  {
    if (m_buckets.size()) {
      auto& bucket = m_buckets[m_hash(item) % m_buckets.size()];

      size_t slot_sz{};
      auto slot = find_if(std::begin(bucket), std::end(bucket),
                          [&](const auto& i) { ++slot_sz; return i.first == item; });
      if (slot != std::end(bucket))
        return slot->second;

      ++m_sz;

      if (slot_sz < 6) {
        bucket.emplace_front(item, V{});
        return bucket.front().second;
      }
    } else {
      ++m_sz;
    }

    rehash();
    auto& nbucket = m_buckets[m_hash(item) % m_buckets.size()];
    nbucket.emplace_front(item, V{});
    return nbucket.front().second;
  }

  const V& at(const K& item) const
  {
    if (m_buckets.size()) {
      const auto& bucket = m_buckets[m_hash(item) % m_buckets.size()];
      auto slot = find_if(std::begin(bucket), std::end(bucket),
                          [&item](const auto& i) { return i.first == item; });
      if (slot != std::end(bucket))
        return slot->second;
    }

    throw std::out_of_range{"no such item in hash_map"};
  }

  iterator begin()
  {
    auto first_nonempty = find_if(std::begin(m_buckets), std::end(m_buckets),
                                  [](const auto& i) { return !i.empty(); });
    if (first_nonempty == std::end(m_buckets))
      return end();
    return {first_nonempty, std::begin(*first_nonempty), std::end(m_buckets)};
  }

  iterator end()
  {
    return {std::end(m_buckets), {}, std::end(m_buckets)};
  }

private:
  std::vector<std::forward_list<std::pair<K, V>>> m_buckets;
  size_t m_sz;
  std::hash<K> m_hash{};

  void rehash()
  {
    std::vector<std::forward_list<std::pair<K, V>>> old( m_buckets.size() + 6 );
    std::swap(m_buckets, old);
    m_sz = 0;
    for (auto& i : old)
      for (auto& j : i)
        insert(std::move(j.first), std::move(j.second));
  }
};

}

#endif
