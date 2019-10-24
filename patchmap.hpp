#ifndef ORDERED_PATCH_MAP_H
#define ORDERED_PATCH_MAP_H

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <boost/container/allocator.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <typeinfo>
#include <exception>
#include <memory>

namespace whash{
  bool constexpr VERBOSE_PATCHMAP = false;
  using std::allocator_traits;
  using std::array;
  using std::cerr;
  using std::conditional;
  using std::cout;
  using std::enable_if;
  using std::endl;
  using std::false_type;
  using std::get;
  using std::index_sequence;
  using std::index_sequence_for;
  using std::initializer_list;
  using std::integral_constant;
  using std::is_const;
  using std::is_fundamental;
  using std::is_same;
  using std::is_trivially_copyable;
  using std::make_unique;
  using std::numeric_limits;
  using std::pair;
  using std::setw;
  using std::swap;
  using std::true_type;
  using std::tuple;
  using std::unique_ptr;

  template<class T>
  double frac(const T& n){
    return n*pow(0.5,numeric_limits<T>::digits);
  }

  template<typename T>
  struct dummy_comp{ // dummy comparator for when we don't need a comparator
    constexpr bool operator()(const T&,const T&) const {return false;}
  };

  template <typename T>
  constexpr size_t digits(const T& n=0){
    return numeric_limits<T>::digits;
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
    return {r>>64,r};
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

  constexpr size_t popcount(const uint32_t n){
    return __builtin_popcountl(n);
  }

  constexpr size_t popcount(const uint64_t n){
    return __builtin_popcountll(n);
  }

  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr distribute(const T& a); // mix the hash value good, clmul_circ
                                    // with odious integer is suitable

  uint8_t  constexpr inline distribute(const uint8_t& a){
    return (a+111)*97;
  }

  uint16_t constexpr inline distribute(const uint16_t& a){
    return (a+36690)*43581;
  }

  uint32_t constexpr distribute(uint32_t a){
    const uint32_t  b = 0x55555555ul;
    const uint32_t c0 = 3107070805ul;
    const uint32_t c1 = 3061963241ul;
    a = (a^(a>>16))*b;
    a = a^(a>>16);
    return a;
  }

  uint64_t constexpr distribute(uint64_t a){
    const uint64_t  b =   0x5555555555555555ull;
    const uint64_t c0 = 16123805160827025777ull;
    const uint64_t c1 = 13834579444137454003ull;
    const uint64_t c2 = 14210505232527258663ull;
    a^=(a>>32); a*=b; // a^=(a>>32);
    return a;
  }

  template<typename,typename=void>
  struct is_injective : false_type {};

  template<typename T>
  struct is_injective<T,typename enable_if<T::is_injective::value>::type>
  : true_type {};
  
  template<typename,typename=void>
  struct has_unhash : false_type {};

  template<typename T>
  struct has_unhash<T,typename enable_if<T::unhash_defined::value>::type>
  : true_type {};

  template<typename,typename=void>
  struct has_std_hash : false_type {};

  template<typename T>
  struct has_std_hash<T,decltype(std::hash<T>()(std::declval<T>()),void())>
  : true_type {};

  template<typename T>
  typename
  enable_if<has_std_hash<T>::value&&(!is_fundamental<T>::value),size_t>::type
  constexpr hash(const T& v){
    return std::hash<T>()(v);
  }

  size_t constexpr hash() {
    return 0;
  }
  
  size_t constexpr unhash() {
    return 0;
  }

  size_t constexpr hash(const size_t& seed,const size_t& n) {
    return seed^distribute(n);
  }

  template<class K>
  typename enable_if<(sizeof(K)>sizeof(size_t)
                   &&is_fundamental<K>::value),size_t>::type
  constexpr hash(const K& k){
    size_t h(k);
    const size_t n = sizeof(K)/sizeof(size_t);
    for (size_t i=sizeof(size_t);i<sizeof(K);i+=sizeof(size_t))
      h = hash(h,size_t(k>>(i*CHAR_BIT)));
    return h;
  }
  
  template<typename T>
  constexpr T modular_inverse(const T& a) {
    T x = a&1;
    for (T i=1;i!=digits<T>();++i) x*=2-a*x;
    return x;
  }
 
  uint8_t constexpr hash(const uint8_t& v){
    return (v+111)*97;
  }
  
  uint8_t constexpr uhash(const uint8_t& v){
    return v*modular_inverse(97)-111;
  }

  int8_t constexpr hash(const int8_t& v){
    return hash(uint8_t(v));
  }
  
  int8_t constexpr unhash(const int8_t& v){
    return unhash(uint8_t(v));
  }
  
  uint16_t constexpr hash(const uint16_t& v){
    return (v+36690)*43581;
  }
  
  uint16_t constexpr unhash(const uint16_t& v){
    return (v*modular_inverse(43581))-36690;
  }

  int16_t constexpr hash(const int16_t& v){
    return hash(uint16_t(v));
  }
  
  int16_t constexpr unhash(const int16_t& v){
    return unhash(uint16_t(v));
  }

  uint32_t constexpr hash(uint32_t v){
    const uint32_t  a = 3370923577ul;
    v^= v>>16;
    v*= a;
    v^= v>>16;
    return v;
  }
  
  uint32_t constexpr unhash(uint32_t v){
    const uint32_t  a = 3370923577ul;
    v^= v>>16;
    v*= modular_inverse(a);
    v^= v>>16;
    return v;
  }

  int32_t constexpr hash(const int32_t v){
    return hash(uint32_t(v));
  }
  
  int32_t constexpr unhash(const int32_t v){
    return unhash(uint32_t(v));
  }

  uint64_t constexpr hash(uint64_t v){
    const uint64_t  a = 15864664792644967873ull;
    v^= v>>32;
    v*= a;
    v^= v>>32;
    return v;
  }
  
  uint64_t constexpr unhash(uint64_t v){
    const uint64_t  a = 15864664792644967873ull;
    v^= v>>32;
    v*= modular_inverse(a);
    v^= v>>32;
    return v;
  }

  int64_t constexpr hash(const int64_t& v){
    return hash(uint64_t(v));
  }
  
  int64_t constexpr unhash(const int64_t& v){
    return unhash(uint64_t(v));
  }
 
  template <typename T,typename R,typename... Rest>
  size_t constexpr hash(const T& v,const R& r,Rest... rest);

  uint16_t constexpr hash(const uint8_t& v0,const uint8_t& v1){
    return (uint16_t(v0)<<8)^uint16_t(v1);
  }

  uint32_t constexpr hash(const uint16_t& v0,const uint16_t& v1){
    return (uint32_t(v0)<<16)^(uint32_t(v1));
  }
  
  uint64_t constexpr hash(const uint32_t& v0,const uint32_t& v1){
    return (uint64_t(v0)<<32)^(uint64_t(v1));
  }
  
  template<typename T,size_t... I>
  size_t constexpr hash_tuple_impl(const T& t, index_sequence<I...>){
    return hash(std::get<I>(t)...);
  }

  template<typename... Ts>
  size_t constexpr hash(const tuple<Ts...>& t){
    return hash_tuple_impl(t,index_sequence_for<Ts...>{});
  }

  template<typename T,size_t n>
  size_t constexpr hash(const array<T,n> a){
    size_t h(0);
    for (size_t i=0;i!=n;++i) {
      if constexpr(sizeof(T)<=sizeof(size_t))
        if (i%(sizeof(size_t)/sizeof(T))==0) h = distribute(h); 
      h = hash(h,a[i]);
      if constexpr(sizeof(T)>sizeof(size_t)) h = distribute(h);
    }
    return h;
  }
  
  template <typename T, typename R, typename... Rest>
  size_t constexpr hash(const T& v, const R& r, Rest... rest) {
    return hash(hash(v),hash(r,rest...));
  }

  template<class K,class enable = void>
  struct hash_functor{
    typedef typename false_type::type is_injective;
    typedef typename false_type::type unhash_defined;
    size_t operator()(const K& k) const {
      return hash(k);
    }
  };
  
  template<class K>
  struct hash_functor<
    K,
    typename enable_if<is_fundamental<K>::value,void>::type
  >{
    // if size_t has at least as many digits as the hashed type the hash can
    // be injective and I will make it so
    typedef typename integral_constant<bool,sizeof(K)<=sizeof(size_t)>::type
      is_injective;
    typedef typename is_injective::type unhash_defined;
    auto constexpr operator()(const K& k) const {
      return hash(k);
    }
    auto constexpr unhash(const K& k) const {
      if constexpr (is_injective::value) return whash::unhash(k);
      else return K(0);
    }
  };
  
  template<typename T,size_t n>
  struct hash_functor<
    array<T,n>,void>
  {
    // if size_t has at least as many digits as the hashed type the hash can
    // be injective and I will make it so
    typedef typename integral_constant<bool,n*sizeof(T)<=sizeof(size_t)>::type
      is_injective;
    typedef typename false_type::type unhash_defined;
    auto constexpr operator()(const array<T,n>& k) const {
      return hash(k);
    }
  };
  
  template<typename T0,typename T1,typename T2>
  constexpr T0 clip(const T0& n,const T1& l,const T2& h){
    return n<l?l:n>h?h:n;
  }
  
  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr clz(const T x,const T lower=0,const T upper=digits<T>()){
    return (upper-lower==T(1))?digits<T>()-upper:
      (x&(T(0)-T(1)<<((upper+lower)/2))?
           clz(x,(upper+lower)/2,upper):
           clz(x,lower,(upper+lower)/2));
  }
 
  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr ctz(const T x,const T lower=0,const T upper=digits<T>()){
    return
      (upper-lower==T(1))?lower:(x&(T(0)-T(1)<<((upper+lower)/2))?
          ctz(x,(upper+lower)/2,upper):
          ctz(x,lower,(upper+lower)/2));
    // TODO
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
    return x==0?32:__builtin_clz(x);
  }
  
  uint32_t constexpr ctz(const uint32_t x){
    return x==0?32:__builtin_ctz(x);
  }
  
  uint64_t constexpr clz(const uint64_t x){
    return x==0?64:__builtin_clzll(x);
  }
  
  uint64_t constexpr ctz(const uint64_t x){
    return x==0?64:__builtin_ctzll(x);
  }

  uint32_t constexpr log2(const uint32_t x){
    return x==0?0:31-__builtin_clz(x);
  }
  
  uint64_t constexpr log2(const uint64_t x){
    return x==0?0:63-__builtin_clzll(x);
  }
#endif
  
  template <typename T,typename S>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr shl(const T n, const S i){
    if ((i<digits<T>())&&(i>=0)) return n<<i;
    return 0;
  }
  
  template <typename T,typename S>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr shr(const T n, const S i){
    if ((i<digits<T>())&&(i>=0)) return n>>i;
    return 0;
  }

  template<class key_type,
           class mapped_type,
           class hash        = hash_functor<key_type>,
           class equal       = std::equal_to<key_type>,
           class comp        = typename conditional<
             is_injective<hash>::value,
             dummy_comp<key_type>,
             std::less<key_type>>::type,
           class alloc       = typename boost::container::allocator<
             typename conditional<
               std::is_same<mapped_type,void>::value,
               std::pair<key_type,std::true_type>,
               std::pair<key_type,mapped_type>
             >::type,2>,
           bool dynamic      = true
          >
  class patchmap{
    public:
      typedef alloc allocator_type;
      typedef typename alloc::value_type value_type;
      typedef typename alloc::pointer value_pointer;
      typedef typename alloc::reference reference;
      typedef typename alloc::const_reference const_reference;
      typedef typename alloc::difference_type difference_type;
      typedef typename alloc::size_type size_type;
      typedef typename std::result_of<hash(key_type)>::type hash_type;
      typedef typename conditional<is_same<mapped_type,void>::value,
                                  std::true_type,
                                  mapped_type>::type
                                  _mapped_type;
    private:
      size_type num_data;
      size_type datasize;
      size_type masksize;
      value_type * data;
      size_type  * mask;
      allocator_type allocator;
      boost::container::allocator<size_type,2> maskallocator;
      comp  comparator;
      equal equator;
      hash  hasher;
      using uphold_iterator_validity = false_type;
      // size_type const inline masksize() const {
      //  return (datasize+digits<size_type>()-1)/digits<size_type>();
      //}
      template<typename T>
      const key_type& key_of(T&& value) const {
        if constexpr (is_same<void,mapped_type>::value) return value;
        else return value.first;
      }
      template<typename T>
      const _mapped_type& mapped_of(T&& value) const {
        if constexpr (is_same<void,mapped_type>::value) return std::true_type();
        else return value.second;
      }
      size_type inline map(
          const hash_type& h,
          const hash_type& n
          ) const {
        return get<0>(long_mul(h,n));
      }
      size_type inline map(const hash_type& h) const {
        return map(h,datasize);
      }
      size_type inline map_diff(
          const hash_type& h0,
          const hash_type& h1,
          const hash_type& n
          ) const {
        const auto lm = long_mul(hash_type(h0-h1),n);
        return get<0>(lm);
      }
      size_type inline map_diff(
          const hash_type& h0,
          const hash_type& h1
          ) const {
        return map_diff(h0,h1,datasize);
      }
      size_type inline map_diff_round(
          const hash_type& h0,
          const hash_type& h1,
          const hash_type& n
          ) const {
        const auto lm = long_mul(hash_type(h0-h1),n);
        return get<0>(lm)+(get<1>(lm)>((~hash_type(0))>>1));
      }
      size_type inline map_diff_round(
          const hash_type& h0,
          const hash_type& h1
          ) const {
        return map_diff_round(h0,h1,datasize);
      }
      hash_type inline order(const key_type& k) const {
        return hasher(k);
      }
      bool inline is_equal(
          const pair<key_type,hash_type> a,
          const key_type b) const {
        if constexpr (has_unhash<hash>::value) {
          return a.second == b;
        } else {
          return equator(a.first,b);
        }
      }
      bool inline is_less(
          const key_type& a,
          const key_type& b,
          const hash_type& oa,
          const hash_type& ob
          ) const {
        if constexpr (is_injective<hash>::value){
          assert(equator(a,b)==(oa==ob));
          if (oa<ob) return true;
          else       return false;
        } else {
          if (oa<ob) return true;
          if (oa>ob) return false;
          return comparator(a,b);
        }
      }
      bool inline is_less(
          const key_type& a,
          const key_type& b,
          const hash_type& oa
          ) const {
        return is_less(a,b,oa,order(b));
      }
      bool inline is_less(const key_type& a,const key_type& b) const {
        return is_less(a,b,order(a),order(b));
      }
      bool inline is_more(
          const key_type& a,
          const key_type& b,
          const hash_type& oa,
          const hash_type& ob
          ) const {
        
        if constexpr (is_injective<hash>::value){
          assert(equator(a,b)==(oa==ob));
          if (oa>ob) return true;
          else       return false;
        } else {
          if (oa>ob) return true;
          if (oa<ob) return false;
          return !((comparator(a,b))||(equator(a,b)));
        }
      }
      bool inline is_more(
          const key_type& a,
          const key_type& b,
          const hash_type& oa
          ) const {
        return is_more(a,b,oa,order(b));
      }
      bool inline is_more(
          const key_type& a,
          const key_type& b
          ) const {
        return is_more(a,b,order(a),order(b));
      }
      bool inline is_set(const size_type& n) const {        
        const size_type i = n/digits<size_type>();
        const size_type j = n%digits<size_type>();
        assert(i<masksize);
        return (mask[i]&(size_type(1)<<(digits<size_type>()-j-1)));
      }
      bool inline is_set_any(
          const size_type& lo,
          const size_type& hi) const {
        const size_type k0 = lo/digits<size_type>();
        const size_type l0 = lo%digits<size_type>();
        const size_type m0 = (~size_type(0))>>l0;
        const size_type k1 = hi/digits<size_type>();
        const size_type l1 = hi%digits<size_type>();
        const size_type m1 = (~size_type(0))<<(digits<size_type>()-l1-1);
        if (k0==k1) return ((m0&m1&mask[k0])!=0);
        if (((m0&mask[k0])!=0)||((m1&mask[k1])!=0)) return true;
        for (size_type i = k0+1;i!=k1;++i)
          if (mask[i]!=0) return true;
        return false;
      }
      void inline set(const size_type& n) {
        const size_type i = n/digits<size_type>();
        const size_type j = n%digits<size_type>();
        mask[i]|=size_type(1)<<(digits<size_type>()-j-1);
      }
      void inline unset(const size_type& n) {
        const size_type i = n/digits<size_type>();
        const size_type j = n%digits<size_type>();
        mask[i]&=((~size_type(0))^(size_type(1)<<(digits<size_type>()-j-1)));
      }
      void inline swap_set(const size_type& i,const size_type& j){
        if (is_set(i)==is_set(j)) return;
        if (is_set(i)){
          set(j);
          unset(i);
        }else{
          set(i);
          unset(j);
        }
      }
      /*hash_type inline index(const size_type& i) const {
        assert(i<datasize);
        if (is_set(i)) return order(data[i].first);
        else           return hash_type(i)*inversed;
      }*/
      bool inline index_key_is_less(const size_type& i,const key_type& k) const{
        if (is_set(i)) {
          if constexpr (has_unhash<hash>::value) {
            return is_less(unhash(data[i].first),k,data[i].first,order(k));
          } else {
            return is_less(data[i].first,k);
          }
        }
        return i<map(order(k));
      }
      bool inline key_index_is_less(const key_type& k,const size_type& i) const{
        if (is_set(i)) {
          if constexpr (has_unhash<hash>::value) {
            return is_less(k,hasher.unhash(data[i].first),
                           order(k),data[i].first);
          } else {
            return is_less(k,data[i].first);
          }
        }
        return map(order(k))<i;
      }
      bool inline index_index_is_less(const size_type& i,const size_type& j)
        const {
        assert(i<datasize);
        assert(j<datasize);
        if (is_set(i)&&is_set(j)) {
          if constexpr (has_unhash<hash>::value) {
            return data[i].first < data[j].first;
          } else {
            return is_less(data[i].first,data[j].first);
          }
        }
        if (is_set(i)) {
          if constexpr (has_unhash<hash>::value) {
            return map(data[i].first)<j;
          } else {
            return map(order(data[i].first))<j;
          }
        }
        if (is_set(j)) {
          if constexpr (has_unhash<hash>::value) {
            return i<map(data[j].first);
          } else {
            return i<map(order(data[j].first));
          }
        }
        return i<j;
      }
      bool inline index_index_is_more(const size_type& i,const size_type& j)
        const {
        return index_index_is_less(j,i);
      }
      size_type inline find_first() const {
        size_type i=0;
        if (i>=datasize) return ~size_type(0);
        while(true){
          const size_type k = i/digits<size_type>();
          const size_type l = i%digits<size_type>();
          const size_type m = (~size_type(0))>>l; 
          assert(k<masksize);
          size_type p = (mask[k]&m)<<l;
          if (k+1<masksize)
            p|=shr(mask[k+1]&(~m),digits<size_type>()-l);
          const size_type s = clz(p);
          if (s==0) return i;
          i+=s;
          if (i>=datasize) return ~size_type(0);
        }
      }
      // search for free bucket in decreasing order
      size_type inline search_free_dec(size_type i) const {
        while(true){
          const size_type k = i/digits<size_type>();
          const size_type l = i%digits<size_type>();
          const size_type m = (~size_type(0))<<(digits<size_type>()-l-1);
          assert(k<masksize);
                size_type p = ((~(mask[k]&m))>>(digits<size_type>()-l-1));
          if (k!=0) p|=shl(~(mask[k-1]&(~m)),l+1);
          const size_type s = ctz(p);
          if (s==0){
            assert(!is_set(i));
            return i;
          }
          i-=s;
          if (i>=datasize) return ~size_type(0);
        }
      }
      // search for free bucket in increasing order
      size_type inline search_free_inc(size_type i) const {
        while(true){
          const size_type k = i/digits<size_type>();
          const size_type l = i%digits<size_type>();
          const size_type m = (~size_type(0))>>l; 
          assert(k<masksize);
                size_type p = (~(mask[k]&m))<<l;
          if (k+1<masksize) p|=shr(~(mask[k+1]&(~m)),digits<size_type>()-l);
          const size_type s = clz(p);
          if (s==0){
            assert(!is_set(i));
            return i;
          }
          i+=s;
          if (i>=datasize) return ~size_type(0);
        }
      }
      // search for free bucket bidirectional
      size_type inline search_free_bidir_v0(size_type i) const {
        const size_type k = search_free_inc(i);
        const size_type l = search_free_dec(i);
        assert((k<datasize)||(l<datasize));
        if (k>=datasize) i=l;
        else if (l>=datasize) i=k;
        else if (k-i<i-l) i=k;
        else i=l;
        return i;
      }
      // search for free bucket truly bidirectional
      // this is optimal vor very high load factors > 0.98
      size_type inline search_free_bidir(const size_type& n) const {
        size_type i = n, j = n, si=~size_type(0),sj=~size_type(0);
        while(true){
          if ((i!=~size_type(0))&&si){
            const size_type k = i/digits<size_type>();
            const size_type l = i%digits<size_type>();
            const size_type m = (~size_type(0))>>l; 
                  size_type p = (~(mask[k]&m))<<l;
            if (k+1<masksize) p|=shr(~(mask[k+1]&(~m)),digits<size_type>()-l);
                            si= clz(p);
          }
          if (si==0){
            if (j==~size_type(0)) return i;
            if (i-n+digits<size_type>()<=n-j) return i;
          } else {
            i+=si;
            if (i>=datasize) i=~size_type(0);
            if ((i&j)==~size_type(0)) return ~size_type(0);
          }
          if ((j!=~size_type(0))&&sj){
            const size_type k = j/digits<size_type>();
            const size_type l = j%digits<size_type>();
            const size_type m = (~size_type(0))<<(digits<size_type>()-l-1);
                  size_type p = ((~(mask[k]&m))>>(digits<size_type>()-l-1));
            if (k!=0)       p|=shl(~(mask[k-1]&(~m)),l+1);
                            sj= ctz(p);
          }
          if (sj==0) {
            if (i==~size_type(0)) return j;
            if (n-j+digits<size_type>()<=i-n) return j;
          } else {
            j-=sj;
            if (j>=datasize) j=~size_type(0);
            if ((i&j)==~size_type(0)) return ~size_type(0);
          }
          if ((si==0)&&(sj==0)){
            if (i-n<=n-j) return i;
            else          return j;
          }
        }
        return ~size_type(0);
      }
      size_type const inline reserve_node(
          const  key_type&   k,
          const hash_type&  ok,
          const size_type& mok
          ){
        if (!is_set(mok)) {
          set(mok);
          ++num_data;
          return mok;
        }
        const size_type j = search_free_bidir_v0(mok);
        assert(j<datasize);
        assert(!is_set(j));
        set(j);
        ++num_data;
        size_type i = j;
        while(true){
          if (i==0) break;
          if (!is_set(i-1)) break;
          if constexpr (has_unhash<hash>::value) {
            if (data[i-1].first<ok) break;
          } else {
            if (is_less(data[i-1].first,k,order(data[i-1].first),ok)) break; 
          }
          swap(data[i],data[i-1]);
          --i;
        }
        if (i!=j) return i;
        while(true){
          if (i+1>=datasize) break;
          if (!is_set(i+1)) break;
          if constexpr (has_unhash<hash>::value) {
            if (data[i+1].first>ok) break;
          } else {
            if (is_less(k,data[i+1].first,ok,order(data[i+1].first))) break; 
          }
          swap(data[i],data[i+1]);
          ++i;
        }
        return i;
      }
      size_type inline reserve_node(
          const key_type&   k,
          const hash_type& ok){
        const hash_type mok = map(ok);
        return reserve_node(k,ok,mok);
      }
      size_type inline reserve_node(const key_type& k){
        const hash_type  ok = order( k);
        const size_type mok =   map(ok);
        assert(mok<datasize);
        return reserve_node(k,ok,mok);
      }
      size_type inline interpol(
          const hash_type& ok,
          const hash_type& olo,
          const hash_type& ohi,
          const size_type& lo,
          const size_type& hi
          ) const {
        auto lm             = long_mul(size_type(ok)-size_type(olo),hi-lo);
        // this is theoretically better but not worth the time
        //const hash_type tmp = get<1>(lm)+(ohi-olo)/2;
        //if (tmp<get<1>(lm)) ++get<0>(lm);
        //get<1>(lm)          = tmp;
        const size_type n   = clz(get<0>(lm));
        const size_type m   = digits<size_type>()-n;
        const hash_type den = (size_type(ohi)-size_type(olo))>>m;
        const hash_type nom = (get<0>(lm)<<n)+(get<1>(lm)>>m);
        return lo+nom/den;
      }

      size_type inline find_node_interpol(
        const  key_type&   k,
        const hash_type&  ok,
        const size_type& mok,
              size_type   lo,
              hash_type  olo,
              bool is_set_lo,
              size_type   hi,
              size_type  ohi,
              bool is_set_hi
          ) const {
        assert(lo<=hi||datasize==0);
        size_type mi;
        while(true) {
          if (hi-lo<8) {
            if (hi-lo<4) {
              if (hi-lo<2) {
                if (hi-lo<1) {
                  if (is_set(lo))
                    if (is_equal({k,ok},data[lo].first)) return lo;
                  return ~size_type(0);
                } else {
                  if (is_set_lo&&is_set_hi) return ~size_type(0);
                  if (is_set(lo))
                    if (is_equal({k,ok},data[lo].first)) return lo;
                  if (is_set(hi))
                    if (is_equal({k,ok},data[hi].first)) return hi;
                  return ~size_type(0);
                }
              } else {
                mi = lo + ((hi-lo)>>1);
              }
            } else {
              if (is_set_hi && is_set_lo) {
                mi = lo + ((hi-lo)>>1);
              } else if (is_set_lo) {
                mi = lo + ((hi-lo+2)>>2);
              } else if (is_set_hi) {
                mi = hi - ((hi-lo+2)>>2);
              } else {
                return ~size_type(0);
              } 
            }
          } else {
            if (is_set_hi && is_set_lo) {
              mi = interpol(ok,olo,ohi,lo,hi);
            } else if (is_set_lo) {
              const size_type st = map_diff(ok,olo);
              mi = lo+st<hi?lo+st:hi;
            } else if (is_set_hi) {
              const size_type st = map_diff(ohi,ok);
              mi = lo+st<hi?hi-st:lo;
            } else {
              return ~size_type(0);
            }
            mi = clip(mi,lo+1,hi-1);
          }
          if (!is_set(mi)) {
            if (mi<mok) {
              lo = mi;
              is_set_lo=false;
              continue;
            }
            if (mi>mok) {
              hi = mi;
              is_set_hi=false;
              continue;
            }
            return ~size_type(0);
          }
          if (is_equal({k,ok},data[mi].first)) return mi;
          const hash_type omi = (has_unhash<hash>::value)?
            data[mi].first:order(data[mi].first);
          if (ok<omi) {
            hi = mi;
            ohi = omi;
            is_set_hi = true;
            continue;
          }
          if (ok>omi) {
            lo = mi;
            olo = omi;
            is_set_lo = true;
            continue;
          }
          if constexpr (is_injective<hash>::value) {
            return ~size_type(0);
          } else {
            if (comparator(k,data[mi].first)) {
              hi = mi;
              ohi = omi;
              is_set_hi = true;
              continue;
            }
            if (comparator(data[mi].first,k)) {
              lo = mi;
              olo = omi;
              is_set_lo = true;
              continue;
            }
          }
          return ~size_type(0);
        }
        return ~size_t(0);
      }

      size_type inline find_node(
          const key_type &   k,
          const hash_type&  ok,
          const size_type& mok)
        const {
        assert((mok<datasize)||(datasize==0));
        if (datasize==0) return ~size_type(0);
        if (!is_set(mok)) return ~size_type(0);
        if constexpr (has_unhash<hash>::value) {
          if (data[mok].first == ok) return mok;
        } else {
          if (equator(data[mok].first,k)) return mok;
        }
        const hash_type omi = (has_unhash<hash>::value)?
          data[mok].first:order(data[mok].first);
        if (omi<ok) {
          return find_node_interpol(k,ok,mok,
              mok       ,omi          ,true ,
              datasize-1,~size_type(0),false);
        } else {
          return find_node_interpol(k,ok,mok,
              0         ,0            ,false,
              mok       ,          omi,true );
        }
      }
      
      size_type const inline find_node(
          const  key_type&  k,
          const size_type& ok
          ) const { return find_node(k,ok,map(ok)); }
      
      size_type const inline find_node(const key_type& k)
      const { return find_node(k,order(k)); }

      size_type const inline find_node_bruteforce(const key_type& k) const {
        for (size_type i = 0; i!=datasize; ++i)
          if (is_set(i)) if (is_equal({k,order(k)},data[i].first)) return i;
        return ~size_type(0);
      }

      void inline const restore_order() { // insertion sort
        for (size_type i=0;i!=datasize;++i){
          for(size_type j=i;j!=0;--j){
            if (index_index_is_less(j-1,j)) break;
            swap_set(j,j-1);
            swap(data[j],data[j-1]);
          }
        }
      }

      template<typename map_type>
      typename conditional<is_const<map_type>::value,
        const _mapped_type&,
              _mapped_type&>::type
      static inline const_noconst_at(map_type& hashmap,const key_type& k) {
        size_type i = hashmap.find_node(k);
        if (i<hashmap.datasize){
          assert(hashmap.is_set(i));
          hashmap.data[i].second;
        } else throw std::out_of_range(
            std::string(typeid(hashmap).name())
            +".const_noconst_at("+typeid(k).name()+" k)"
            +"key not found, array index "
            +to_string(i)+" out of bounds"
           );
      }
      void const resize_out_of_place(const size_type& n){
        size_type old_datasize = n;
        size_type old_masksize =
          (old_datasize+digits<size_type>()-1)/digits<size_type>();
        value_pointer old_data = allocator.allocate    (old_datasize); 
        size_type * old_mask = mask;
        old_mask = maskallocator.allocation_command(
            boost::interprocess::allocate_new|boost::interprocess::zero_memory,
            old_masksize,
            old_masksize,
            old_mask);
        for (size_type i=0;i!=old_masksize;++i) old_mask[i]=0;
        num_data = 0;
        swap(old_mask,mask);
        swap(old_data,data);
        swap(old_datasize,datasize);
        swap(old_masksize,masksize);
        for (size_type n=0;n<old_datasize;++n) {
          const size_type i = n/digits<size_type>();
          const size_type j = n%digits<size_type>();
          if (old_mask[i]&(size_type(1)<<(digits<size_type>()-j-1))){
            size_type l;
            if constexpr (has_unhash<hash>::value) {
              l = reserve_node(key_type(0),old_data[n].first);
            } else {
              reserve_node(old_data[n].first);
            }
            allocator_traits<alloc>::construct(allocator,data+l,old_data[n]);
            set(l);
          }
        }
        assert(check_ordering());
        maskallocator.deallocate(old_mask,old_masksize);
        allocator.deallocate    (old_data,old_datasize);
      }
    public:
      // constructor
      patchmap(const size_type& datasize = 0)
        :datasize(datasize)
      {
        num_data = 0;
        if (datasize)
          data = allocator_traits<alloc>::allocate(allocator,datasize);
        else data = nullptr;
        masksize = (datasize+digits<size_type>()-1)/digits<size_type>();
        if (masksize) mask = maskallocator.allocate(masksize);
        else mask = nullptr;
        for (size_type i=0;i!=masksize;++i) mask[i]=0;
      }
      ~patchmap(){                                  // destructor
        maskallocator.deallocate(mask,masksize);
        allocator_traits<alloc>::deallocate(allocator,data,datasize);
      }
      patchmap(patchmap&& other) noexcept  // move constructor
      {
        mask = nullptr;
        masksize = 0;
        data = nullptr;
        datasize = 0;
        swap(mask,other.mask);
        swap(data,other.data);
        swap(datasize,other.datasize);
        swap(masksize,other.masksize);
      }
      template<
        class key_type_other,
        class mapped_type_other,
        class hash_other,
        class equal_other,
        class comp_other,
        class alloc_other
              >
      inline patchmap& operator=                   // copy assignment
        (const patchmap<
           key_type_other,
           mapped_type_other,
           hash_other,
           equal_other,
           comp_other,
           alloc_other
         >& other)
      {
        typedef patchmap<
           key_type_other,
           mapped_type_other,
           hash_other,
           equal_other,
           comp_other,
           alloc_other
         > other_type;
        maskallocator.deallocate(mask,masksize);
        allocator_traits<alloc>::deallocate(allocator,data,datasize);
        num_data = other.num_data;
        datasize = other.datasize;
        masksize = other.masksize;
        if (masksize) mask = maskallocator.allocate(masksize);
        else mask = nullptr;
        for (size_type i=0;i!=masksize;++i) mask[i]=0;
        if (datasize)
          data = allocator_traits<alloc>::allocate(allocator,datasize);
        else data = nullptr;
        if constexpr (
            is_same<hash , hash_other>::value
          &&is_same<equal,equal_other>::value
          &&is_same<comp , comp_other>::value
          ){
          memcpy(reinterpret_cast<void*>(mask),
                 reinterpret_cast<void*>(other.mask),
                 masksize*sizeof(size_t));
          if constexpr (
              is_trivially_copyable<value_type>::value
            &&is_same<value_type,typename other_type::value_type>::value)
            memcpy(reinterpret_cast<void*>(data),
                   reinterpret_cast<void*>(other.data),
                   datasize*sizeof(value_type));
          else for (size_type i=0;i!=datasize;++i) data[i]=other.data[i];
        } else {
          for (auto it=other.begin();it!=other.end();++it) insert(*it);
        }
      }
      patchmap(const patchmap& other){
        num_data = other.num_data;
        datasize = other.datasize;
        masksize = other.masksize;
        if (masksize) mask = maskallocator.allocate(masksize);
        else mask = nullptr;
        for (size_type i=0;i!=masksize;++i) mask[i]=0;
        if (datasize)
          data = allocator_traits<alloc>::allocate(allocator,datasize);
        else data = nullptr;
        memcpy(reinterpret_cast<void*>(mask),
               reinterpret_cast<void*>(other.mask),
               masksize*sizeof(size_t));
          if constexpr (is_trivially_copyable<value_type>::value)
            memcpy(reinterpret_cast<void*>(data),
                   reinterpret_cast<void*>(other.data),
                   datasize*sizeof(value_type));
        else for (size_type i=0;i!=datasize;++i) data[i]=other.data[i];
      }
      inline patchmap& operator=                   // copy assignment
        (const patchmap& other)
      {
        return *this = patchmap(other);
      }
      inline patchmap& operator=                   // move assignment
        (patchmap&& other)
        noexcept{
        swap(mask,other.mask);
        swap(data,other.data);
        swap(datasize,other.datasize);
        swap(masksize,other.masksize);
        return *this;
      }
      void print() const {
        cerr << datasize << " " << num_data << endl;
        for (size_type i=0;i!=datasize;++i) {
          cout << std::fixed << std::setprecision(16);
          const size_type  ok = has_unhash<hash>::value?
            data[i].first:order(data[i].first);
          const size_type mok = map(ok);
          if (is_set(i)) cout << setw(6) << i;
          else           cout << "      "    ;
                         cout << setw(20) << frac(uint32_t(ok))
                              << setw(20) << frac(uint32_t(data[i].second));
          if (is_set(i)) cout << setw( 8) << mok
                              << setw( 8) << int(mok)-int(i);
          else           cout << setw( 8) << i
                              << setw( 8) << 0;
          cout << endl;
        }
        cout << endl;
      }
      size_type erase(
          const  key_type&   k,
          const hash_type&  ok,
          const size_type& mok){
        size_type i = find_node(k,ok,mok);
        if (i>=datasize) return 0;
        const size_type j = i;
        while(true){
          if (i+1==datasize) break;
          if (!is_set(i+1)) break;
          if constexpr (has_unhash<hash>::value) {
            if (map(data[i+1].first)>i) break;
          } else {
            if (map(order(data[i+1].first))>i) break;
          }
          swap(data[i],data[i+1]);
          ++i;
        }
        if (i==j){
          while(true){
            if (i==0) break;
            if (!is_set(i-1)) break;
            if constexpr (has_unhash<hash>::value) {
              if (map(data[i-1].first)<i) break;  
            } else {
              if (map(order(data[i-1].first))<i) break;
            }
            swap(data[i],data[i-1]);
            --i;
          }
        }
        unset(i);
        data[i]=value_type();
        --num_data;
        assert(num_data<datasize);
        assert(check_ordering());
        return 1;
      }
      size_type erase(const  key_type& k,const size_type& ok){
        const hash_type mok = map(ok);
        return erase(k,ok,mok);
      }
      size_type erase(const key_type & k){
        const size_type ok = order(k);
        return erase(k,ok);
      }
      void inline clear(){
        for (size_type i=0;i!=masksize;++i) mask[i]=0;
        num_data=0;
      }
      void const resize(const size_type& n){
        if (n<num_data) return;
        if (VERBOSE_PATCHMAP)
          cerr << "resizing from " << datasize << " to " << n << endl;
        resize_out_of_place(n); return;
        if constexpr (!is_same<
            alloc,
            boost::container::allocator<std::pair<key_type,mapped_type>,2>
            >::value) {
          resize_out_of_place(n);
          return;
        }
        if (datasize == 0){
          resize_out_of_place(n);
          return;
        }
        resize_out_of_place(n); return;
        if (VERBOSE_PATCHMAP)
          cerr << "attempting to resize in place " << endl;
        size_type old_datasize = n;
        size_type min_datasize = (datasize+old_datasize+1)/2;
        value_pointer reuse = data;
        value_pointer old_data = allocator.allocation_command(
          //boost::interprocess::expand_bwd
            boost::interprocess::allocate_new
           |boost::interprocess::expand_fwd,
            min_datasize,
            old_datasize,
            reuse);
        //cerr << "scary allocation_command did not crash" <<  endl;
        size_type old_masksize =
          (old_datasize+digits<size_type>()-1)/digits<size_type>();
        //cerr << "allocating mask " << old_masksize << endl;
        /*size_type * old_mask = maskallocator.allocation_command(
            boost::interprocess::allocate_new|boost::interprocess::zero_memory,
            old_masksize,
            old_masksize,
            nullptr);*/
        swap(datasize,old_datasize);
        // should not be necessary but it is, boost has not implemented
        // zero_memory yet :( TODO myself
        if (reuse){
          //if (old_data<data) cerr << "backwards expansion, oO " << endl;
          if (VERBOSE_PATCHMAP)
            cerr << "in place resize, yea :D " << endl;
          size_type * reuse = mask;
          size_type * old_mask = maskallocator.allocation_command(
            boost::interprocess::allocate_new
           |boost::interprocess::expand_fwd,
            old_masksize,
            old_masksize,
            reuse);
          if (!reuse) for(size_type i=0;i<masksize;++i) old_mask[i]=mask[i];
          for(size_type i=masksize; i<old_masksize;++i) old_mask[i]=0;
          if (!reuse) swap(old_mask,mask);
          swap(old_masksize,masksize);
          num_data = 0;
          //cerr << old_data << " "<< data << endl;
          for (size_type n=old_datasize-1;n<old_datasize;--n){
            if (is_set(n)){
            //const size_type i = n/digits<size_type>();
            //const size_type j = n%digits<size_type>();
            //if (old_mask[i]&(size_type(1)<<(digits<size_type>()-j-1))){
              value_type temp = move(data[n]);
              unset(n);
              const size_type l = reserve_node(temp.first);
              data[l] = move(temp);
            }
          }
          assert(check_ordering());
          if (!reuse) maskallocator.deallocate(old_mask,old_masksize); 
        } else {
          size_type * old_mask = maskallocator.allocate(old_masksize);
          for (size_type i=0;i!=old_masksize;++i) old_mask[i]=0;
          swap(old_mask,mask);
          swap(old_masksize,masksize);
          num_data = 0;
          swap(old_data,data);
          if (VERBOSE_PATCHMAP)
            cerr << "no reuse :/ " << endl;
          //cerr << old_data << " "<< data << endl;
          for (size_type n=0;n<old_datasize;++n){
            const size_type i = n/digits<size_type>();
            const size_type j = n%digits<size_type>();
            if (old_mask[i]&(size_type(1)<<(digits<size_type>()-j-1))){
              const size_type l = reserve_node(old_data[n].first);
              data[l] = old_data[n];
            }
          }
          assert(check_ordering());
          //cerr << "scary free " << endl;
          allocator.deallocate    (old_data,old_datasize);
          maskallocator.deallocate(old_mask,old_masksize); 
        }
        //cerr << "deallocating old mask" << endl;
      }
      size_type inline size() const { return num_data; }
      size_type const test_size() const {
        size_type test = 0;
        for (size_type i=0;i!=datasize;++i) test += is_set(i);
        return test;
      }
      bool check_ordering() const {
        bool ordered = true;
        for (size_type i=0,j=1;j<datasize;(++i,++j)){
          if (!index_index_is_less(j,i)) continue;
          cout << std::fixed << std::setprecision(16)
               << is_set(i) << " " << is_set(j) << " "
               << i << " " << j << " "
               << data[i].first << " " << data[j].first << endl;
          ordered = false;
        }
        if (!ordered) print();
        return ordered;
      }
      bool check_ordering(const size_type& i) const {
        if (  i>0       ) if (!index_index_is_less(i-1,i)) return false;
        if (i+1<datasize) if (!index_index_is_less(i,i+1)) return false;
        return true;
      }
      void inline ensure_size(){
        if constexpr (!dynamic) return;
#if defined PATCHMAP_EXPANSIVE
        //if (num_data*9<datasize*7) return; 
        if (num_data*5<datasize*4) return;
        if (datasize) {
          size_type nextsize = (12*datasize+6)/7;
          nextsize = (nextsize+digits<size_type>()-1)/digits<size_type>();
          nextsize*= digits<size_type>();
          resize(nextsize);
        } else {
          resize(256);
        }
        return;
#endif
        if (num_data*8 < datasize*7 ) return;
        size_type nextsize;
        if (datasize < 257) {
          if (datasize == 0) {
            nextsize = digits<size_type>();
          } else {
            nextsize = 2*datasize;
            nextsize = (nextsize+digits<size_type>()-1)/digits<size_type>();
            nextsize*= digits<size_type>();
          }
        } else {
          //nextsize = 50*datasize/31;
          //nextsize = 48*datasize/31;
          nextsize = 47*datasize/31;
          //nextsize = 45*datasize/31;
          nextsize = (nextsize+digits<size_type>()-1)/digits<size_type>();
          nextsize*= digits<size_type>();
        }
        resize(nextsize);
      }
      _mapped_type& operator[](const key_type& k){
        const size_type i = find_node(k);
        if (VERBOSE_PATCHMAP) cerr << "i = " << i << endl; 
        if (i<datasize) return data[i].second;
        ensure_size();
        const size_type j = reserve_node(k);
        if (VERBOSE_PATCHMAP) cerr << "j = " << j << endl;
        if constexpr (has_unhash<hash>::value) {
          allocator_traits<alloc>::construct(allocator,data+j,
              order(k),_mapped_type());
        } else {
          allocator_traits<alloc>::construct(allocator,data+j,k,_mapped_type());
        }
        assert(check_ordering());
        return data[j].second;
      }
      const _mapped_type& operator[](const key_type& k) const {
        const size_type i = find_node(k);
        if (i<datasize) data[i].second;
        else throw std::out_of_range(
            std::string(typeid(*this).name())
            +".const_noconst_at("+typeid(k).name()+" k)"
            +"key not found, array index "
            +to_string(i)+" out of bounds"
           );
      }
      _mapped_type& at(const key_type& k){
        return const_noconst_at(*this,k);
      }
      const _mapped_type& at(const key_type& k) const {
        return const_noconst_at(*this,k);
      }
      size_type const inline count(const key_type& k) const {
        return (find_node(k)<datasize);
      }
      double average_offset(){
        double v = 0;
        for (size_type i=0;i!=datasize;++i){
          if (is_set(i)){
            v+=double(map(data[i].first))-double(i);
            cout << map(order(data[i].first)) << " " << i << " "
                 << datasize << endl;
          }
        }
        return v/size()/datasize;
      }
      void print_offsets(){
        for (size_type i=0;i!=datasize;++i){
          if (is_set(i)) cout << map(order(data[i].first)) << " " << i << endl;
          //else           cout << i                         << " " << i << endl;
        }
      }
      template<class key_type_other,
               class mapped_type_other,
               class hash_other,
               class equal_other,
               class comp_other,
               class alloc_other
              >
      bool operator==(
          const patchmap<
            key_type_other,
            mapped_type_other,
            hash_other,
            equal_other,
            comp_other,
            alloc_other>& other)
      const {
        if (datasize!=other.datasize) return false;
        if constexpr (
            is_same<hash , hash_other>::value
          &&is_same<equal,equal_other>::value
          &&is_same<comp , comp_other>::value
          ){
          auto it0 = begin();
          auto it1 = other.begin();
          while (true){
            if (it0==end()) return true;
            if ((*it0)!=(*it1)) return false;
            ++it0;++it1;
          }
        } else {
          for (auto it=other.begin();it!=other.end();++it){
            if (nount(it->first)) if (at(it->first)==it->second) continue;
            return false;
          }
          return true;
        }
      }
      template<class key_type_other,
               class mapped_type_other,
               class hash_other,
               class equal_other,
               class comp_other,
               class alloc_other
              >
      bool operator!=(
          const patchmap<
            key_type_other,
            mapped_type_other,
            hash_other,
            equal_other,
            comp_other,
            alloc_other>& o)
      const{ return !((*this)==o); }
      equal key_eq() const{ // get key equivalence predicate
        return equal{};
      }
      comp key_comp() const{ // get key order predicate
        return comp{};
      }
      alloc get_allocator() const{
        return allocator;
      }
      hash hash_function() const{ // get hash function
        return hash{};
      }  
      template<bool is_const>
      class const_noconst_iterator {
        friend class patchmap;
        public:
          size_type hint;
          key_type key;
          typename conditional<is_const,
                               const patchmap*,
                               patchmap*
                              >::type map;
        private:
          void inline update_hint(){
            if constexpr (!uphold_iterator_validity::value) return;
            if (hint<map->datasize)
              if (equal{}(map->data[hint].first,key)) return;
            hint = map->find_node(key,hint);
            if (hint>=map->datasize) hint = ~size_type(0);
          }
          void inline unsafe_increment(){ // assuming hint is valid
            if (++hint>=map->datasize){
              hint=~size_type(0);
              return;
            }
            while(true){
              const size_type k = hint/digits<size_type>();
              const size_type l = hint%digits<size_type>();
              const size_type m = (~size_type(0))>>l; 
              assert(k<map->masksize);
              size_type p = (map->mask[k]&m)<<l;
              if (k+1<map->masksize)
                p|=shr(map->mask[k+1]&(~m),digits<size_type>()-l);
              const size_type s = clz(p);
              if (s==0) break;
              hint+=s;
              if (hint>=map->datasize){
                hint=~size_type(0);
                return;
              }
            }
            key = map->data[hint].first;
          }
          void inline unsafe_decrement(){ // assuming hint is valid
            if (--hint>=map->datasize){
              hint=~size_type(0);
              return;
            }
            while(true){
              const size_type k = hint/digits<size_type>();
              const size_type l = hint%digits<size_type>();
              const size_type m = (~size_type(0))<<(digits<size_type>()-l-1);
              assert(k<map->masksize);
              size_type p = (map->mask[k]&m)>>(digits<size_type>()-l-1);
              if (k!=0) p|=shl(map->mask[k-1]&(~m),l+1);
              const size_type s = ctz(p);
              if (s==0) break;
              hint-=s;
              if (hint>=map->datasize){
                hint=~size_type(0);
                return;
              }
            }
            key = map->data[hint].first;
          }
          template<bool is_const0,bool is_const1>
            difference_type inline friend diff(
                const_noconst_iterator<is_const0>& it0,
                const_noconst_iterator<is_const1>& it1){
              if (it1<it0) return -diff(it0,it1);
              it0.update_hint();
              it1.update_hint();
              const size_type k0 = it0->hint/digits<size_type>();
              const size_type l0 = it0->hint%digits<size_type>();
              const size_type m0 = (~size_type(0))>>l0;
              const size_type k1 = it1->hint/digits<size_type>();
              const size_type l1 = it1->hint%digits<size_type>();
              const size_type m1 = (~size_type(0))<<(digits<size_type>()-l1-1);
              if (k0==k1) return popcount(m0&m1&it0.map->mask[k0])-1;
            size_type d = popcount(m0&it0.map->mask[k0])
                         +popcount(m1&it1.map->mask[k1]);
            for (size_type i = k0+1;i!=k1;++i)
              d+=popcount(it0.map->mask[i]);
            return d;
          }
          void inline add(const size_type& n){
            update_hint();
                  size_type k = hint/digits<size_type>();
            const size_type l = hint%digits<size_type>();
            const size_type m = (~size_type(0))>>l;
                  size_type i = 0;
                  size_type p = popcount(map->mask[k]&m)-1; 
            while (i+p<n){
              if (++k>=map->mapsize){
                hint=~size_type(0);
                return;
              }
              hint+=digits<size_type>();
              p = popcount(map->mask[k]);
            }
            for (;i!=n;++i) unsafe_increment();
            key = map->data[hint].first;
          }
          void inline sub(const size_type& n){
            update_hint();
                  size_type k = hint/digits<size_type>();
            const size_type l = hint%digits<size_type>();
            const size_type m = (~size_type(0))<<(digits<size_type>()-l-1);
                  size_type i = 0;
                  size_type p = popcount(map->mask[k]&m)-1;
            while (i+p<n){
              if (--k>=map->mapsize){
                hint=~size_type(0);
                return;
              }
              hint+=digits<size_type>();
              p = popcount(map->mask[k]);
            }
            for (;i!=n;++i) unsafe_decrement();
            key = map->data[hint].first;
          }
        public:
          typedef typename alloc::difference_type difference_type;
          typedef typename alloc::value_type value_type;
          typedef pair<const key_type,
            typename conditional<is_const,const mapped_type&,mapped_type&>::type
          > reference;
          typedef std::unique_ptr<pair<const key_type,
            typename conditional<is_const,const mapped_type&,mapped_type&>::type
          >> pointer;
          typedef std::bidirectional_iterator_tag iterator_category;
          const_noconst_iterator(){
            //cout << "constructor 0" << endl;
          }
          const_noconst_iterator(
            const size_t& hint,
            typename conditional<is_const,
                                 const patchmap*,
                                       patchmap*
                                >::type map)
            :hint(hint),key(key_type{}),map(map){
            //cout << "constructor 1 " << hint << endl;
          }
          const_noconst_iterator(
            const size_t& hint,
            const key_type& key,
            typename conditional<is_const,
                                 const patchmap*,
                                       patchmap*
                                >::type map)
            :hint(hint),key(key),map(map) {
              //cout << "constructor 2 " << hint << endl;
          }
          ~const_noconst_iterator(){
          //cout << "destructor of const_noconst_iterator " << is_const << endl;
          //cout << hint << endl;
          }
          // copy constructor
          template<bool is_const_other>
          const_noconst_iterator(const const_noconst_iterator<is_const_other>& o)
          :hint(o.hint),key(o.key),map(o.map){
            //cout << "copy constructor" << endl;
          }
          // move constructor
          template<bool is_const_other>
          const_noconst_iterator(
              const_noconst_iterator<is_const_other>&& o) noexcept{
            //cout << "move constructor" << endl;
            swap(hint,o.hint);
            swap(key,o.key);
            swap(map,o.map);
          }
          // copy assignment
          template<bool is_const_other>
          const_noconst_iterator<is_const>& operator=(
              const const_noconst_iterator<is_const_other>& other){
            //cout << "copy assignment" << endl;
            return  (*this=const_noconst_iterator<is_const>(other));
          }
          template<bool is_const_other>
          bool operator==(
              const const_noconst_iterator<is_const_other>& o) const {
            if ((hint>=map->datasize)&&(o.hint>=o.map->datasize)) return true;
            if ((hint>=map->datasize)||(o.hint>=o.map->datasize)) return false;
            if (key!=o.key) return false;
            return true;
          }
          template<bool is_const_other>
          bool operator!=(
              const const_noconst_iterator<is_const_other>& o) const{
            return !((*this)==o);
          }
          template<bool is_const_other>
          bool operator< (
              const const_noconst_iterator<is_const_other>& o) const{
            if ((o.hint<o.map->datasize)){
              if (hint<map->datasize){
                return comp(key,o.key);
              }else{
                return false;
              }
            } else {
              return false;
            }
          }
          template<bool is_const_other>
          bool operator> (
              const const_noconst_iterator<is_const_other>& o) const{
            if ((o.hint<o.map->datasize)){
              if (hint<map->datasize){
                return (!comp(key,o.key))&&(!equal(key,o.key));
              }else{
                return true;
              }
            } else {
              return false;
            }
          }
          template<bool is_const_other>
          bool operator<=(
              const const_noconst_iterator<is_const_other>& o) const{
            if ((o.hint<o.map->datasize)){
              if (hint<map->datasize){
                return comp(key,o.key)||equal(key,o.key);
              }else{
                return false;
              }
            } else {
              return true;
            }
          }
          template<bool is_const_other>
          bool operator>=(
              const const_noconst_iterator<is_const_other>& o) const{
            if ((o.hint<o.map->datasize)){
              if (hint<map->datasize){
               return !comp(key,o.key);
              }else{
                return true;
              }
            } else {
              return true;
            }
          }
          const_noconst_iterator<is_const>& operator++(){   // prefix
            update_hint();
            unsafe_increment();
            return *this;
          }
          const_noconst_iterator<is_const> operator++(int){ // postfix
            update_hint();
            iterator pre(*this);
            unsafe_increment();
            return pre;
          }
          const_noconst_iterator<is_const>& operator--(){   // prefix
            update_hint();
            unsafe_decrement();
            return *this;
          }
          const_noconst_iterator<is_const> operator--(int){ // postfix
            update_hint();
            iterator pre(*this);
            unsafe_decrement();
            return pre;
          }
          // not a random_acces_iterator but we can still do better than default
          template<bool is_const_other>
          difference_type operator-(
              const const_noconst_iterator<is_const_other>& o) const {
            iterator it0(*this);
            iterator it1(o);
            return diff(it0,it1);
          }
          const_noconst_iterator<is_const>& operator+=(const size_type& n){
            add(n);
            return *this;
          }
          const_noconst_iterator<is_const> operator+(const size_type& n) const {
            return (const_noconst_iterator<is_const>(*this)+=n);
          }
          friend const_noconst_iterator<is_const> operator+(
              const size_type& n,
              const const_noconst_iterator<is_const>& it){
            return (const_noconst_iterator<is_const>(it)+=n);
          }
          const_noconst_iterator<is_const>& operator-=(const size_type& n){
            sub(n);
            return *this;
          }
          const_noconst_iterator<is_const> operator-(const size_type& n) const{
            return (const_noconst_iterator<is_const>(*this)-=n);
          }
          reference operator*() {
            update_hint();
            return {map->hasher.unhash(map->data[hint].first),
                    map->data[hint].second};
          }
          auto operator->() {
            update_hint();
            if constexpr (is_same<void,mapped_type>::value) {
              if constexpr (has_unhash<hash>::value) {
                return make_unique<key_type>(
                    map->hasher.unhash(map->data[hint]));
              } else {
                return &map->data[hint];
              }
            }
            if constexpr (has_unhash<hash>::value) {
              return make_unique<pair<const key_type,mapped_type&>>(
                  map->hasher.unhash(map->data[hint].first),
                  map->data[hint].second);
            } else {
                return &map->data[hint];
            }
          }
          auto operator*() const {
            size_type i;
            if (hint>=map->datasize) {
              i = map->find_node(key);
            } else {
              if (map->is_set(hint)) {
                if (map->data[hint]!=key) {
                  i = hint;
                } else {
                  i = map->find_node(key);
                }
              } else {
                i = map->find_node(key);
              }
            }
            if constexpr (has_unhash<hash>::value) {
              if constexpr (is_same<void,mapped_type>::value) {
                return map->hasher.unhash(map->data[i].first);
              } else {
                return pair<const key_type,mapped_type&>
                    {map->hasher.unhash(map->data[i].first), 
                     map->data[i].second};
              }
            } else {
              return pair<const key_type&,mapped_type&>{map->data[i].first, 
                      map->data[i].second};
            }
          }
          auto operator->() const {
            size_type i;
            if (hint>=map->datasize) {
              i = map->find_node(key);
            } else {
              if (map->is_set(hint)) {
                if (map->data[hint]!=key) {
                  i = hint;
                } else {
                  i = map->find_node(key);
                }
              } else {
                i = map->find_node(key);
              }
            }
            if constexpr (is_same<void,mapped_type>::value) {
              if constexpr (has_unhash<hash>::value) {
                return make_unique<key_type>(map->hasher.unhash(map->data[i]));
              } else {
                return &map->data[hint];
              }
            }
            if constexpr (has_unhash<hash>::value) {
              return make_unique<pair<const key_type,mapped_type&>>(
                  map->hasher.unhash(map->data[i].first),
                  map->data[i].second);
            } else {
                return &map->data[i];
            }
            return make_unique<pair<const key_type,mapped_type&>>(
                map->hasher.unhash(map->data[i].first),
                map->data[i].second);
          }
    };
    typedef const_noconst_iterator<false> iterator;
    typedef const_noconst_iterator<true>  const_iterator;    
    iterator begin(){
      const size_type i = find_first();
      return iterator(i,data[i].first,this);
    }
    const_iterator begin() const {
      const size_type i = find_first();
      return const_iterator(i,data[i].first,this);
    }
    const_iterator cbegin() const {
      const size_type i = find_first();
      return const_iterator(i,data[i].first,this);
    }
    iterator end() {
      const size_type i = find_first();
      return iterator(~size_type(0),this);
    }
    const_iterator end() const {
      return const_iterator(~size_type(0),this);
    }
    const_iterator cend() const {
      return const_iterator(~size_type(0),this);
    }
    // void swap(unpatchmap&); // TODO
    size_type max_size()         const {
      return std::numeric_limits<size_type>::max();
    }
    bool empty()                 const {return (num_data==0);}
    size_type bucket_count()     const {return datasize;}
    size_type max_bucket_count() const {
      return std::numeric_limits<size_type>::max();
    }
    void rehash(const size_type& n) { if (n>=size()) resize(n); }
    void reserve(const size_type& n){ if (3*n>=2*(size()+1)) resize(n*3/2); }
    template<class _value_type>
    pair<iterator,bool> insert ( const _value_type& val ){
      const size_type i = find_node(key_of(val));
      if (i<datasize) return {iterator(i,key_of(val),this),false};
      ensure_size();
      const size_type j = reserve_node(key_of(val));
      if constexpr (has_unhash<hash>::value) {
        if constexpr (is_same<void,mapped_type>::value) {
          allocator_traits<alloc>::construct(allocator,data+j,
              order(key_of(val)),true_type{});
        } else {
          allocator_traits<alloc>::construct(allocator,data+j,
              order(key_of(val)),mapped_of(val));
        }
      } else {
        allocator_traits<alloc>::construct(allocator,data+j,val);
      }
      return {{j,key_of(val),this},true};
    }
    iterator insert ( const_iterator hint, const value_type& val ){
      const size_type i = find_node(key_of(val),hint.hint);
      if (i<datasize) return {iterator(i,key_of(val),this),false};
      ensure_size();
      const size_type j = reserve_node(key_of(val));
      if constexpr (is_same<void,mapped_type>::value)
        allocator_traits<alloc>::construct(allocator,data+j,{val,{}});
      else
        allocator_traits<alloc>::construct(allocator,data+j,val);
      return {{j,key_of(val),this},true};
    }
    template <class P>
    iterator insert ( const_iterator hint, P&& val ){
      const size_type i = find_node(key_of(val),hint.hint);
      if (i<datasize) return {iterator(i,key_of(val),this),false};
      ensure_size();
      const size_type j = reserve_node(key_of(val));
      allocator_traits<alloc>::construct(allocator,data+j,{val,{}});
      return {{j,key_of(val),this},true};
    }
    template <class InputIterator>
    void insert ( InputIterator first, InputIterator last ){
      for (auto it(first);it!=last;++it){
        insert(*it);
      }
    }
    void insert ( initializer_list<value_type> il ){
      insert(il.begin(),il.end());
    }
    template <class... Args>
    pair<iterator, bool> emplace ( Args&&... args ){
      insert(value_type(args...));
    }
    template <class... Args>
    iterator emplace_hint(const_iterator position,Args&&... args){
      insert(position,value_type(args...));
    }
    pair<iterator,iterator> equal_range(const key_type& k){
      const size_type i = find_node(k);
      if (i>=datasize) return {end(),end()};
      iterator lo(i,data[i].first,this);
      iterator hi(lo);
      ++hi;
      return {lo,hi};
    }
    pair<const_iterator,const_iterator>
    equal_range ( const key_type& k ) const{
      const size_type i = find_node(k);
      if (i>=datasize) return {cend(),cend()};
      iterator lo(i,data[i].first,this);
      iterator hi(lo);
      ++hi;
      return {lo,hi};
    }
    float load_factor() const noexcept{
      return float(num_data)/float(datasize);
    }
    float average_patchsize() const noexcept{
      double avg = 0;
      double counter = 0;
      for (size_type i=0;i<datasize;++i){
        const size_type j = search_free_inc(i);
        if (j<datasize) avg += (j-i);
        else break;
        i=j;
        ++counter;
      }
      return avg/counter;
    }
    void print_patchsizes() const noexcept {
      for (size_type i=0;i<datasize;++i){
        const size_type j = search_free_inc(i);
        if (j<datasize) cout << j-i << endl;
        else break;
        i=j;
      }
    }
    /*void print_offsethist(){
      patchmap<int,size_t> hist;
      for (size_type i=0;i!=datasize;++i)
        ++hist[int(map(order(data[i].first)))-int(i)];
      for (auto it=hist.begin();it!=hist.end();++it)
        cout << it->first << " " << it->second << endl;
    }*/
    float const max_load_factor() const noexcept {
      return 1.0;
    }
    template<bool is_const>
    iterator erase(const_noconst_iterator<is_const> position){
      iterator it(position);
      ++it;
      erase(position.key);//,position.hint);
      return it;
    }
    template<bool is_const>
    iterator erase(
        const_noconst_iterator<is_const> first,
        const_noconst_iterator<is_const> last){
      for (auto it=first;it!=last;it=erase(it));
    }
    [[deprecated(
        "disabled for performance reasons"
    )]] void max_load_factor(float z) {
      // m = number of elements ; n = number of buckets
      // n*load_factor >= m
      // n*mul_n >= m*mul_m
      size_type mul_n =  ceil(z*16);
      size_type mul_m = floor(z*16);
    }
  }; 

  /* TODO
  template<class K,
           class T,
           class hash=hash_functor<K>,
           class equal = std::equal_to<K>,
           class comp = std::less<K>,
           class A = std::allocator<std::pair<K,T>>
          >
  void swap(patchmap<K,K,hash,equal,comp,A>&,
            patchmap<K,K,hash,equal,comp,A>&);
  */
  
  template<class key_type    = int,  // int is the default, why not
           class mapped_type = int,  // int is the default, why not
           class hash        = hash_functor<key_type>,
           class equal       = std::equal_to<key_type>,
           class comp        = typename conditional<is_injective<hash>::value,
                                                    dummy_comp<key_type>,
                                                    std::less<key_type>>::type,
           class alloc       = typename boost::container::allocator<
             typename conditional<
               std::is_same<mapped_type,void>::value,
               std::pair<key_type,true_type>,
               std::pair<key_type,mapped_type>
             >::type,2>
          >
  using static_patchmap =
    patchmap<key_type,mapped_type,hash,equal,comp,alloc,false>;
  
  template<class key_type,           // unordered_map has no default key_type
           class mapped_type,        // unordered_map has no default mapped_type
           class hash        = hash_functor<key_type>,
           class equal       = std::equal_to<key_type>,
           class alloc       = typename // mapped_type must not be void
             boost::container::allocator<std::pair<key_type,mapped_type>,2>,
           class comp        = typename conditional<is_injective<hash>::value,
             dummy_comp<key_type>,typename std::less<key_type>::type>::type
          >
  using unordered_map =
    patchmap<key_type,mapped_type,hash,equal,alloc,comp>;
  
  template<class key_type,           // unordered_set has no default key_type
           class hash        = hash_functor<key_type>,
           class equal       = std::equal_to<key_type>,
           class alloc       = typename
             boost::container::allocator<pair<key_type,true_type>,2>,
           class comp        = typename conditional<is_injective<hash>::value,
             dummy_comp<key_type>,typename std::less<key_type>::type>::type
          >
  using unordered_set =
    patchmap<key_type,void,hash,equal,alloc,comp>;
  
}
#endif // ORDERED_PATCH_MAP_H
