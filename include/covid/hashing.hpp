#ifndef INCLUDE_COVID_HASHING_HPP_
#define INCLUDE_COVID_HASHING_HPP_

#include <functional>

namespace covid {
  template <typename T>
    inline size_t hash_combine(size_t seed, const T& v) {
      std::hash<T> hasher;
      size_t k = 0x9E3779B97F4A7C15ul;
      size_t a = (hasher(v) ^ seed) * k;
      a ^= (a >> 47);
      size_t b = (seed ^ a) * k;
      b ^= (b >> 47);
      return b * k;
    }
}  // namespace covid

namespace std {
  template<typename S, typename T>
  struct hash<pair<S, T>> {
    inline size_t operator()(const pair<S, T> & v) const {
      return covid::hash_combine(covid::hash_combine(0ul, v.first), v.second);
    }
  };
}  // namespace std

#endif  // INCLUDE_COVID_HASHING_HPP_
