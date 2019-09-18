#!/bin/zsh
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DPATCHMAP        bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_patchmap &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DKHASH           bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_khash &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DFLATMAP         bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_flatmap &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DBYTELL          bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_bytell &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DSPARSEPP        bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_sparsepp &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DUNORDERED_MAP   bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_unordered_map &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DSPARSEMAP       bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_sparsemap &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DDENSEMAP        bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_densemap &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DSPARSE_PATCHMAP bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_patchmap_sparse &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DPATCHMAP -DPATCHMAP_EXPANSIVE bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_patchmap_expansive &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DJUDY            bench_ordered_patch_map.cpp -lboost_container -lprocps -lJudy -o bench_judy &
#g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DWHASH           bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_whash &
g++ -std=c++17 -I/home/usr/src/wmath/ -labsl_base_libbase -labsl_hash_libhash -labsl_base_libspinlock_wait -mpclmul -O3 -DNDEBUG -DABSL bench_ordered_patch_map.cpp -lprocps -o bench_absl &
g++ -std=c++17 -I/home/usr/src/wmath/ -I/home/usr/src/sparse-map/include/tsl/ -mpclmul -O3 -DNDEBUG -DTSL bench_ordered_patch_map.cpp -lprocps -o bench_tsl &
g++ -Wfatal-errors -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -I/home/usr/src/folly -mpclmul -O3 -DNDEBUG -DF14 bench_ordered_patch_map.cpp -lboost_container -lprocps /home/usr/src/folly/libfolly.a -o bench_f14 &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/robin-hood-hashing/src/include -I/home/usr/src/wmath/ -mpclmul -O3 -DNDEBUG -DROBINMAP bench_ordered_patch_map.cpp -lboost_container -lprocps -o bench_robinmap &
wait
