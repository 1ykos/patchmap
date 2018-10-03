#ifndef ORDERED_PATCH_MAP_H
#define ORDERED_PATCH_MAP_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#if defined (__has_include) && (__has_include(<x86intrin.h>))
#include <x86intrin.h>
#endif
#include <bitset>

constexpr bool VERBOSE_WMATH = false;

namespace wmath{
  using std::cerr;
  using std::cout;
  using std::enable_if;
  using std::endl;
  using std::get;
  using std::index_sequence;
  using std::index_sequence_for;
  using std::is_trivially_copyable;
  using std::is_unsigned;
  using std::iterator;
  using std::iterator_traits;
  using std::make_unsigned;
  using std::numeric_limits;
  using std::pair;
  using std::random_access_iterator_tag;
  using std::result_of;
  using std::swap;
  using std::tuple;

  template <typename T>
  constexpr size_t digits(const T& n=0){
    return std::numeric_limits<T>::digits;
  }

  template<typename T>
  typename std::enable_if<std::is_unsigned<T>::value,tuple<T,T>>::type
  constexpr long_mul(const T& a, const T& b);

  // calculate a * b = r0r1
  template<typename T>
  typename std::enable_if<std::is_unsigned<T>::value,tuple<T,T>>::type
  constexpr long_mul(const T& a, const T& b){
    const T N  = digits<T>()/2;
    const T t0 = (a>>N)*(b>>N);
    const T t1 = ((a<<N)>>N)*(b>>N);
    const T t2 = (a>>N)*((b<<N)>>N);
    const T t3 = ((a<<N)>>N)*((b<<N)>>N);
    const T t4 = t3+(t1<<N);
    const T r1 = t4+(t2<<N);
    const T r0 = (r1<t4)+(t4<t3)+(t1>>N)+(t2>>N)+t0;
    return {r0,r1};
  }
  
  template <typename T,typename S>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr ror(const T n, const S i){
    const T m = (std::numeric_limits<T>::digits-1);
    const T c = i&m;
    return (n>>c)|(n<<((-c)&m));
  }

  template <typename T,typename S>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr rol(const T n, const S i){
    const T m = (std::numeric_limits<T>::digits-1);
    const T c = i&m;
    return (n<<c)|(n>>((-c)&m));
  }

#ifdef __SIZEOF_INT128__
  template<>
  tuple<uint64_t,uint64_t>
  constexpr long_mul(const uint64_t& a, const uint64_t& b){
    unsigned __int128 r = ((unsigned __int128)(a))*((unsigned __int128)(b));
    return {uint64_t(r>>64),uint64_t(r)};
  }
#endif

  template<>
  tuple<uint8_t,uint8_t> constexpr long_mul(const uint8_t& a,const uint8_t& b){
    const int_fast16_t r = int_fast16_t(a)*int_fast16_t(b);
    return {uint8_t(r>>8),uint8_t(r)};
  }
  
  template<>
  tuple<uint16_t,uint16_t> constexpr long_mul(
      const uint16_t& a,
      const uint16_t& b){
    const int_fast32_t r = int_fast32_t(a)*int_fast32_t(b);
    return {uint16_t(r>>16),uint16_t(r)};
  }
  
  template<>
  tuple<uint32_t,uint32_t> constexpr long_mul(
      const uint32_t& a,
      const uint32_t& b){
    const int_fast64_t r = int_fast64_t(a)*int_fast64_t(b);
    return {uint32_t(r>>32),uint32_t(r)};
  }

  template <typename T>
  constexpr size_t popcount(const T n){
    size_t c=0;
    while(n) (n&=(n-1),++c);
    return c;
  }

#if __GNUC__ > 3 || __clang__
  constexpr size_t popcount(const uint32_t n){
    return __builtin_popcountl(n);
  }

  constexpr size_t popcount(const uint64_t n){
    return __builtin_popcountll(n);
  }
#endif

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr bitmask(const T& lower, const T& upper){ // exclusive upper limit
    return T(1)<<upper-T(1)<<lower;
  }

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr bitmask_upper(const T& upper){
    return (upper<std::numeric_limits<T>::digits)?
      T(1)<<upper-T(1):std::numeric_limits<T>::max();
  }

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr bitmask_lower(const T& lower){
    return T(0)-T(1)<<lower;
  }
  
  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr alternating_bitmask(const size_t step){
    T mask(0);
    for (size_t i=0;i<digits<T>();i+=2*step){
      mask|=(~T(0)>>(digits<T>()-step))<<i;
    }
    return mask;
  }

  template<class T, class U = typename std::make_unsigned<T>::type>
  constexpr T bswap(T n){
    for (size_t i=digits<unsigned char>();i<digits<T>();i*=2){
      n = ((n&(~(alternating_bitmask<T>(i))))>>i)|
          ((n&( (alternating_bitmask<T>(i))))<<i);
    }
    return n;
  }

#if __GNUC__ > 3 || __clang__
  constexpr uint16_t bswap(uint16_t n){
    return __builtin_bswap16(n);
  }

