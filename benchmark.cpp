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
#ifdef PATCHMAP
#include "patchmap.hpp"
#endif
//#include "doubling_patchmap.hpp"
//#include "unordered_map.hpp"
#ifdef ABSL
#include <absl/container/flat_hash_map.h>
#endif
#ifdef KTPRIME
#include "hash_table5.hpp"
#endif
#ifdef TSL
#include "sparse_map.h"
#endif
#ifdef SPARSE_PATCHMAP
#include "sparse_patchmap.hpp"
#endif
#ifdef FLATMAP
#include "flat_hash_map.hpp"
#endif
#ifdef BYTELL
#include "bytell_hash_map.hpp"
#endif
#ifdef SPARSEPP
#include "sparsepp/spp.h"
#endif
#ifdef SPARSEMAP
#include <sparsehash/sparse_hash_map>
#endif
#ifdef DENSEMAP
#include <sparsehash/dense_hash_map>
#endif
#ifdef KHASH
#include "khash.h"
#endif
#ifdef WMAP
#include "wmath_unordered_map.hpp"
#endif
#ifdef JUDY
#include <Judy.h>
#endif
#ifdef F14
#include "folly/container/F14Map.h"
#endif
#include "wmath"
#ifdef WHASH
#include "whash.h"
#endif
#ifdef SPARSE
#include "sparse.hpp"
#endif
#ifdef ROBINMAP
#include "robin_hood.h"
#endif
#ifdef HORDI
#include "hash_set.h"
#endif
#ifdef PHAMT
#include "hash_trie.hpp"
#endif

#include <stdio.h>
#include <proc/readproc.h>

using wmath::reflect;
using wmath::rol;
using wmath::ror;

using std::bitset;
using std::cerr;
using std::cout;
using std::endl;
using std::get;
using std::max;
using std::setw;
using std::stoi;

#ifdef KHASH
KHASH_MAP_INIT_INT64(64, uint64_t)
#endif

uint64_t gen_rand(uint64_t i) {
  return wmath::clmul_mod(
      uint64_t(i*8593922412848152131ull),
      uint64_t( 16647681018011545579ull));
}

