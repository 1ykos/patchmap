#!/bin/zsh
g++ -std=c++17 -I/home/usr/src/wmath/ -march=native  -O3 -DNDEBUG -DPATCHMAP        benchmark.cpp  -lprocps -o bench_patchmap &
g++ -std=c++17 -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DKHASH           benchmark.cpp  -lprocps -o bench_khash &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DFLATMAP         benchmark.cpp -lprocps -o bench_flatmap &
g++ -std=c++17 -I/home/usr/src/wmath/ -I/home/usr/src/flat_hash_map -march=native -O3 -DNDEBUG -DBYTELL benchmark.cpp -lprocps -o bench_bytell &
#g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DSPARSEPP        benchmark.cpp -lprocps -o bench_sparsepp &
g++ -std=c++17 -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DUNORDERED_MAP   benchmark.cpp -lprocps -o bench_unordered_map &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DSPARSEMAP       benchmark.cpp -lprocps -o bench_sparsemap &
g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DDENSEMAP        benchmark.cpp -lprocps -o bench_densemap &
#g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DSPARSE_PATCHMAP benchmark.cpp -lprocps -o bench_patchmap_sparse &
#g++ -std=c++17 -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DPATCHMAP -DPATCHMAP_EXPANSIVE benchmark.cpp -lprocps -o bench_patchmap_expansive &
g++ -std=c++17 -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DJUDY benchmark.cpp -lprocps -lJudy -o bench_judy &
#g++ -std=c++17 -I/home/usr/src/sparsepp -I/home/usr/src/flat_hash_map -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DWHASH benchmark.cpp -lprocps -o bench_whash &
#g++ -std=c++17 -I/home/usr/src/wmath/ -labsl_base_libbase -labsl_hash_libhash -labsl_base_libspinlock_wait -march=native -O3 -DNDEBUG -DABSL benchmark.cpp -lprocps -o bench_absl &
g++ -std=c++17 -I/home/usr/src/wmath/ -I/home/usr/src/sparse-map/include/tsl/ -march=native -O3 -DNDEBUG -DTSL benchmark.cpp -lprocps -o bench_tsl &
#g++ -Wfatal-errors -std=c++17 -I/home/usr/src/wmath/ -I/home/usr/src/folly -march=native -O3 -DNDEBUG -DF14 benchmark.cpp -lprocps /home/usr/src/folly/libfolly.a -o bench_f14 &
g++ -std=c++17 -I/home/usr/src/robin-hood-hashing/src/include -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DROBINMAP benchmark.cpp -lprocps -o bench_robinmap &
#g++ -std=c++17 -march=native -O3 -DNDEBUG -I/home/usr/src/wmath/ -DHORDI benchmark.cpp -lprocps -o bench_hordi &
g++ -std=c++17 -I/home/usr/src/hash_trie -I/home/usr/src/wmath/ -march=native -O3 -DNDEBUG -DPHAMT benchmark.cpp -lprocps -o bench_hash_trie &
wait
