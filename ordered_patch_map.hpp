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
  using std::index_sequence;
  using std::index_sequence_for;
  using std::is_unsigned;
  using std::iterator;
  using std::iterator_traits;
  using std::make_unsigned;
  using std::numeric_limits;
  using std::pair;
  using std::random_access_iterator_tag;
  using std::result_of;
  using std::swap;

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
  
  uint32_t constexpr ctz(const uint32_t x){
    return __builtin_clzl(reverse(x));
  }
  
  uint64_t constexpr clz(const uint64_t x){
    return __builtin_clzl(x);
  }
  
  uint64_t constexpr ctz(const uint64_t x){
    return __builtin_clzl(reverse(x));
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
    return rol(circadd(v0,v1),n)^circdff(v0,v1);
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
            const size_t s = clz(reverse(p))>n?n:clz(reverse(p));
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