int main(int argc, char** argv){
  std::ios_base::sync_with_stdio(false);
  if (argc<2) return 1;
  const size_t N = stoi(argv[1]);
  std::random_device urand;
  std::minstd_rand mr(urand());
  double initial_memory;
  {
    struct proc_t usage;
    look_up_our_self(&usage);
    initial_memory = usage.vsize;
  }
  double base_time;
  int sand = 0;
  {
    auto start = std::clock();
    for (size_t i=0;i!=N;++i){
      sand+=gen_rand(i);
    }
    base_time = std::clock()-start;
    base_time/=N;
  }
#ifdef ABSL
  absl::flat_hash_map<uint64_t,uint64_t> test;
#endif
#ifdef KTPRIME
  emilib2::HashMap<uint64_t,uint64_t> test;
#endif
#ifdef TSL
  tsl::sparse_map<uint64_t,uint64_t> test;
#endif
#ifdef PATCHMAP
  whash::patchmap<uint64_t,uint64_t> test;
#endif
#ifdef SPARSE_PATCHMAP
  wmath::sparse_patchmap<uint64_t,uint64_t> test;
#endif
#ifdef WMATH_ROBIN_MAP
  wmath::robin_map<uint64_t,uint64_t> test;
#endif
#ifdef ROBINMAP
  robin_hood::unordered_map<uint64_t,uint64_t> test;
#endif
#ifdef SPARSEPP
  spp::sparse_hash_map<uint64_t,uint64_t> test;
#endif
#ifdef UNORDERED_MAP
  std::unordered_map<uint64_t,uint64_t> test;
#endif
#ifdef MAP
  std::map<uint64_t,uint64_t> test;
#endif
#ifdef FLATMAP
  ska::flat_hash_map<uint64_t,uint64_t> test;
#endif
#ifdef BYTELL
  ska::bytell_hash_map<uint64_t,uint64_t> test;
#endif
#ifdef SPARSEMAP
  google::sparse_hash_map<uint64_t,uint64_t> test;
#endif
#ifdef DENSEMAP
  google::dense_hash_map<uint64_t,uint64_t> test;
  test.set_empty_key(0);
  test.set_deleted_key(~uint64_t(0));
#endif
#ifdef KHASH
  khash_t(64) *h = kh_init(64);
  int ret, is_missing;
  khiter_t k;
#endif
#ifdef WMAP
  wmath::unordered_map<uint64_t,uint64_t> test;
#endif
#ifdef JUDY
  void *j_array = nullptr;
#endif
#ifdef F14
  folly::F14ValueMap<uint64_t,uint64_t> test;
#endif
#ifdef WHASH
  struct hash_table test;
#endif
#ifdef SPARSE
  wmath::sparse_map<uint64_t,uint64_t> test;
#endif
#ifdef HORDI
  hordi::hash_map<uint64_t,uint64_t> test;
#endif
#ifdef PHAMT
  hamt::hash_trie<uint64_t> test;
#endif
  std::uniform_int_distribution<size_t> distr;
  double counter = 0;
  double insert_time = 0;
  double find_time = 0;
  double delete_time = 0;
  double not_find_time = 0;
  double acc_insert = 0;
  double acc_find = 0;
  double acc_delete = 0;
  double acc_not_find = 0;
  double typical_insert_time = 0;
  double typical_delete_time = 0;
  double typical_find_time = 0;
  double typical_not_find_time = 0;
  double typical_memory = 0;
  double memory = 0;
  auto start = std::clock();
  size_t next_point = 0;
  double avg_insert_time = 0;
  double avg_delete_time = 0;
  double avg_find_time = 0;
  double avg_not_find_time = 0;
  double avg_memory = 0;
  double avg_counter = 0;
  for (uint64_t i=0;i!=N/4096;++i) {
    auto start = std::clock();
    for (uint64_t j=0;j!=4096;++j) {
      const uint64_t n = gen_rand(uint64_t(2*(i*4096+j)));
#ifdef KHASH
      k = kh_put(64, h, n, &ret);
      kh_value(h, k) = n;
#elif JUDY
      void* pvalue;
      JLI(pvalue,j_array,n);
      *reinterpret_cast<size_t*>(pvalue) = n;
#elif WHASH
      insert_into_hash_table(&n,&n,&test);
#elif PHAMT
      test.insert(n);
#else
      test[n]=n;
#endif
    }
    insert_time = (std::clock()-start-base_time);
    typical_insert_time+=(acc_insert+=insert_time)/(i+1);
    struct proc_t usage;
    look_up_our_self(&usage);
    memory = double(usage.vsize)-initial_memory;
    typical_memory+=memory/(i+1);
    mr.seed(i+7ul);
    std::uniform_int_distribution<size_t> distr(0,i);
    const size_t l0 = distr(mr);
    start = std::clock();
    for (size_t j=0;j!=4096;++j) {
      const uint64_t n = gen_rand(uint64_t(2*(i*4096+j)));
#ifdef KHASH
      k = kh_get(64, h, n);
      sand += k;
#elif JUDY
      void* pvalue;
      JLG(pvalue,j_array,n);
      sand+=*reinterpret_cast<uint64_t*>(pvalue);
#elif WHASH
      sand+=(get_from_hash_table(&n,&test)!=NULL);
#elif PHAMT
      //sand+=test.find(n).size();
      test.find(n);
#else
      sand+=test.count(n);
#endif
    }
    find_time = std::clock()-start-base_time;
    typical_find_time+=(acc_find+=find_time)/(i+1);
    const size_t l1 = distr(mr);
    start = std::clock();
    for (size_t j=0;j!=4096;++j) {
      const uint64_t n = gen_rand(uint64_t(2*(i*4096+j)));
#ifdef KHASH
      k = kh_get(64, h, n);
      sand += k;
#elif JUDY
      void* pvalue;
      JLG(pvalue,j_array,n);
      sand+=(pvalue!=NULL);
#elif WHASH
      sand+=(get_from_hash_table(&n,&test)!=NULL);
#elif PHAMT
      //sand+=test.find(n).size();
      test.find(n);
#else
      sand+=test.count(n);
#endif
    }
    not_find_time = std::clock()-start-base_time; 
    typical_not_find_time+=(acc_not_find+=not_find_time)/(i+1);
    //const size_t l2 = distr(mr);
    start = std::clock();
    for (size_t j=0;j!=4096;++j){
      const uint64_t n = gen_rand(uint64_t(2*(i*4096+j)));
#ifdef KHASH
      k = kh_get(64, h, n);
      if (k!=kh_end(h)) kh_del(64, h, k);
#elif JUDY
      int ret;
      JLD(ret,j_array,n);
#elif WHASH
      del_from_hash_table(&n,&test);
#elif PHAMT
      1;
#else
      test.erase(n);
#endif
    }
    delete_time = std::clock()-start-base_time;
    typical_delete_time+=(acc_delete+=delete_time)/(i+1);
    for (size_t j=0;j!=4096;++j){
      const uint64_t n = gen_rand(uint64_t(2*(i*4096+j)));
#ifdef KHASH
      k = kh_put(64, h, n, &ret);
      kh_value(h, k) = n;
#elif JUDY
      void* pvalue;
      JLI(pvalue,j_array,n);
      *reinterpret_cast<size_t*>(pvalue) = n;
#elif WHASH
      insert_into_hash_table(&n,&n,&test);
#elif PHAMT
      //test.insert(n);
      1;
#else
      test[n]=n;
#endif
    }
    avg_counter += 4096;
    avg_memory += memory/(i+1);
    avg_insert_time += insert_time;
    avg_delete_time += delete_time;
    avg_find_time += find_time;
    avg_not_find_time += not_find_time;
    if (i==next_point) {
      avg_counter = 1.0/avg_counter;
      avg_memory*=avg_counter;
      avg_insert_time*=avg_counter;
      avg_delete_time*=avg_counter;
      avg_find_time*=avg_counter;
      avg_not_find_time*=avg_counter;
      cerr << (i+1)*4096        << " "
           << avg_memory        << " "
           << avg_insert_time   << " "
           << avg_delete_time   << " "
           << avg_find_time     << " "
           << avg_not_find_time << endl;
      avg_counter = 0;
      avg_memory = 0;
      avg_insert_time = 0;
      avg_delete_time = 0;
      avg_find_time = 0;
      avg_not_find_time = 0;
      next_point = (next_point==next_point*17/16)?next_point+1:next_point*17/16;
    }
  }
  cout << typical_memory/N        << " "
       << typical_insert_time/N   << " "
       << typical_delete_time/N   << " "
       << typical_find_time/N     << " "
       << typical_not_find_time/N << endl;
#ifdef KHASH
  kh_destroy(64, h);
#endif
//#ifdef PATCHMAP
//  std::cerr << test.size() << " " << test.test_size() << endl;
//#endif
#ifdef JUDY
  //size_t rcword;
  //JLC(rcword,j_array,0,~size_t(0));
  //cout << rcword << endl;
#endif
#ifdef WMAP
  std::cerr << test.size() << endl;
#endif
#ifdef WHASH
  free_hash_table(&test);
#endif
  return sand;
}
