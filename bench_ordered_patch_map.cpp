#include <iostream>
#include <limits>
#include <cmath>
#include <random>
#include <bitset>
#include <iomanip>
#include <unordered_map>
#include <chrono>
#include "ordered_patch_map.hpp"
#include "sparsepp/spp.h"

using wmath::ordered_patch_map;
using wmath::reflect;

using std::cout;
using std::endl;
using std::bitset;
using std::setw;
using std::get;

int main(){
  std::ios_base::sync_with_stdio(false);
  std::random_device urand;
  std::ranlux48 mr(urand());
  std::uniform_int_distribution<uint32_t> u32distr(1u<<27);
  /*for (size_t i=0;i!=256;++i){
    uint8_t a[256]{};
    for (size_t j=0;j!=256;++j){
      const uint8_t m = wmath::circmul(uint8_t(j),uint8_t(j));
      const uint8_t n = (j+1)*j;
      ++a[uint8_t(n+m)];
      //cout << int(uint8_t(n+m)) << endl;
      //if (n!=m){//cout << int(n) << " " << int(m) << endl;
        cout << i << " " <<  j << " = " << i*j << endl;
        cout << uint(get<0>(r)) << " " << (i*j)%256 << endl
             << uint(get<1>(r)) << " " << (i*j)/256 << endl;
      //}
    }
    size_t n =0;
    for (size_t j=0;j!=256;++j) n+=(a[j]>0);
    cout << "::" << int(i) << " " << int(n) << endl;
  }
  return 0;*/
  ordered_patch_map<uint32_t,uint32_t> test;
  //spp::sparse_hash_map<uint32_t,uint32_t> test;
  //ordered_patch_map<size_t,size_t,wmath::insecure_hash_functor<size_t>> test(32);
  //std::unordered_map<size_t,size_t> test;
  std::uniform_int_distribution<size_t> distr;
  uint32_t * precompute0 = new uint32_t[67108864ul];
  uint32_t * precompute1 = new uint32_t[67108864ul];
  for (size_t i=0;i!=67108864ull;++i) precompute0[i]=precompute1[i]=u32distr(mr);
  std::shuffle(precompute1,precompute1+67108864ul,mr);
  uint32_t * cache0 = new uint32_t[256];
  uint32_t * cache1 = new uint32_t[256];
  for (size_t i=0;i!=67108864ul;i+=256){
    for (size_t j=0;j!=256;++j){
      cache0[j]=i+j;//precompute0[i+j];
      cache1[j]=i+j;//precompute1[i+j];
    }
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t j=0;j!=256;++j){
      const uint32_t r = cache0[j]; // reverse(i+j);// distr(mr);
      test[r]=r;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    cout << i << " " << (finish-start).count() << " ";
    start = finish;
    size_t n = 0;
    for (size_t j=0;j!=256;++j){
      const uint32_t r = cache1[j];
      /*if (r%3==0){
        if (!test.erase(r)){
          cout << "map broken (element not found)" << endl;
          return 1;
        }
      }*/
      n+=test.count(r);
    }
    finish = std::chrono::high_resolution_clock::now();
    //const double a = test.average_offset();
    cout << (finish-start).count() << " " << test.size() << " "
         << test.load_factor() << " " << n << endl;
    start = finish;
  }
  return 0;
}
