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
using std::string;
using std::vector;
using std::tuple;
using std::get;
using std::allocator_traits;


int main(){
  const uint32_t N = 1u<<24; // fails when > 2¹⁷ bc. inversed breaks
  ordered_patch_map<uint32_t,uint32_t> test;
  cout << "sizeof(ordered_patch_map{}) = " << sizeof(test) << endl;
  for (uint32_t i=0;i!=N;++i){
    const uint32_t j = i*7;
    test[j]=i;
    const uint32_t k = (2*i/3)*7;
    if (test.count(k)==0) {
      //test.print();
      cout << "could not retrieve key that should be there" << endl;
      cout << wmath::frac(wmath::distribute(k)) << endl;
      return 1;
    }
    const uint32_t l = uint32_t(0)-1-i;
    if (test.count(l)){
      cout << "found something that should not be there" << endl;
      cout << l << " " << wmath::frac(wmath::distribute(k)) << endl;
      return 1;
    }
  }
  if (!test.check_ordering()){
    test.print();
    cout << "ordering is broken (2)" << endl;
    return 1;
  }
  //test.print();
  cout << "test was successfull" << endl;
  return 0;
}
