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

    categorical_array(const std::initializer_list<T>& args) {
      if (size != args.size())
        throw std::length_error("Argument list size does not match array size");

      for (size_t i=0; i<size; i++)
        _ar[i] = *(args.begin()+i);
    }

    const T& operator[](I ind) const { return _ar[static_cast<size_t>(ind)]; }
    T& operator[](I ind) { return _ar[static_cast<size_t>(ind)]; }

    // summs across first axis
    T sum() const{
      T s{};
      for (size_t i=0; i<size; i++)
        s += _ar[i];
      return s;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    // it's okay if _ar[i] is uninitialised
    categorical_array<T, I, size>&
    operator+=(const categorical_array<T, I, size>& o) {
      for (size_t i = 0; i < size; i++)
        _ar[i] += o._ar[i];
      return *this;
    }
#pragma GCC diagnostic pop

    friend categorical_array<T, I, size>
    operator+(
        categorical_array<T, I, size> o1,
        const categorical_array<T, I, size>& o2) {
      o1 += o2;
      return o1;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    // it's okay if _ar[i] is uninitialised
    categorical_array<T, I, size>&
    operator-=(const categorical_array<T, I, size>& o) {
      for (size_t i = 0; i < size; i++)
        _ar[i] -= o._ar[i];
      return *this;
    }
#pragma GCC diagnostic pop

    friend categorical_array<T, I, size>
    operator-(
        categorical_array<T, I, size> o1,
        const categorical_array<T, I, size>& o2) {
      o1 -= o2;
      return o1;
    }

#define _covid_define_numeric_multiplication(_Tp)   \
    categorical_array<T, I, size>&                  \
    operator*=(_Tp m) {                             \
      for (size_t i = 0; i < size; i++)             \
        _ar[i] *= m;                                \
      return *this;                                 \
    }                                               \
                                                    \
    friend categorical_array<T, I, size>            \
    operator*(categorical_array<T, I, size> o,      \
        _Tp m) {                                    \
      o *= m;                                       \
      return o;                                     \
    }                                               \
                                                    \
    categorical_array<T, I, size>&                  \
    operator/=(_Tp m) {                             \
      for (size_t i = 0; i < size; i++)             \
        _ar[i] /= m;                                \
      return *this;                                 \
    }                                               \
                                                    \
    friend categorical_array<T, I, size>            \
    operator/(categorical_array<T, I, size> o,      \
        _Tp m) {                                    \
      o /= m;                                       \
      return o;                                     \
    }

    // floating point
    _covid_define_numeric_multiplication(float)
    _covid_define_numeric_multiplication(double)
    _covid_define_numeric_multiplication(long double)

    // integral
    _covid_define_numeric_multiplication(bool)
    _covid_define_numeric_multiplication(char)
    _covid_define_numeric_multiplication(signed char)
    _covid_define_numeric_multiplication(unsigned char)
    _covid_define_numeric_multiplication(wchar_t)
    _covid_define_numeric_multiplication(char16_t)
    _covid_define_numeric_multiplication(char32_t)
    _covid_define_numeric_multiplication(short)
    _covid_define_numeric_multiplication(int)
    _covid_define_numeric_multiplication(long)
    _covid_define_numeric_multiplication(long long)
    _covid_define_numeric_multiplication(unsigned short)
    _covid_define_numeric_multiplication(unsigned int)
    _covid_define_numeric_multiplication(unsigned long)
    _covid_define_numeric_multiplication(unsigned long long)
#undef _covid_define_numeric_multiplication

  };
}  // namespace covid

#define _covid_define_reverse_numeric_multiplication(_Tp) \
  template <class T, class I, size_t size>                \
  covid::categorical_array<T, I, size>                    \
    operator*(_Tp m,                                      \
        covid::categorical_array<T, I, size> a) {         \
      a *= m;                                             \
      return a;                                           \
    }

    // floating point
    _covid_define_reverse_numeric_multiplication(float)
    _covid_define_reverse_numeric_multiplication(double)
    _covid_define_reverse_numeric_multiplication(long double)

    // integral
    _covid_define_reverse_numeric_multiplication(bool)
    _covid_define_reverse_numeric_multiplication(char)
    _covid_define_reverse_numeric_multiplication(signed char)
    _covid_define_reverse_numeric_multiplication(unsigned char)
    _covid_define_reverse_numeric_multiplication(wchar_t)
    _covid_define_reverse_numeric_multiplication(char16_t)
    _covid_define_reverse_numeric_multiplication(char32_t)
    _covid_define_reverse_numeric_multiplication(short)
    _covid_define_reverse_numeric_multiplication(int)
    _covid_define_reverse_numeric_multiplication(long)
    _covid_define_reverse_numeric_multiplication(long long)
    _covid_define_reverse_numeric_multiplication(unsigned short)
    _covid_define_reverse_numeric_multiplication(unsigned int)
    _covid_define_reverse_numeric_multiplication(unsigned long)
    _covid_define_reverse_numeric_multiplication(unsigned long long)
#undef _covid_define_reverse_numeric_multiplication

#endif  // INCLUDE_COVID_CATEGORICAL_ARRAY_HPP_
