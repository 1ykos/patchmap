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
#include <typeinfo>
#include <exception>
#include <memory>
#include "wmath_forward.hpp"
#include "wmath_bits.hpp"
#include "wmath_hash.hpp"

namespace wmath{
  using std::allocator_traits;
  template <typename T>
  typename std::enable_if<std::is_unsigned<T>::value,T>::type
  constexpr distribute(const T& a); // mix the hash value good, clmul_circ
                                    // with odious integer is suitable

  uint8_t  const inline distribute(const uint8_t& a){
    return clmul_circ(a,uint8_t(0b01000101u));
    return (a+111)*97;
  } 
  uint16_t const inline distribute(const uint16_t& a){
    return clmul_mod(a,uint16_t(0b01000101'10100101u));
    return (a+36690)*43581;
  }
  uint32_t const inline distribute(const uint32_t& a){
    return rol(a*3107070805ul,11)*3061963241ul;
    return clmul_mod(a,uint32_t(3107070805ul));
  }
  uint64_t const inline distribute(const uint64_t& a){
    return rol(a*16123805160827025777ull,21)*16123805160827025777ull;
    return clmul_mod(a,uint64_t(16123805160827025777ull));
  }

  template<typename T>
  struct dummy_comp{ // dummy comparator for when we don't need a comparator
    constexpr bool operator()(const T&,const T&) const {return false;}
  };

  template<class key_type    = int,  // int is the default, why not
           class mapped_type = int,  // int is the default, why not
           class hash        = hash_functor<key_type>,
           class equal       = std::equal_to<key_type>,
           class comp        = typename conditional<is_injective<hash>::value,
                                                    dummy_comp<key_type>,
                                                    std::less<key_type>>::type,
           class alloc       = std::allocator<std::pair<key_type,mapped_type>>
          >
  class ordered_patch_map{
    public:
      typedef alloc allocator_type;
      typedef typename alloc::value_type value_type;
      typedef typename alloc::reference reference;
      typedef typename alloc::const_reference const_reference;
      typedef typename alloc::difference_type difference_type;
      typedef typename alloc::size_type size_type;
      typedef typename std::result_of<hash(key_type)>::type hash_type;
    private:
      size_type num_data;
      size_type datasize;
      size_type inversed;
      size_type nextsize;
      size_type masksize;
      pair<key_type,mapped_type> * data;
      size_type * mask;
      allocator_type allocator;
      comp comparator;
      equal equator;
      hash hasher;
      using uphold_iterator_validity = true_type;
      size_type inline map(const hash_type& h) const {
        constexpr size_type s =
          (digits<hash_type>()>digits<size_type>())?
          (h>>(digits<hash_type>()-digits<size_type>())):0;
        const auto l = long_mul(h,datasize);
        return get<0>(l);
      }
      hash_type inline order(const key_type& k) const {
        return distribute(hasher(k));
      }
      bool inline is_less(const key_type& a,const key_type& b) const {
        if constexpr (is_injective<hash>::value)
          assert(equator(a,b)==(order(a)==order(b)));
        if (order(a)<order(b)) return true;
        if (order(a)>order(b)) return false;
        if constexpr (is_injective<hash>::value) return false;
        return comparator(a,b);
      }
      bool inline is_more(const key_type& a,const key_type& b) const {
        if constexpr (is_injective<hash>::value)
          assert(equator(a,b)==(order(a)==order(b)));
        if (order(a)>order(b)) return true;
        if (order(a)<order(b)) return false;
        if constexpr (is_injective<hash>::value) return false;
        return !((comparator(a,b))||(equator(a,b)));
      }
      bool inline is_set(const size_type& n) const {
        /*const size_type i = n/32;
        const size_type j = n%32;
        if (*(reinterpret_cast<uint32_t*>(mask)+i)&(uint32_t(1)<<(31-j)))
          return true;
        return false;*/
        const size_type i = n/digits<size_type>();
        //if (!mask[i]) return false;
        const size_type j = n%digits<size_type>();
        //cout << datasize << " " << masksize << " " << i << " " << n << endl;
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
      hash_type inline index(const size_type& i) const {
        assert(i<datasize);
        if (is_set(i)) return order(data[i].first);
        return i*inversed;
      }
      bool inline index_index_is_less(const size_type& i,const size_type& j)
        const {
        assert(i<datasize);
        assert(j<datasize);
        if (is_set(i)&&is_set(j)) return is_less(data[i].first,data[j].first);
        if (is_set(i)) return order(data[i].first)<hash_type(j*inversed);
        if (is_set(j)) return hash_type(i*inversed)<order(data[j].first);
        return i<j;
      }
      bool inline index_key_is_less(const size_type& i,const key_type& k) const{
        if (is_set(i)) return is_less(data[i].first,k);
        //cout << i*inversed << " " << order(k) << endl;
        return hash_type(i*inversed)<order(k);
      }
      bool inline index_key_is_more(const size_type& i,const key_type& k) const{
        if (is_set(i)) return is_more(data[i].first,k);
        return hash_type(i*inversed)>order(k);
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
          if (i>datasize) return ~size_type(0);
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
      size_type const inline insert_node(
          const key_type& key,
          const size_type& hint,
          const hash_type& ok
          ){
        assert(hint<datasize);
        size_type i = hint;
        /*while(i+1<datasize){
          if ((i+1)*inversed<ok) ++i;
          else break;
        }
        while(i!=0){
          if ((i-1)*inversed>ok) --i;
          else break;
        }*/
        assert((i+1)*inversed>ok||i==datasize-1);
        assert((i-1)*inversed<=ok||i==0);
        if (ok < index(i)) {
          const size_type  j = search_free_dec(i);
          if (j>=datasize) i = search_free_inc(i);
          else i = j;
        }else{
          const size_type  j = search_free_inc(i);
          if (j>=datasize) i = search_free_dec(i);
          else i = j;
        }
        assert(i<datasize);
        assert(!is_set(i));
        //cout << i << " " << key << endl;
        allocator_traits<alloc>::construct(allocator,data+i,key,mapped_type());
        //data[i].first=key;
        set(i);
        ++num_data;
        //cout << "insert_node at " << i << endl;
        //const size_type j = i;
        while(true){
          if (i==0) break;
          //cout << i-1 << " " << i << endl;
          //cout << index(i-1) << " " << index(i) << endl;
          if (!is_set(i-1)) break;
          if (is_less(data[i].first,data[i-1].first))
            swap(data[i],data[i-1]);
          else break;
          --i;
        }
        while(true){
          if (i+1>=datasize) break;
          //cout << i << " " << i+1 << endl;
          //cout << index(i) << " " << index(i+1) << endl;
          if (!is_set(i+1)) break;
          if (is_more(data[i].first,data[i+1].first))
            swap(data[i],data[i+1]);
          else break;
          ++i;
        }
        //cout << int(i)-int(j) << " " << int(j)-int(hint) << endl;
        //assert(check_ordering());
        return i;
      }
      size_type inline insert_node(
          const key_type& key,
          const size_type& hint){
        const hash_type ok = order(key);
        size_t i = hint;
        // TODO use hint better 
        return insert_node(key,i,ok);
      }
      // TODO move version insert_node(K&& key)
      size_type inline insert_node(const key_type& key){
        const hash_type ok = order(key);
        const size_type hint = map(ok);
        assert(hint<datasize);
        return insert_node(key,hint,ok);
      }
      size_type inline find_node_binary(
          const key_type& key,
          const size_type& lo, // inclusive bounds
          const size_type& hi  // inclusive bounds
          ) const {
        //cout << "find_node_binary(" << lo << ", " << hi << ")"<< endl;  
        //if (lo>hi) return ~size_type(0);
        assert(lo<datasize);
        assert(hi<datasize);
        assert(lo<=hi);
        if (!is_set_any(lo,hi)) return ~size_type(0); 
        //if (hi-lo<digits<size_t>()) return find_node_linear(key,lo,hi);
        //if (index_key_is_less(hi,key)) return ~size_type(0);
        //if (index_key_is_more(lo,key)) return ~size_type(0);
        const size_type  mi = (hi+lo)/2;
        if (is_set(mi)) if (data[mi].first==key) return mi;
        //if (hi==lo) return ~size_type(0);
        //if (index_key_is_less(mi,key)){
        //  return find_node_binary(key,mi<hi?mi+1:mi,hi);
        //} else {
        //  return find_node_binary(key,lo,mi>lo?mi-1:mi);
        //}
        if (index_key_is_less(mi,key)){
          if (mi<hi) return find_node_binary(key,mi+1,hi);
          else return ~size_type(0);
        } else {
          if (mi>lo) return find_node_binary(key,lo,mi-1);
          else return ~size_type(0);
        }
      }
      size_type inline find_node_interpol(
          const key_type& k,
          const hash_type& ok,
          const size_type& lo,
          const size_type& hi
          ) const {
        //cout << "find_node_interpol" << endl;
        //if (lo>hi) return ~size_type(0);
        assert(lo<=hi||datasize==0);
        if (hi-lo<17) return find_node_binary(k,lo,hi);
        const size_type l = log2(hi-lo);
        const hash_type ihi       = index(hi);
        const hash_type ilo       = index(lo);
        //cout << ilo << " " << ok << " " << ihi << endl;
        if (ilo>ok) return ~size_type(0);
        if (ihi<ok) return ~size_type(0);
        assert(ilo<=ok);
        assert(ihi>=ok);
        if (((ihi-ilo)>>l)==0) return find_node_binary(k,lo,hi);
        size_type mi = lo+(size_type((ok-ilo)>>l)*(hi-lo))
                          /size_type((ihi-ilo)>>l);
        if (mi==lo) ++mi;
        else if (mi==hi) --mi;
        const hash_type imi = index(mi);
        assert(mi<=hi);
        assert(mi>=lo);
        assert(imi<=ihi);
        assert(imi>=ilo);
        if (index_key_is_less(mi,k)) return find_node_interpol(k,ok,mi,hi);
        else                         return find_node_interpol(k,ok,lo,mi);
      }
      size_type inline find_node(
          const key_type& k,
          const hash_type& ok,
          const size_type& hint)
        const {
        //cout << "find_node " << k << " " << hint << endl;
        size_type i = hint;
        if (i>=datasize) return ~size_type(0);
        size_type oi;
        if (!is_set(i)){
          return ~size_type(0);
        } else {
          if (data[i].first==k) return i;
          oi = order(data[i].first);
        }
        size_type lo=0;
        size_type hi=datasize-1;
        assert((i+1)*inversed>ok||i==datasize-1);
        assert((i-1)*inversed<=ok||i==0);
        //return find_node_interpol(k,lo,hi);
        //size_type d = hint;
        for (size_type j=0;j!=digits<size_type>();++j){
          size_type d = map(oi);
          d = d>hint?d-hint:hint-d;
          d+=1;
          if (oi<ok){
            lo = (i<hi)?i+1:i;
            if (i+d>hi) i=hi;//return find_node_interpol(k,ok,lo,hi);
            else i+=d;
          } else if (oi>ok) {
            hi = (i>lo)?i-1:i;
            if (i<lo+d) i=lo;//return find_node_interpol(k,ok,lo,hi);
            else i-=d; 
          } else if constexpr (!is_injective<hash>::value){
            if (index_key_is_less(i,k)){
              lo = (i<hi)?i+1:i;
              if (i<hi) ++i;
              else return ~size_type(0);
            }else{
              hi = (i>lo)?i-1:i;
              if (i>lo) --j;
              else return ~size_type(0);
            }
          } else {
            assert(oi!=ok);
          }
          if (is_set(i)){
            if (data[i].first==k) return i;
            oi = order(data[i].first);
          } else {
            oi = i*inversed;
          }
          if (hi==lo) return ~size_type(0);
        }
        return find_node_interpol(k,ok,lo,hi);
      }
      size_type const inline find_node(const key_type& k)
      const {
        const hash_type ok = order(k);
        const size_type hint = map(ok);
        return find_node(k,ok,hint);
      }
      template<typename map_type>
      typename conditional<is_const<map_type>::value,
                           const mapped_type&,
                           mapped_type&>::type
      static inline const_noconst_at(map_type& hashmap,const key_type& k) {
        size_type i = hashmap.find_node(k);
        if (i<hashmap.datasize){
          assert(hashmap.is_set(i));
          return hashmap.data[i].first;
        } else throw std::out_of_range(
            std::string(typeid(hashmap).name())
            +".const_noconst_at("+typeid(k).name()+" k)"
            +"key not found, array index "
            +to_string(i)+" out of bounds"
           );
      }
    public:
      // constructor
      ordered_patch_map(const size_type& datasize = 0)
        :datasize(datasize)
      {
        num_data = 0;
        nextsize = (datasize/digits<size_type>()*3+2)/2*digits<size_type>();
        inversed = inverse(datasize-1);
        data     = allocator_traits<alloc>::allocate(allocator,datasize);
        masksize = (datasize+digits<size_type>()-1)/digits<size_type>();
        mask = new size_type[masksize]();
      }
      ~ordered_patch_map(){                                  // destructor
        delete[] mask;
        allocator_traits<alloc>::deallocate(allocator,data,datasize);
        //delete[] data;
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
      template<
        class key_type_other,
        class mapped_type_other,
        class hash_other,
        class equal_other,
        class comp_other,
        class alloc_other
              >
      inline ordered_patch_map& operator=                   // copy assignment
        (const ordered_patch_map<
           key_type_other,
           mapped_type_other,
           hash_other,
           equal_other,
           comp_other,
           alloc_other
         >& other)
      {
        typedef ordered_patch_map<
           key_type_other,
           mapped_type_other,
           hash_other,
           equal_other,
           comp_other,
           alloc_other
         > other_type;
        delete[] mask;
        allocator_traits<alloc>::deallocate(allocator,data,datasize);
        num_data = other.num_data;
        datasize = other.datasize;
        inversed = other.inversed;
        nextsize = other.nextsize;
        masksize = other.masksize;
        mask = new size_type[masksize]();
        data = allocator_traits<alloc>::allocate(allocator,datasize);
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
      ordered_patch_map(const ordered_patch_map& other){
        num_data = other.num_data;
        datasize = other.datasize;
        inversed = other.inversed;
        nextsize = other.nextsize;
        masksize = other.masksize;
        mask = new size_type[masksize]();
        data = allocator_traits<alloc>::allocate(allocator,datasize);
        memcpy(reinterpret_cast<void*>(mask),
               reinterpret_cast<void*>(other.mask),
               masksize*sizeof(size_t));
          if constexpr (is_trivially_copyable<value_type>::value)
            memcpy(reinterpret_cast<void*>(data),
                   reinterpret_cast<void*>(other.data),
                   datasize*sizeof(value_type));
        else for (size_type i=0;i!=datasize;++i) data[i]=other.data[i];
      }
      inline ordered_patch_map& operator=                   // copy assignment
        (const ordered_patch_map& other)
      {
        return *this = ordered_patch_map(other);
      }
      inline ordered_patch_map& operator=                   // move assignment
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
      size_type erase(const key_type& k,const size_type& hint){
        size_type i = find_node(k,hint);
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
        //cout << "unset position " << i << endl;
        --num_data;
        //cout << num_data << endl;
        assert(num_data<datasize);
        return 1;
      }
      size_type erase(const key_type& k){
        return erase(k,map(order(k)));
      }
      void inline clear(){
        for (size_type i=0;i!=masksize;++i) mask[i]=0;
        num_data=0;
      }
      void const resize(const size_type& n){
        //cout << "resizing from " << datasize << " to " << n << endl;
        //cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        if (n<num_data) return;
        value_type * olddata = allocator_traits<alloc>::allocate(allocator,n);
        //value_type * olddata = new value_type[n];
        masksize = (n+digits<size_type>()-1)/digits<size_type>();
        size_type * oldmask = new size_type[masksize]();
        swap(olddata,data);
        swap(oldmask,mask);
        const size_type olddatasize = datasize;
        datasize = n;
        if (datasize<257) nextsize=2*datasize;
        else nextsize = datasize+olddatasize;
        //nextsize = datasize+olddatasize;
        if (nextsize==datasize) nextsize+=datasize;
        num_data = 0;
        //const size_type oldinversed = inversed;
        inversed = inverse(datasize-1);
        //size_type j = 0;
        const size_type c = (datasize+olddatasize/2)/olddatasize;
        for (size_type i=0;i!=olddatasize;++i){
          const size_type k = i/digits<size_type>();
          const size_type l = i%digits<size_type>();
          if (oldmask[k]&(size_type(1)<<(digits<size_type>()-l-1))){
            //cout << j << " " ;
            //j=insert_node(olddata[i].first);//,j+c<datasize?j+c:j);
            //j=insert_node(olddata[i].first,j+c<datasize?j+c:j);
            //cout << j << endl;
            //const size_type hint = map(i*oldinversed);
            //data[j].second=olddata[i].second;
            data[insert_node(olddata[i].first)].second=olddata[i].second;
            //operator[](olddata[i].first)=olddata[i].second;
            //
          }
        }
        assert(check_ordering());
        assert(test_size()==num_data);
        //cout << "test" << endl;
        allocator_traits<alloc>::deallocate(allocator,olddata,olddatasize);
        //delete[] olddata;
        delete[] oldmask;
      }
      size_type inline size() const { return num_data; }
      size_type const test_size() const {
        size_type test = 0;
        for (size_type i=0;i!=datasize;++i) test += is_set(i);
        return test;
      }
      void test_chunks() const {
        for (size_type i=0;i!=masksize;++i){
          cout << popcount(mask[i]) << endl;
        }
      }
      bool check_ordering() const {
        bool ordered = true;
        for (size_type i=0,j=1;j<datasize;(++i,++j)){
          if (index_index_is_less(i,j)) continue;
          cout << is_set(i) << " " << is_set(j) << endl;
          cout << i << " " << j << endl;
          cout << index(i) << " " << index(j) << endl;
          cout << double(index(i))/pow(2.0,64.0) << " "
               << double(index(j))/pow(2.0,64.0) << endl;
          ordered = false;
        }
        return ordered;
      }
      void inline enshure_size(){
      //if ((num_data+2)*4>=datasize*3) resize(nextsize); // 0.75
      //if (num_data*9>=7*datasize) resize(nextsize);   // 0.7777777777777778
      //if (num_data*5>=4*datasize) resize(nextsize);   // 0.8
      //if (num_data*6>=5*datasize) resize(nextsize);   // 0.8333333333333334
      //if ((num_data+2)*8>=datasize*7) resize(nextsize); // 0.875
      //if ((num_data+2)*16>=datasize*15) resize(nextsize); // 0.9375
      //if ((num_data+2)*32>=datasize*31) resize(nextsize); // 0.96875
        if ((num_data+2)*64>=datasize*63) resize(nextsize); // 0.984375
      }
      mapped_type& operator[](const key_type& k){
        const size_type i = find_node(k);
        if (i<datasize) return data[i].second;
        enshure_size();
        return data[insert_node(k)].second;
      }
      const mapped_type& operator[](const key_type& k) const {
        const size_type i = find_node(k);
        assert(i<datasize);
        return data[i].second; // this is only valid if key exists!
      }
      mapped_type& at(const key_type& k){
        return const_noconst_at(*this,k);
      }
      const mapped_type& at(const key_type& k) const {
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
      template<class key_type_other,
               class mapped_type_other,
               class hash_other,
               class equal_other,
               class comp_other,
               class alloc_other
              >
      bool operator==(
          const ordered_patch_map<
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
            if (count(it->first)) if (at(it->first)==it->second) continue;
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
          const ordered_patch_map<
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
        friend class ordered_patch_map;
        public:
          size_type hint;
          key_type key;
          typename conditional<is_const,
                               const ordered_patch_map*,
                               ordered_patch_map*
                              >::type map;
        private:
          void inline update_hint(){
            if constexpr (!uphold_iterator_validity::value) return;
            if (hint<map->datasize) if (map->data[hint].first==key) return;
            hint = map->find_node(key,hint);
            if (hint>=map->datasize) hint = ~size_type(0);
          }
          void inline unsafe_increment(){ // assuming hint is valid
            //cout << "unsafe_increment() " << hint << " " << key << endl;
            if (++hint>=map->datasize){
              //cout << "test1" << endl;
              //cout << "becoming an end()" << endl;
              hint=~size_type(0);
              return;
            }
            while(true){
              //cout << "test2" << endl;
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
              //cout << hint << " " << s << endl;
              if (hint>=map->datasize){
                //cout << "test3" << endl;
                //cout << "becoming an end()" << endl;
                hint=~size_type(0);
                return;
              }
            }
            //cout << "test4" << endl;
            //cout << "new hint=" << hint << endl;
            key = map->data[hint].first;
            //cout << "new key=" << key << endl;
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
          typedef typename
            conditional<is_const,
                        const typename alloc::reference,
                              typename alloc::reference
                       >::type
            reference;
          typedef typename
            conditional<is_const,
                        const typename alloc::pointer,
                              typename alloc::pointer
                       >::type
            pointer;
          typedef std::bidirectional_iterator_tag iterator_category;
          const_noconst_iterator(){
            //cout << "constructor 0" << endl;
          }
          const_noconst_iterator(
            const size_t& hint,
            typename conditional<is_const,
                                 const ordered_patch_map*,
                                       ordered_patch_map*
                                >::type map)
            :hint(hint),key(key_type{}),map(map){
            //cout << "constructor 1 " << hint << endl;
          }
          const_noconst_iterator(
            const size_t& hint,
            const key_type& key,
            typename conditional<is_const,
                                 const ordered_patch_map*,
                                       ordered_patch_map*
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
            //cout << "comparing " << hint << " " << key << " with "
            //     << o.hint << " " << key << endl;
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
            if ((o.hint<o.mpa->datasize)){
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
            if ((o.hint<o.mpa->datasize)){
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
            if ((o.hint<o.mpa->datasize)){
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
            if ((o.hint<o.mpa->datasize)){
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
            //cout << "operator++()" << endl;
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
            return map->data[hint];
          }
          pointer operator->() {
            update_hint();
            return &(map->data[hint]);
          }
          reference operator*() const {
            size_type i;
            if (hint>=map->datasize){
              i = map->find_node(key);
            } else if (map->data[hint]!=key){
              i = map->find_node(key,hint);
            } else {
              i = hint;
            }
            return map->data[i];
          }
          pointer operator->() const {
            size_type i;
            if (hint>=map->datasize){
              i = map->find_node(key);
            } else if (map->data[hint]!=key){
              i = map->find_node(key,hint);
            } else {
              i = hint;
            }
            return &(map->data[i]);
          }
    };
    typedef const_noconst_iterator<false> iterator;
    typedef const_noconst_iterator<true>  const_iterator;    
    iterator begin(){
      //cout << "begin()" << endl;
      const size_type i = find_first();
      //cout << "this should call constructor 2" << endl;
      return iterator(i,data[i].first,this);
    }
    const_iterator begin() const {
      //cout << "begin()" << endl;
      const size_type i = find_first();
      //cout << "this should call constructor 2" << endl;
      return const_iterator(i,data[i].first,this);
    }
    const_iterator cbegin() const {
      //cout << "cbegin()" << endl;
      const size_type i = find_first();
      //cout << "this should call constructor 2" << endl;
      return const_iterator(i,data[i].first,this);
    }
    iterator end() {
      //cout << "end()" << endl;
      const size_type i = find_first();
      //cout << "this should call constructor 1" << endl;
      return iterator(~size_type(0),this);
    }
    const_iterator end() const {
      //cout << "end()" << endl;
      //cout << "this should call constructor 2" << endl;
      return const_iterator(~size_type(0),this);
    }
    const_iterator cend() const {
      //cout << "cend()" << endl;
      //cout << "this should call constructor 2" << endl;
      return const_iterator(~size_type(0),this);
    }
    // void swap(unordered_patch_map&); // TODO
    size_type max_size()         const{return numeric_limits<size_type>::max();}
    bool empty()                 const{return (num_data==0);}
    size_type bucket_count()     const{return datasize;}
    size_type max_bucket_count() const{return numeric_limits<size_type>::max();}
    void rehash(const size_type& n) { if (n>=size()) resize(n); }
    void reserve(const size_type& n){ if (3*n>=2*(size()+1)) resize(n*3/2); }
    pair<iterator,bool> insert ( const value_type& val ){
      const size_type i = find(val.first);
      if (i<datasize) return {iterator(i,val.first,this),false};
      enshure_size();
      const size_type j = insert_node(val.first);
      data[j].second=val.second;
      return {data[j],true};
    }
    template <class P>
    pair<iterator,bool> insert ( P&& val ){
      const size_type i = find(val.first);
      if (i<datasize) return {iterator(i,val.first,this),false};
      enshure_size();
      const size_type j = insert_node(val.first);
      swap(data[j].second,val.second);
      return {data[j],true};
    }
    iterator insert ( const_iterator hint, const value_type& val ){
      const size_type i = find(val.first,hint.hint);
      if (i<datasize) return {iterator(i,val.first,this),false};
      enshure_size();
      const size_type j = insert_node(val.first);
      data[j].second=val.second;
      return {data[j],true};
    }
    template <class P>
    iterator insert ( const_iterator hint, P&& val ){
      const size_type i = find(val.first,hint.hint);
      if (i<datasize) return {iterator(i,val.first,this),false};
      enshure_size();
      const size_type j = insert_node(val.first,hint.hint);
      swap(data[j].second,val.second);
      return {data[j],true};
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
    float max_load_factor() const noexcept{
      return 1;
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

    // void max_load_factor(float z);
  }; 

  /* TODO
  template<class K,
           class T,
           class hash=hash_functor<K>,
           class equal = std::equal_to<K>,
           class comp = std::less<K>,
           class A = std::allocator<std::pair<K,T>>
          >
  void swap(ordered_patch_map<K,K,hash,equal,comp,A>&,
            ordered_patch_map<K,K,hash,equal,comp,A>&);
  */
  
}
#endif // ORDERED_PATCH_MAP_H
