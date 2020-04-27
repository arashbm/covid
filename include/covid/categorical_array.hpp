#ifndef INCLUDE_COVID_CATEGORICAL_ARRAY_HPP_
#define INCLUDE_COVID_CATEGORICAL_ARRAY_HPP_

#include <array>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "magic_enum.hpp"
#pragma GCC diagnostic pop


namespace covid {
  // Array with type-safe enum class as index
  template <class T, class I>
  class categorical_array {
  public:
    static constexpr size_t size = magic_enum::enum_count<I>();
    using item_type = T;
    using index_type = I;

    categorical_array() {
      _ar.fill(T{});
    }

    categorical_array(const std::initializer_list<T>& args) {
      if (size != args.size())
        throw std::logic_error("Argument list size does not match array size");

      for (size_t i = 0; i < size; i++)
        _ar[i] = *(args.begin()+i);
    }

    const item_type& operator[](index_type ind) const {
      return _ar[static_cast<size_t>(magic_enum::enum_index(ind).value())];
    }

    item_type& operator[](index_type ind) {
      return _ar[static_cast<size_t>(magic_enum::enum_index(ind).value())];
    }

    // summs across first axis
    T sum() const {
      T s{};
      for (size_t i = 0; i < size; i++)
        s += _ar[i];
      return s;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    // it's okay if _ar[i] is uninitialised
    categorical_array<T, I>&
    operator+=(const categorical_array<T, I>& o) {
      for (size_t i = 0; i < size; i++)
        _ar[i] += o._ar[i];
      return *this;
    }
#pragma GCC diagnostic pop

    friend categorical_array<T, I>
    operator+(
        categorical_array<T, I> o1,
        const categorical_array<T, I>& o2) {
      o1 += o2;
      return o1;
    }

    friend categorical_array<T, I>
    operator+(categorical_array<T, I> o) {
      for (size_t i = 0; i < size; i++)
        o._ar[i] = +o._ar[i];
      return o;
    }


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    // it's okay if _ar[i] is uninitialised
    categorical_array<T, I>&
    operator-=(const categorical_array<T, I>& o) {
      for (size_t i = 0; i < size; i++)
        _ar[i] -= o._ar[i];
      return *this;
    }
#pragma GCC diagnostic pop

    friend categorical_array<T, I>
    operator-(
        categorical_array<T, I> o1,
        const categorical_array<T, I>& o2) {
      o1 -= o2;
      return o1;
    }

    friend categorical_array<T, I>
    operator-(categorical_array<T, I> o) {
      for (size_t i = 0; i < size; i++)
        o._ar[i] = -o._ar[i];
      return o;
    }

#define _covid_define_numeric_multiplication(_Tp)   \
    categorical_array<T, I>&                  \
    operator*=(_Tp m) {                             \
      for (size_t i = 0; i < size; i++)             \
        _ar[i] *= m;                                \
      return *this;                                 \
    }                                               \
                                                    \
    friend categorical_array<T, I>            \
    operator*(categorical_array<T, I> o,      \
        _Tp m) {                                    \
      o *= m;                                       \
      return o;                                     \
    }                                               \
                                                    \
    categorical_array<T, I>&                  \
    operator/=(_Tp m) {                             \
      for (size_t i = 0; i < size; i++)             \
        _ar[i] /= m;                                \
      return *this;                                 \
    }                                               \
                                                    \
    friend categorical_array<T, I>            \
    operator/(categorical_array<T, I> o,      \
        _Tp m) {                                    \
      o /= m;                                       \
      return o;                                     \
    }

    // floating point
    _covid_define_numeric_multiplication(float)               //NOLINT
    _covid_define_numeric_multiplication(double)              //NOLINT
    _covid_define_numeric_multiplication(long double)         //NOLINT

    // integral
    _covid_define_numeric_multiplication(bool)                //NOLINT
    _covid_define_numeric_multiplication(char)                //NOLINT
    _covid_define_numeric_multiplication(signed char)         //NOLINT
    _covid_define_numeric_multiplication(unsigned char)       //NOLINT
    _covid_define_numeric_multiplication(wchar_t)             //NOLINT
    _covid_define_numeric_multiplication(char16_t)            //NOLINT
    _covid_define_numeric_multiplication(char32_t)            //NOLINT
    _covid_define_numeric_multiplication(short)               //NOLINT
    _covid_define_numeric_multiplication(int)                 //NOLINT
    _covid_define_numeric_multiplication(long)                //NOLINT
    _covid_define_numeric_multiplication(long long)           //NOLINT
    _covid_define_numeric_multiplication(unsigned short)      //NOLINT
    _covid_define_numeric_multiplication(unsigned int)        //NOLINT
    _covid_define_numeric_multiplication(unsigned long)       //NOLINT
    _covid_define_numeric_multiplication(unsigned long long)  //NOLINT
#undef _covid_define_numeric_multiplication

  private:
    std::array<T, size> _ar;
  };
}  // namespace covid

#define _covid_define_reverse_numeric_multiplication(_Tp) \
  template <class T, class I>                \
  covid::categorical_array<T, I>                    \
    operator*(_Tp m,                                      \
        covid::categorical_array<T, I> a) {         \
      a *= m;                                             \
      return a;                                           \
    }

    // floating point
    _covid_define_reverse_numeric_multiplication(float)               //NOLINT
    _covid_define_reverse_numeric_multiplication(double)              //NOLINT
    _covid_define_reverse_numeric_multiplication(long double)         //NOLINT

    // integral
    _covid_define_reverse_numeric_multiplication(bool)                //NOLINT
    _covid_define_reverse_numeric_multiplication(char)                //NOLINT
    _covid_define_reverse_numeric_multiplication(signed char)         //NOLINT
    _covid_define_reverse_numeric_multiplication(unsigned char)       //NOLINT
    _covid_define_reverse_numeric_multiplication(wchar_t)             //NOLINT
    _covid_define_reverse_numeric_multiplication(char16_t)            //NOLINT
    _covid_define_reverse_numeric_multiplication(char32_t)            //NOLINT
    _covid_define_reverse_numeric_multiplication(short)               //NOLINT
    _covid_define_reverse_numeric_multiplication(int)                 //NOLINT
    _covid_define_reverse_numeric_multiplication(long)                //NOLINT
    _covid_define_reverse_numeric_multiplication(long long)           //NOLINT
    _covid_define_reverse_numeric_multiplication(unsigned short)      //NOLINT
    _covid_define_reverse_numeric_multiplication(unsigned int)        //NOLINT
    _covid_define_reverse_numeric_multiplication(unsigned long)       //NOLINT
    _covid_define_reverse_numeric_multiplication(unsigned long long)  //NOLINT
#undef _covid_define_reverse_numeric_multiplication

#endif  // INCLUDE_COVID_CATEGORICAL_ARRAY_HPP_
