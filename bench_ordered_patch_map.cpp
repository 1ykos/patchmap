#include <bitset>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <string>
#include <unordered_map>
#include "ordered_patch_map.hpp"
#include "flat_hash_map.hpp"
#include "bytell_hash_map.hpp"
#include "sparsepp/spp.h"
#include <sparsehash/sparse_hash_map>
#include <sparsehash/dense_hash_map>
#include "khash.h"

#include <stdio.h>
#include <proc/readproc.h>

using wmath::ordered_patch_map;
using wmath::reflect;
using wmath::rol;

using std::cout;
using std::endl;
using std::bitset;
using std::setw;
using std::get;
using std::stoi;
using std::max;

//#define PATCHMAP
//#define SPARSEPP
//#define KHASH
//#define UNORDERED_MAP
//#define BYTELL
//#define FLATMAP
//#define SPARSEMAP
//#define MAP

#ifdef KHASH
KHASH_MAP_INIT_INT(32, uint32_t)
#endif

uint32_t gen_rand(uint32_t i){
  return i*3063032679ull;
}

int main(int argc, char** argv){
  std::ios_base::sync_with_stdio(false);
  if (argc<2) return 1;
  const size_t N = stoi(argv[1]);
  std::random_device urand;
  std::minstd_rand mr(urand());
  std::uniform_int_distribution<uint32_t> u32distr(1u<<30);
  double initial_memory;
  {
    struct proc_t usage;
    look_up_our_self(&usage);
    initial_memory = usage.vsize;
  }
#ifdef PATCHMAP
  ordered_patch_map<uint32_t,uint32_t> test;
#endif
#ifdef SPARSEPP
  spp::sparse_hash_map<uint32_t,uint32_t> test;
#endif
#ifdef UNORDERED_MAP
  std::unordered_map<uint32_t,uint32_t> test;
#endif
#ifdef MAP
  std::map<size_t,size_t> test;
#endif
#ifdef FLATMAP
  ska::flat_hash_map<uint32_t,uint32_t> test;
#endif
#ifdef BYTELL
  ska::bytell_hash_map<uint32_t,uint32_t> test;
#endif
#ifdef SPARSEMAP
  google::sparse_hash_map<uint32_t,uint32_t> test;
#endif
#ifdef KHASH
  khash_t(32) *h = kh_init(32);
  int ret, is_missing;
  khiter_t k;
#endif
  int sand = 0;
  std::uniform_int_distribution<size_t> distr;
  double counter = 0;
  double acc_insert = 0;
  double acc_find = 0;
  double acc_delete = 0;
  double acc_not_find = 0;
  double typical_insert_time = 0;
  double typical_delete_time = 0;
  double typical_find_time = 0;
  double typical_not_find_time = 0;
  double typical_memory = 0;
  auto start = std::clock();
  for (size_t i=0;i!=N/4096;++i) {
    auto start = std::clock();
    for (size_t j=0;j!=4096;++j) {
      const uint32_t n = gen_rand(2*(i*4096+j));
#ifdef KHASH
      k = kh_put(32, h, n, &ret);
      kh_value(h, k) = n;
#else
      test[n]=n;
#endif
    }
    typical_insert_time+=(acc_insert+=(std::clock()-start))/(i+1);
    struct proc_t usage;
    look_up_our_self(&usage);
    typical_memory+=(double(usage.vsize)-initial_memory)/(i+1);
    mr.seed(i+7ul);
    std::uniform_int_distribution<size_t> distr(0,i);
    const size_t l0 = distr(mr);
    start = std::clock();
    for (size_t j=0;j!=4096;++j) {
      const uint32_t n = gen_rand(2*(l0*4096+j));
#ifdef KHASH
      k = kh_get(32, h, n);
      sand += k;
#else
      sand+=test.count(n);
#endif
    }
    typical_find_time+=(acc_find+=(std::clock()-start))/(i+1);
    const size_t l1 = distr(mr);
    start = std::clock();
    for (size_t j=0;j!=4096;++j) {
      const uint32_t n = gen_rand(2*(l1*4096+j)+1);
#ifdef KHASH
      k = kh_get(32, h, n);
      sand += k;
#else
      sand+=test.count(n);
#endif
    }
    typical_not_find_time+=(acc_not_find+=(std::clock()-start))/(i+1);
    const size_t l2 = distr(mr);
    start = std::clock();
    for (size_t j=0;j!=4096;++j){
      const uint32_t n = gen_rand(2*(l2*4096+j));
#ifdef KHASH
      k = kh_get(32, h, n);
      if (k!=kh_end(h)) kh_del(32, h, k);
#else
      test.erase(n);
#endif
    }
    typical_delete_time+=(acc_delete+=(std::clock()-start))/(i+1);
    for (size_t j=0;j!=4096;++j){
      const uint32_t n = gen_rand(2*(l2*4096+j));
#ifdef KHASH
      k = kh_put(32, h, n, &ret);
      kh_value(h, k) = n;
#else
      test[n]=n;
#endif      
    }
  }
  cout << typical_memory/N        << " "
       << typical_insert_time/N   << " "
       << typical_delete_time/N   << " "
       << typical_find_time/N     << " "
       << typical_not_find_time/N << endl;
#ifdef KHASH
  kh_destroy(32, h);
#endif
  return sand;
}