  constexpr uint32_t bswap(uint32_t n){
    return __builtin_bswap32(n);
  }

  constexpr uint64_t bswap(uint64_t n){
    return __builtin_bswap64(n);
  }
  
  constexpr int16_t bswap(int16_t n){
    return __builtin_bswap16(n);
  }

  constexpr int32_t bswap(int32_t n){
    return __builtin_bswap32(n);
  }

  constexpr int64_t bswap(int64_t n){
    return __builtin_bswap64(n);
  }
#endif

  template<class T,size_t step=1,class U = typename std::make_unsigned<T>::type>
  constexpr U reflect(T n) {
    for (size_t i=step;i<digits<unsigned char>();i*=2){ 
      n = ((n&(~(alternating_bitmask<T>(i))))>>i)|
        ((n&( (alternating_bitmask<T>(i))))<<i);
    }
    return bswap(n);
  }

  uint8_t inline reflect(const uint8_t n){
    switch (n){
      case 0b00000000: return 0b00000000;
      case 0b00000001: return 0b10000000;
      case 0b00000010: return 0b01000000;
      case 0b00000011: return 0b11000000;
      case 0b00000100: return 0b00100000;
      case 0b00000101: return 0b10100000;
      case 0b00000110: return 0b01100000;
      case 0b00000111: return 0b11100000;
      case 0b00001000: return 0b00010000;
      case 0b00001001: return 0b10010000;
      case 0b00001010: return 0b01010000;
      case 0b00001011: return 0b11010000;
      case 0b00001100: return 0b00110000;
      case 0b00001101: return 0b10110000;
      case 0b00001110: return 0b01110000;
      case 0b00001111: return 0b11110000;

      case 0b00010000: return 0b00001000;
      case 0b00010001: return 0b10001000;
      case 0b00010010: return 0b01001000;
      case 0b00010011: return 0b11001000;
      case 0b00010100: return 0b00101000;
      case 0b00010101: return 0b10101000;
      case 0b00010110: return 0b01101000;
      case 0b00010111: return 0b11101000;
      case 0b00011000: return 0b00011000;
      case 0b00011001: return 0b10011000;
      case 0b00011010: return 0b01011000;
      case 0b00011011: return 0b11011000;
      case 0b00011100: return 0b00111000;
      case 0b00011101: return 0b10111000;
      case 0b00011110: return 0b01111000;
      case 0b00011111: return 0b11111000;

      case 0b00100000: return 0b00000100;
      case 0b00100001: return 0b10000100;
      case 0b00100010: return 0b01000100;
      case 0b00100011: return 0b11000100;
      case 0b00100100: return 0b00100100;
      case 0b00100101: return 0b10100100;
      case 0b00100110: return 0b01100100;
      case 0b00100111: return 0b11100100;
      case 0b00101000: return 0b00010100;
      case 0b00101001: return 0b10010100;
      case 0b00101010: return 0b01010100;
      case 0b00101011: return 0b11010100;
      case 0b00101100: return 0b00110100;
      case 0b00101101: return 0b10110100;
      case 0b00101110: return 0b01110100;
      case 0b00101111: return 0b11110100;

      case 0b00110000: return 0b00001100;
      case 0b00110001: return 0b10001100;
      case 0b00110010: return 0b01001100;
      case 0b00110011: return 0b11001100;
      case 0b00110100: return 0b00101100;
      case 0b00110101: return 0b10101100;
      case 0b00110110: return 0b01101100;
      case 0b00110111: return 0b11101100;
      case 0b00111000: return 0b00011100;
      case 0b00111001: return 0b10011100;
      case 0b00111010: return 0b01011100;
      case 0b00111011: return 0b11011100;
      case 0b00111100: return 0b00111100;
      case 0b00111101: return 0b10111100;
      case 0b00111110: return 0b01111100;
      case 0b00111111: return 0b11111100;

      case 0b01000000: return 0b00000010;
      case 0b01000001: return 0b10000010;
      case 0b01000010: return 0b01000010;
      case 0b01000011: return 0b11000010;
      case 0b01000100: return 0b00100010;
      case 0b01000101: return 0b10100010;
      case 0b01000110: return 0b01100010;
      case 0b01000111: return 0b11100010;
      case 0b01001000: return 0b00010010;
      case 0b01001001: return 0b10010010;
      case 0b01001010: return 0b01010010;
      case 0b01001011: return 0b11010010;
      case 0b01001100: return 0b00110010;
      case 0b01001101: return 0b10110010;
      case 0b01001110: return 0b01110010;
      case 0b01001111: return 0b11110010;

      case 0b01010000: return 0b00001010;
      case 0b01010001: return 0b10001010;
      case 0b01010010: return 0b01001010;
      case 0b01010011: return 0b11001010;
      case 0b01010100: return 0b00101010;
      case 0b01010101: return 0b10101010;
      case 0b01010110: return 0b01101010;
      case 0b01010111: return 0b11101010;
      case 0b01011000: return 0b00011010;
      case 0b01011001: return 0b10011010;
      case 0b01011010: return 0b01011010;
      case 0b01011011: return 0b11011010;
      case 0b01011100: return 0b00111010;
      case 0b01011101: return 0b10111010;
      case 0b01011110: return 0b01111010;
      case 0b01011111: return 0b11111010;

      case 0b01100000: return 0b00000110;
      case 0b01100001: return 0b10000110;
      case 0b01100010: return 0b01000110;
      case 0b01100011: return 0b11000110;
      case 0b01100100: return 0b00100110;
      case 0b01100101: return 0b10100110;
      case 0b01100110: return 0b01100110;
      case 0b01100111: return 0b11100110;
      case 0b01101000: return 0b00010110;
      case 0b01101001: return 0b10010110;
      case 0b01101010: return 0b01010110;
      case 0b01101011: return 0b11010110;
      case 0b01101100: return 0b00110110;
      case 0b01101101: return 0b10110110;
      case 0b01101110: return 0b01110110;
      case 0b01101111: return 0b11110110;

      case 0b01110000: return 0b00001110;
      case 0b01110001: return 0b10001110;
      case 0b01110010: return 0b01001110;
      case 0b01110011: return 0b11001110;
      case 0b01110100: return 0b00101110;
      case 0b01110101: return 0b10101110;
      case 0b01110110: return 0b01101110;
      case 0b01110111: return 0b11101110;
      case 0b01111000: return 0b00011110;
      case 0b01111001: return 0b10011110;
      case 0b01111010: return 0b01011110;
      case 0b01111011: return 0b11011110;
      case 0b01111100: return 0b00111110;
      case 0b01111101: return 0b10111110;
      case 0b01111110: return 0b01111110;
      case 0b01111111: return 0b11111110;

      case 0b10000000: return 0b00000001;
      case 0b10000001: return 0b10000001;
      case 0b10000010: return 0b01000001;
      case 0b10000011: return 0b11000001;
      case 0b10000100: return 0b00100001;
      case 0b10000101: return 0b10100001;
      case 0b10000110: return 0b01100001;
      case 0b10000111: return 0b11100001;
      case 0b10001000: return 0b00010001;
      case 0b10001001: return 0b10010001;
      case 0b10001010: return 0b01010001;
      case 0b10001011: return 0b11010001;
      case 0b10001100: return 0b00110001;
      case 0b10001101: return 0b10110001;
      case 0b10001110: return 0b01110001;
      case 0b10001111: return 0b11110001;

      case 0b10010000: return 0b00001001;
      case 0b10010001: return 0b10001001;
      case 0b10010010: return 0b01001001;
      case 0b10010011: return 0b11001001;
      case 0b10010100: return 0b00101001;
      case 0b10010101: return 0b10101001;
      case 0b10010110: return 0b01101001;
      case 0b10010111: return 0b11101001;
      case 0b10011000: return 0b00011001;
      case 0b10011001: return 0b10011001;
      case 0b10011010: return 0b01011001;
      case 0b10011011: return 0b11011001;
      case 0b10011100: return 0b00111001;
      case 0b10011101: return 0b10111001;
      case 0b10011110: return 0b01111001;
      case 0b10011111: return 0b11111001;

      case 0b10100000: return 0b00000101;
      case 0b10100001: return 0b10000101;
      case 0b10100010: return 0b01000101;
      case 0b10100011: return 0b11000101;
      case 0b10100100: return 0b00100101;
      case 0b10100101: return 0b10100101;
      case 0b10100110: return 0b01100101;
      case 0b10100111: return 0b11100101;
      case 0b10101000: return 0b00010101;
      case 0b10101001: return 0b10010101;
      case 0b10101010: return 0b01010101;
      case 0b10101011: return 0b11010101;
      case 0b10101100: return 0b00110101;
      case 0b10101101: return 0b10110101;
      case 0b10101110: return 0b01110101;
      case 0b10101111: return 0b11110101;

      case 0b10110000: return 0b00001101;
      case 0b10110001: return 0b10001101;
      case 0b10110010: return 0b01001101;
      case 0b10110011: return 0b11001101;
      case 0b10110100: return 0b00101101;
      case 0b10110101: return 0b10101101;
      case 0b10110110: return 0b01101101;
      case 0b10110111: return 0b11101101;
      case 0b10111000: return 0b00011101;
      case 0b10111001: return 0b10011101;
      case 0b10111010: return 0b01011101;
      case 0b10111011: return 0b11011101;
      case 0b10111100: return 0b00111101;
      case 0b10111101: return 0b10111101;
      case 0b10111110: return 0b01111101;
      case 0b10111111: return 0b11111101;

      case 0b11000000: return 0b00000011;
      case 0b11000001: return 0b10000011;
      case 0b11000010: return 0b01000011;
      case 0b11000011: return 0b11000011;
      case 0b11000100: return 0b00100011;
      case 0b11000101: return 0b10100011;
      case 0b11000110: return 0b01100011;
      case 0b11000111: return 0b11100011;
      case 0b11001000: return 0b00010011;
      case 0b11001001: return 0b10010011;
      case 0b11001010: return 0b01010011;
      case 0b11001011: return 0b11010011;
      case 0b11001100: return 0b00110011;
      case 0b11001101: return 0b10110011;
      case 0b11001110: return 0b01110011;
      case 0b11001111: return 0b11110011;

      case 0b11010000: return 0b00001011;
      case 0b11010001: return 0b10001011;
      case 0b11010010: return 0b01001011;
      case 0b11010011: return 0b11001011;
      case 0b11010100: return 0b00101011;
      case 0b11010101: return 0b10101011;
      case 0b11010110: return 0b01101011;
      case 0b11010111: return 0b11101011;
      case 0b11011000: return 0b00011011;
      case 0b11011001: return 0b10011011;
      case 0b11011010: return 0b01011011;
      case 0b11011011: return 0b11011011;
      case 0b11011100: return 0b00111011;
      case 0b11011101: return 0b10111011;
      case 0b11011110: return 0b01111011;
      case 0b11011111: return 0b11111011;

      case 0b11100000: return 0b00000111;
      case 0b11100001: return 0b10000111;
      case 0b11100010: return 0b01000111;
      case 0b11100011: return 0b11000111;
      case 0b11100100: return 0b00100111;
      case 0b11100101: return 0b10100111;
      case 0b11100110: return 0b01100111;
      case 0b11100111: return 0b11100111;
      case 0b11101000: return 0b00010111;
      case 0b11101001: return 0b10010111;
      case 0b11101010: return 0b01010111;
      case 0b11101011: return 0b11010111;
      case 0b11101100: return 0b00110111;
      case 0b11101101: return 0b10110111;
      case 0b11101110: return 0b01110111;
      case 0b11101111: return 0b11110111;

      case 0b11110000: return 0b00001111;
      case 0b11110001: return 0b10001111;
      case 0b11110010: return 0b01001111;
      case 0b11110011: return 0b11001111;
      case 0b11110100: return 0b00101111;
      case 0b11110101: return 0b10101111;
      case 0b11110110: return 0b01101111;
      case 0b11110111: return 0b11101111;
      case 0b11111000: return 0b00011111;
      case 0b11111001: return 0b10011111;
      case 0b11111010: return 0b01011111;
      case 0b11111011: return 0b11011111;
      case 0b11111100: return 0b00111111;
      case 0b11111101: return 0b10111111;
      case 0b11111110: return 0b01111111;
      case 0b11111111: return 0b11111111;
      
      default: return 0;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr inverse(const T& n){
    if (n==0) return n;
    return (~T(0))/n+(popcount(n)==1);
  }
  
  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr largest_prime(const T& i=0);

  template <>
  uint8_t constexpr largest_prime(const uint8_t& i){
    switch (i){
      case 0:
        return 251;
      case 1:
        return 241;
      case 2:
        return 239;
      case 3:
        return 233;
      default:
        return 1;
    }
  }

  template <>
  uint16_t constexpr largest_prime(const uint16_t& i){
    switch (i){
      case 0:
        return uint16_t(0)-uint16_t(15);
      case 1:
        return uint16_t(0)-uint16_t(17);
      case 2:
        return uint16_t(0)-uint16_t(39);
      case 3:
        return uint16_t(0)-uint16_t(57);
      default:
        return 1;
    }
  }

  template <>
  uint32_t constexpr largest_prime(const uint32_t& i){
    switch (i){
      case 0:
        return uint32_t(0)-uint32_t(5);
      case 1:
        return uint32_t(0)-uint32_t(17);
      case 2:
        return uint32_t(0)-uint32_t(65);
      case 3:
        return uint32_t(0)-uint32_t(99);
      default:
        return 1;
    }
  }

  template <>
  uint64_t constexpr largest_prime(const uint64_t& i){
    switch (i){
      case 0:
        return uint64_t(0)-uint64_t(59);
      case 1:
        return uint64_t(0)-uint64_t(83);
      case 2:
        return uint64_t(0)-uint64_t(95);
      case 3:
        return uint64_t(0)-uint64_t(179);
      default:
        return 1;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr random_prime(const T& i=0);

  template <>
  uint8_t constexpr random_prime(const uint8_t& i){
    uint8_t primes[] = {127u,
                        131u,
                        251u,
                        223u,
                        191u,
                        193u,
                        47u,
                        97u};
    return primes[i];
  }
  
  template <>
  uint16_t constexpr random_prime(const uint16_t& i){
    uint16_t primes[] = {42307u,
                         52313u,
                         51307u,
                         11317u,
                         60317u,
                         60337u,
                         60037u,
                         30137u};
    return primes[i];
  }

  template <>
  uint32_t constexpr random_prime(const uint32_t& i){
    uint32_t primes[] = {4184867191u,
                         4184864197u,
                         4184411197u,
                         3184410197u,
                         2184200197u,
                          728033399u,
                         1061068399u,
                         3183208117u};
    return primes[i];
  }
  
  template <>
  uint64_t constexpr random_prime(const uint64_t& i){
    uint64_t primes[] = {15112557877901478707ul,
                         18446744073709503907ul,
                          5819238023569667969ul,
                         17457704070697003907ul,
                         14023704271282629773ul,
                         15457704070697023907ul,
                         12023704271182029287ul,
                         10023704271182029357ul,
                          8023704271998834967ul};
    return primes[i];
  }
  
  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr clmul_circ(const T& i,const T& j){
    T r=0;
    for (size_t n=0;n!=digits<T>();++n) if(j&(T(1)<<n)) r^=wmath::rol(i,n);
    return r;
  }

#if defined (__has_include) && (__has_include(<x86intrin.h>))
  uint8_t const clmul_circ(const uint8_t& a,const uint8_t& b){
    int64_t _a = a;
    int64_t _b = b;
    __m128i ma{_a,0ull};
    __m128i mb{_b,0ull};
    auto t = _mm_clmulepi64_si128(ma,mb,0);
    return t[0]^(t[0]>>8);
  }
  
  uint16_t const clmul_circ(const uint16_t& a,const uint16_t& b){
    int64_t _a(0);
    int64_t _b(0);
    _a^=a;
    _b^=b;
    __m128i ma{_a,0ull};
    __m128i mb{_b,0ull};
    auto t = _mm_clmulepi64_si128(ma,mb,0);
    return t[0]^(t[0]>>16);
  }
  
  uint32_t const clmul_circ(const uint32_t& a,const uint32_t& b){
    int64_t _a(0);
    int64_t _b(0);
    _a^=a;
    _b^=b;
    __m128i ma{_a,0ull};
    __m128i mb{_b,0ull};
    auto t = _mm_clmulepi64_si128(ma,mb,0);
    return t[0]^(t[0]>>32);
  }
  
  uint64_t const clmul_circ(const uint64_t& a,const uint64_t& b){
    __m128i ma{(const int64_t)(a),0ull};
    __m128i mb{(const int64_t)(b),0ull};
    auto t = _mm_clmulepi64_si128(ma,mb,0);
    return t[0]^t[1];
  }
#endif

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr clz(const T x,const T lower=0,const T upper=digits<T>()){
    return (upper-lower==T(1))?digits<T>()-upper:(x&(T(0)-T(1)<<((upper+lower)/2))?
           clz(x,(upper+lower)/2,upper):
           clz(x,lower,(upper+lower)/2));
  }
 
  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr log2(const T x,const T lower=0,const T upper=digits<T>()){
    return (upper-lower==T(1))?lower:(x&(T(0)-T(1)<<((upper+lower)/2))?
           log2(x,(upper+lower)/2,upper):
           log2(x,lower,(upper+lower)/2));
  }

#if __GNUC__ > 3 || __clang__
  uint32_t constexpr clz(const uint32_t x){
    return __builtin_clzl(x);
  }
  
  uint64_t constexpr clz(const uint64_t x){
    return __builtin_clzl(x);
  }
  
  uint32_t constexpr log2(const uint32_t x){
    return x==0?0:31-__builtin_clzl(x);
  }
  
  uint64_t constexpr log2(const uint64_t x){
    return x==0?0:63-__builtin_clzll(x);
  }
#endif

  template< typename, typename = void >
  struct has_hash : std::false_type {};

  template<typename T>
  struct has_hash<T,decltype( std::hash< T >()( std::declval< T >() ), void() ) >
  : std::true_type {};

  template<typename T>
  typename enable_if<has_hash<T>::value,size_t>::type
  constexpr hash(const T& v){
    return std::hash<T>()(v);
  }

  size_t constexpr hash(const size_t& v){
    return v;
  }
  
  size_t constexpr hash(const size_t& v0,const size_t& v1) {
    const size_t n = 1+(numeric_limits<size_t>::digits*144+116)/233;
    return rol(v0+v1,n)^(v0-v1);
  }
 
  template <typename T,typename... Rest>
  size_t constexpr hash(const T& v,Rest... rest);
  
  template<typename T,size_t... I>
  size_t constexpr hash_tuple_impl(const T& t, index_sequence<I...>){
    return hash(std::get<I>(t)...);
  }

  template<typename... Ts>
  size_t constexpr hash(const tuple<Ts...>& t){
    return hash_tuple_impl(t,index_sequence_for<Ts...>{});
  }
  
  template <typename T, typename... Rest>
  size_t constexpr hash(const T& v, Rest... rest) {
    return hash(hash(v),hash(rest...));
  }

  template <typename T>
  struct hash_functor{
    size_t constexpr operator()(const T& v) const {
      return wmath::hash(v);
    }
  };

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr distribute(const T& a){
    return clmul_circ(a+random_prime<T>(2),random_prime<T>(0));
  }

  template<class K,class T,class hash=hash_functor<K>>
  class ordered_patch_map{
    public:
      hash hasher;
    private:
      size_t num_data;
      size_t datasize;
      size_t inversed;
      size_t nextsize;
      size_t masksize;
      pair<K,T> * data;
      size_t * mask;
      constexpr static bool smartskip = true;
      typedef typename std::result_of<hash(K)>::type H;
      size_t const inline map(const H& h) const {
        constexpr size_t s =
          (digits<H>()>digits<size_t>())?(h>>(digits<H>()-digits<size_t>())):0;
        //cout << "(h>>s) = " << (h>>s) << endl;
        return get<0>(long_mul(h>>s,datasize));
      }
      H const inline order(const K& k) const {
        return distribute(hasher(k));
      }
      bool const inline is_set(const size_t& n) const {
        const size_t i = n/digits<size_t>();
        const size_t j = n%digits<size_t>();
        if (mask[i]&(size_t(1)<<(digits<size_t>()-j-1))) return true;
        return false;
      }
      void const inline set(const size_t& n) {
        const size_t i = n/digits<size_t>();
        const size_t j = n%digits<size_t>();
        mask[i]|=size_t(1)<<(digits<size_t>()-j-1);
      }
      void const inline unset(const size_t& n) {
        const size_t i = n/digits<size_t>();
        const size_t j = n%digits<size_t>();
        mask[i]&=((~size_t(0))^(size_t(1)<<(digits<size_t>()-j-1)));
      }
      const inline void swap_set(const size_t& i,const size_t& j){
        if (is_set(i)==is_set(j)) return;
        if (is_set(i)){
          set(j);
          unset(i);
        }else{
          set(i);
          unset(j);
        }
      }
      H const inline index(const size_t& i) const {
        if (is_set(i)) return order(data[i].first);
        return i*inversed;
      }
      size_t const inline search_backward(size_t i){ // search for free bucket
        while(true){
          if (!is_set(i)) return i;
          const size_t k = i/digits<size_t>();
          const size_t l = i%digits<size_t>();
          const size_t m = (~size_t(0))<<(digits<size_t>()-l-1);
          if constexpr (smartskip){
            const size_t p = ((~(mask[k]&m))>>(digits<size_t>()-l-1));
            const size_t n = l+1;
            const size_t s = clz(reflect(p))>n?n:clz(reflect(p));
            i-=s;
          } else {
            if ((mask[k]&m)==m)
              i=(i/digits<size_t>()-1)*digits<size_t>()+digits<size_t>()-1;
            else --i;
          }
          if (i>datasize) return datasize;
        }
      }
      size_t const inline search_forward(size_t i){ // search for free bucket
        while(true){
          if (!is_set(i)) return i;
          const size_t k = i/digits<size_t>();
          const size_t l = i%digits<size_t>();
          const size_t m = (~size_t(0))>>l; 
          if constexpr (smartskip){
            const size_t p = (~(mask[k]&m))<<l;          
            const size_t n = digits<size_t>()-l;
            const size_t s = clz(p)>n?n:clz(p);
            i+=s;
          } else {
            if ((mask[k]&m)==m) i=(i/digits<size_t>()+1)*digits<size_t>();
            else ++i;
          }
          if (i>datasize) return datasize;
        }
      }
      size_t const inline insert_node(const K& key,const size_t& hint){
        //cout << "insert_node(" << key << ", " << hint << ")" << endl; 
        size_t i = hint;
        const H ok = order(key);
        if (ok < index(i)) {
          const size_t j = search_backward(i);
          if (j==datasize) i = search_forward(i);
          else i = j;
        }else{
          const size_t j = search_forward(i);
          if (j==datasize) i = search_backward(i);
          else i = j;
        }
        //assert(i<datasize);
        data[i].first=key;
        set(i);
        ++num_data;
        while(true){
          if (i==0) break;
          if (!is_set(i-1)) break;
          if (order(data[i].first)<order(data[i-1].first))
            swap(data[i],data[i-1]);
          else break;
          --i;
        }
        while(true){
          if (i+1>=datasize) break;
          if (!is_set(i+1)) break;
          if (order(data[i].first)>order(data[i+1].first))
            swap(data[i],data[i+1]);
          else break;
          ++i;
        }
        //cout << "inserted at " << i << endl;
        return i;
      }
      size_t const inline insert_node(const K& key){
        const size_t hint = map(order(key));
        //assert(hint<datasize);
        return insert_node(key,hint);
      }
      size_t const inline forward_linear_search(
          const K& k,
          const size_t& i) const {
        for (size_t j=0;;++j){
          if (i+j==datasize) return datasize;
          if (is_set(i+j)) if (data[i+j].first==k) return i+j;
          if (index(i+j)>order(k)) return datasize;
        }
      }
      size_t const inline backward_linear_search(
          const K& k,
          const size_t& i) const {
        for (size_t j=0;;++j){
          if (i-j>=datasize) return datasize;
          if (is_set(i-j)) if (data[i-j].first==k) return i-j;
          if (index(i-j)<order(k)) return datasize;
        }
      }
      size_t const inline bidirectional_linear_search(
          const K& k,
          const size_t& i) const {
        if (data[i].first == k) return i;
        size_t j;
        for(j=1;;++j){
          if (i+j<datasize){
            if (is_set(i+j)) if (data[i+j].first==k) return i+j;
            if (index(i+j)!=order(k)) return backward_linear_search(k,i-j);
          } else return backward_linear_search(k,i-j);
          if (i>=j){
            if (is_set(i-j)) if (data[i-j].first==k) return i-j;
            if (index(i-j)!=order(k)) return forward_linear_search(k,i+j+1);
          } else return forward_linear_search(k,i+j+1);
        }
        return datasize;
      }
      size_t const inline find_node_binary(
          const K& k,
          const size_t& lo, // inclusive bounds
          const size_t& hi  // inclusive bounds
          ) const {
        const H ok = order(k);
        const size_t mi = (hi+lo)/2;
        const H     imi = index(mi);
        if (imi==ok) return bidirectional_linear_search(k,mi);
        if ( hi==lo) return datasize;
        if (imi <ok) return find_node_binary(k,mi+1,hi);
        else         return find_node_binary(k,lo  ,mi);
      }
      size_t const inline find_node_interpol(
          const K& k,
          const size_t& lo,
          const size_t& hi
          ) const {
        //assert(lo<=hi||datasize==0);
        const H ok = order(k);
        if (hi-lo<257) return find_node_binary(k,lo,hi);
        const size_t l = log2(hi-lo);
        const H ihi = index(hi);
        const H ilo = index(lo);
        //assert(ilo<=ok);
        //assert(ihi>=ok);
        if (((ihi-ilo)>>l)==0) return find_node_binary(k,lo,hi);
        size_t mi = lo+(size_t((ok-ilo)>>l)*(hi-lo))/size_t((ihi-ilo)>>l);
        if (mi==lo) ++mi;
        else if (mi==hi) --mi;
        const H imi = index(mi);
        //assert(mi<=hi);
        //assert(mi>=lo);
        //assert(imi<=ihi);
        //assert(imi>=ilo);
        if (imi==ok) return bidirectional_linear_search(k,mi);
        if (imi <ok) return find_node_interpol(k,mi,hi);
        return              find_node_interpol(k,lo,mi);
      }
      size_t const inline find_node(const K& k) const {
        const H ok = order(k);
        size_t  i  = map(ok);
        if (i>=datasize) return datasize;
        if (index(i)==ok) return bidirectional_linear_search(k,i);
        size_t lo=0;
        size_t hi=datasize-1;
        if (index(i) <ok){
          lo = i;
          size_t d = map(ok-index(i))+1;
          for (size_t j=0;j!=2;++j){
            if (i+d<datasize){
              i+=d;
              d+=d;
              if (index(i)>ok){
                hi = i;
                break;
              } else {
                lo = i;
              }
              continue;
            }
            break;
          }
        }else{
          hi = i;
          size_t d = map(index(i)-ok)+1;
          for (size_t j=0;j!=2;++j){
            if (i>=d){
              i-=d;
              d+=d;
              if (index(i)<=ok){
                lo = i;
                break;
              } else {
                hi = i;
              }
              continue;
            }
            break;
          }
        }
        return find_node_interpol(k,lo,hi);
      }
    public:
      ordered_patch_map(const size_t& datasize = 0)          // constructor
        :datasize(datasize)
      {
        num_data = 0;
        nextsize = datasize*3/2+1;
        inversed = inverse(datasize);
        data = new pair<K,T>[datasize];
        masksize = (datasize+digits<size_t>()-1)/digits<size_t>();
        mask = new size_t[masksize]();
      }
      ~ordered_patch_map(){                                  // destructor
        delete[] mask;
        delete[] data;
      }
      ordered_patch_map(const ordered_patch_map& other){     // copy constructor
        num_data = other.num_data;
        datasize = other.datasize;
        inversed = other.inversed;
        nextsize = other.nextsize;
        masksize = other.masksize;
        mask = new size_t[masksize];
        memcpy(reinterpret_cast<void*>(mask),
               reinterpret_cast<void*>(other.mask),
               masksize*sizeof(size_t));
        data = new pair<K,T>[datasize];
        if constexpr (is_trivially_copyable<pair<K,T>>::value)
          memcpy(reinterpret_cast<void*>(data),
                 reinterpret_cast<void*>(other.data),
                 datasize*sizeof(pair<K,T>));
        else for (size_t i=0;i!=datasize;++i) data[i]=other.data[i];
      }
      ordered_patch_map(ordered_patch_map&& other) noexcept  // move constructor
      {
        mask = nullptr;
        data = nullptr;
        swap(mask,other.mask);
        swap(data,other.data);
        swap(datasize,other.datasize);
        swap(inversed,other.inversed);
        swap(nextsize,other.nextsize);
        swap(masksize,other.masksize);
      }
      ordered_patch_map& operator=                           // copy assignment
        (const ordered_patch_map& other)
      {
        return *this = ordered_patch_map(other);
      }
      ordered_patch_map& operator=                           // move assignment
        (ordered_patch_map&& other)
        noexcept{
        swap(mask,other.mask);
        swap(data,other.data);
        swap(datasize,other.datasize);
        swap(inversed,other.inversed);
        swap(nextsize,other.nextsize);
        swap(masksize,other.masksize);
        return *this;
      }
      size_t erase(const K& k){
        size_t i = find_node(k);
        if (i==datasize) return 0;
        while(true){
          if (i+1==datasize) break;
          if (!is_set(i+1)) break;
          if (order(data[i+1].first)>=i*inversed) break;
          swap(data[i],data[i+1]);
          ++i;
        }
        while(true){
          if (i==0) break;
          if (!is_set(i-1)) break;
          if (order(data[i-1].first)<=i*inversed) break;
          swap(data[i],data[i-1]);
          --i;
        }
        unset(i);
        --num_data;
        return 1;
      }
      void const inline clear(){
        for (size_t i=0;i!=masksize;++i) mask[i]=0;
      }
      void const inline resize(const size_t& n){
        //cout << "resizing from " << datasize << " to " << n << endl;
        pair<K,T> * olddata      = new pair<K,T>[n];
        masksize = (n+digits<size_t>()-1)/digits<size_t>();
        size_t * oldmask = new size_t[masksize]();
        swap(olddata,data);
        swap(oldmask,mask);
        size_t oldsize = datasize;
        datasize = n;
        nextsize = datasize+oldsize;
        num_data = 0;
        inversed = inverse(datasize);
        size_t j = 0;
        for (size_t i=0;i!=oldsize;++i){
          const size_t k = i/digits<size_t>();
          const size_t l = i%digits<size_t>();
          if (oldmask[k]&(size_t(1)<<(digits<size_t>()-l-1)))
            data[insert_node(olddata[i].first)].second=olddata[i].second;
        }
        //cout << "test" << endl;
        delete[] olddata;
        delete[] oldmask;
      }
      size_t const inline size() const {
        return num_data;
      }
      size_t const inline test_size() const {
        size_t test = 0;
        for (size_t i=0;i!=datasize;++i) test += is_set(i);
        return test;
      }
      void const inline test_chunks() const {
        for (size_t i=0;i!=masksize;++i){
          cout << popcount(mask[i]) << endl;
        }
      }
      bool const inline check_ordering() const {
        for (size_t i=0,j=1;j<datasize;(++i,++j))
          if (index(i)>index(j)){
            cout << i << " " << j << endl;
            cout << double(index(i))/pow(2.0,64.0) << " "
                 << double(index(j))/pow(2.0,64.0) << endl;
            return false;
          }
        return true;
      }
      T& operator[](const K& k){
        //cout << "operator[" << k << "]" << endl;
        const size_t i = find_node(k);
        if (i!=datasize) return data[i].second;
        //while ((num_data+2)*4>=datasize*3) resize(nextsize);
        while ((num_data+2)*5>=datasize*4) resize(nextsize);
        //while ((num_data+2)*8>=datasize*7) resize(nextsize);
        //while ((num_data+2)*16>=datasize*15) resize(nextsize);
        //while ((num_data+2)*32>=datasize*31) resize(nextsize);
        //if ((num_data+2)*64>=datasize*64) resize(nextsize);
        return data[insert_node(k)].second;
      }
      size_t const inline count(const K& k){
        return (find_node(k)!=datasize);
      }
  };
}
#endif // ORDERED_PATCH_MAP_H
