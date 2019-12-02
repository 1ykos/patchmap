#include <iostream>
#include <limits>
#include <cmath>
#include <random>
#include <bitset>
#include <iomanip>
#include <unordered_map>
#include <chrono>
#include "patchmap.hpp"

using whash::patchmap;

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

void test_uint32_t(){
  std::minstd_rand mr;
  const size_t N = 1ull<<8;//1ull<<12;
  uint32_t * keys   = new uint32_t[N];
  for (size_t i=0;i!=N;++i){
    keys[i]   = i;
  }
  std::random_shuffle(keys,keys+N);
  cout << "initialized key and value test data" << endl;
  patchmap<uint32_t,uint32_t> test;
  cout << sizeof(test) << endl;
  for (size_t i=0;i!=N;++i){
    if ((test.size()!=i)||(test.test_size()!=i)){
      cout << "test failed, size does not match" << endl;
      cout << test.size() << " " << i << " " << test.test_size() << endl; 
      exit(1);
    }
    //cout << test.size() << " " << i << " " << test.test_size() << endl;
    if (!test.check_ordering()){
      cout << "ordering of ordered_patch map is broken" << endl;
      exit(1);
    }
    test[keys[i]]=keys[i];
  }
  if (test.size()==N) cout << "inserted " << N << " key-value pairs" << endl;
  else {
    cout << "test failed, not all key-value pairs were inserted" << endl;
    cout << N << " " << test.size() << endl;
    exit(1);
  }
  for (size_t i=0;i!=N;++i){
    if (test.at(keys[i]) != keys[i]){
      cout << "test failed, inserted key value pair does not match 1" << endl;
      exit(1);
    }
  }
  for (auto it=test.begin();it!=test.end();++it){
    if (it->first!=it->second){
      cout << " >" << it->first << "< >" << it->second << "< " << endl;
      cout << "test failed, inserted key value pair does not match 2" << endl;
      exit(1);
    }
  }
  cout << "all inserted key-value pairs have been found again" << endl;
  for (size_t i=0;i<=N;i+=3){
    test.erase(keys[i]);
    /*if (!test.check_ordering()){
      cout << "ordering of ordered_patch map is broken" << endl;
      exit(1);
    }*/
  }
  cout << "removed every 3rd key-value pair, size = " << test.size() << endl;
  for (size_t i=0;i!=N;++i){
    if (i%3==0){
      if (test.count(keys[i])){
        cout << "test failed, erased key is still in map" << endl;
        exit(1);
      }
    } else {
      if (test.count(keys[i])==0){
        cout << "test failed, this key should still exist" << endl;
        exit(1);
      }
      if (test.at(keys[i]) != keys[i]){
        cout << "test failed, inserted key value pair does not"
             << "match after erase" << endl;
        exit(1);
      }
    }
  }
  for (uint32_t i=N;i!=2*N;++i){
    if (test.count(i)){
      cout << "test failed, this key should not be there" << endl;
      exit(1);
    }
  }
  cout << "no key that should not be there has been found" << endl;
  for (auto it=test.begin();it!=test.end();++it) {
    ++it->second;
  }
  cout << "using iterators on uint32_t seems to work too" << endl;
  cout << "squeezing patchmap very hard" << endl;
  test.resize(test.size());
  if (!test.check_ordering()){
    cout << "ordering of ordered_patch map is broken" << endl;
    exit(1);
  }
  cout << test.size() << " " << test.test_size() << endl;
  cout << "ordering is valid until here" << endl;
  //while(test.size()){
  //  cout << test.size() << endl;
  //  test.erase(test.begin());
  //}
  /*for (auto it=test.begin();it!=test.end();it=test.begin()) {
    test.erase(it->first);
  }*/
  for (auto it=test.begin();it!=test.end();it=test.erase(it)) test.print();
  if (test.size()!=0) {
    cout << "test failed, everything should have been erased, but was not"
         << endl;
    cout << test.size() << " " << test.test_size() << endl;
    exit(1);
  }
  delete[] keys;
  cout << "test_uint32_t exits successfully" << endl;
}

void test_string(){
  patchmap<string,string> test;
  vector<tuple<string,string>> testvalues
  {
    {"hallo","ollah"},
    {"miau" ,"uaim"},
    {"a"    ,"a"},
    {"b"    ,"b"},
    {"test" ,"tset"},
    {"12345","54321"},
    {"abcd" ,"abcd"},
  };
  string a("a");
  test[a]=a;
  cout << test.at("a") << endl;
  return;
  for (auto it0=testvalues.begin();it0!=testvalues.end();++it0){
    test[get<0>(*it0)]=get<1>(*it0);
    for (auto it1=testvalues.begin();it1!=it0;++it1){
      if (test.at(get<0>(*it1))!=get<1>(*it1)){
        cout << "test failed, found value is not what it is supposed to be"
             << endl;
        exit(1);
      }
    }
  }
  const auto other = test;
  for (auto it0=testvalues.begin();it0!=testvalues.end();++it0){
    if (other.at(get<0>(*it0))!=get<1>(*it0)){
      cout << "test failed, found value is not what it is supposed to be"
           << endl;
      exit(1);
    };
  }
  for (auto it=test.begin();it!=test.end();++it) it->second=it->first;
  for (auto it=test.begin();it!=test.end();++it) {
    if (it->second!=it->first) {
      cout << "test failed, iterator assigment for string does not work"
           << endl;
      exit(1);
    }
  }
  cout << "test_string() was successfully executed" << endl;
}

int main(){
  /*std::allocator<std::pair<string,string>> allocator;
  std::pair<string,string>* a =
    allocator_traits<decltype(allocator)>::allocate(allocator,777);
  a[77].first = "test";
  allocator_traits<decltype(allocator)>::deallocate(allocator,a,777);
  return 0;*/
  test_uint32_t();
  test_string();
  cout << "all tests were executed successfully" << endl;
  return 0;
}
