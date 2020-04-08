#ifndef INCLUDE_COVID_CATEGORICAL_ARRAY_HPP_
#define INCLUDE_COVID_CATEGORICAL_ARRAY_HPP_

#include <array>

namespace covid {

  // Array with type-safe enum class as index
  template <class T, class I, size_t size>
  class categorical_array {
  private:
    std::array<T, size> _ar;
  public:
    categorical_array() {
      _ar.fill(T{});
    }

    categorical_array(std::initializer_list<T> args) {
      auto it = _ar.begin();
      for (auto&& i: args) {
        *it = i;
        it++;
      }
    }

    const T& operator[](I ind) const { return _ar[static_cast<size_t>(ind)]; }
    T& operator[](I ind) { return _ar[static_cast<size_t>(ind)]; }
  };
}  // namespace covid

#endif  // INCLUDE_COVID_CATEGORICAL_ARRAY_HPP_
