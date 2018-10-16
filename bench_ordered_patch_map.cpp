#include <iostream>
#include <limits>
#include <cmath>
#include <random>
#include <bitset>
#include <iomanip>
#include <unordered_map>
#include <chrono>
#include "ordered_patch_map.hpp"

using wmath::ordered_patch_map;
using wmath::reflect;

using std::cout;
using std::endl;
using std::bitset;
using std::setw;
using std::get;

int main(){
  std::random_device urand;
  std::minstd_rand mr(urand());
  std::uniform_int_distribution<uint32_t> u32distr;
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
  ordered_patch_map<size_t,size_t> test;
  //ordered_patch_map<size_t,size_t,wmath::insecure_hash_functor<size_t>> test(32);
  //std::unordered_map<size_t,size_t> test;
  std::uniform_int_distribution<size_t> distr;
  //size_t * precompute = new size_t[67108864ul];
  //size_t * precompute2= new size_t[67108864ul];
  //for (size_t i=0;i!=67108864ull;++i) precompute[i]=distr(mr);
  //std::shuffle(precompute2,precompute2+67108864ul,mr);
  size_t * cache0 = new size_t[1024];
  size_t * cache1 = new size_t[1024];
  for (size_t i=0;i!=67108864ul;i+=1024){
    /*for (size_t j=0;j!=1024;++j){
      cache1[j]=cache0[j]=i+j;
    }*/
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t j=0;j!=1024;++j){
      const size_t r = i*i*j; // cache[j] // reverse(i+j);// distr(mr);
      test[r]=r;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    cout << i << " " << (finish-start).count() << " ";
    start = finish;
    size_t n = 0;
    for (size_t j=0;j!=1024;++j){
      const size_t r = i*i*j;//i+j+10000012;// cache1[j];
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
    cout << (finish-start).count() << " " << n << endl;
    start = finish;
  }
  return 0;
}
