#include <bitset>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <unordered_map>
#include "ordered_patch_map.hpp"
//#include "doubling_patchmap.hpp"
#include "robin_map.hpp"

using wmath::ordered_patch_map;
using wmath::reflect;

using std::cout;
using std::endl;
using std::bitset;
using std::setw;
using std::get;
using std::string;
using std::vector;
using std::tuple;
using std::get;
using std::allocator_traits;

uint32_t gen_rand(uint32_t i){
  return wmath::clmul_mod(uint32_t(i*3061963241ul),uint32_t(3107070805ul));
  return wmath::rol(uint32_t(i*3063032679ull),13)*2694261069ull;
}

int main(){
  std::minstd_rand mr;
  ordered_patch_map<uint32_t,void> test;
  //static_patchmap<uint32_t,void> test(11u<<29);
  //wmath::robin_map<uint32_t,uint32_t> test;
  //std::unordered_map<uint32_t,uint32_t> test;
  cout << "sizeof(ordered_patch_map{}) = " << sizeof(test) << endl;
  for (uint32_t i=0;i!=1u<<30;++i){
    const uint32_t j = gen_rand(i);
    test[j];
    //cout << j << endl;
    if (test.count(j)==0) {
      //test.print();
      cout << "could not retrieve key that should be there (before erase)" << endl;
      cout << wmath::frac(wmath::distribute(j)) << endl;
      return 1;
    }
    test.erase(j);
    test[j];
    std::uniform_int_distribution<uint32_t> u32distr1(0,i);
    const uint32_t k = gen_rand(u32distr1(mr));
    if (test.count(k)==0) {
      //test.print();
      cout << "could not retrieve key that should be there" << endl;
      cout << wmath::frac(wmath::distribute(k)) << endl;
      return 1;
    }
    std::uniform_int_distribution<uint32_t> u32distr0(i+1,~uint32_t(0));
    const uint32_t l = gen_rand(u32distr0(mr));
    if (test.count(l)){
      cout << "found something that should not be there" << endl;
      cout << l << " " << wmath::frac(wmath::distribute(k)) << endl;
      return 1;
    }
    if (wmath::popcount(i)==1) if (test.test_size()!=test.size()){
      cout << "lost keys :( " << endl;
      return 1;
    }
    if (i==~uint32_t(0)) break;
  }
  /*
  if (!test.check_ordering()){
    //test.print();
    cout << "ordering is broken (2)" << endl;
    return 1;
  }*/
  //test.print();
  cout << "test was successfull" << endl;
  return 0;
}
