#!/bin/zsh
n=$((2**27))
for i in {0..2} ; do ./bench_densemap   $n ; done 2> google_dense_hash_map.stat | sort -gk4 | sed '2q;d' > google_dense_hash_map.point
#for i in {0..2} ; do ./bench_f14        $n ; done 2>> f14.stat | sort -gk4 | sed '2q;d' >> f14.point
for i in {0..2} ; do ./bench_flatmap    $n ; done 2> ska_flat_hash_map.stat | sort -gk4 | sed '2q;d' > ska_flat_hash_map.point
for i in {0..2} ; do ./bench_patchmap   $n ; done 2> ordered_patch_map.stat | sort -gk4 | sed '2q;d' > ordered_patch_map.point
for i in {0..2} ; do ./bench_bytell     $n ; done 2> bytell_hash_map.stat   | sort -gk4 | sed '2q;d' > bytell_hash_map.point
#for i in {0..2} ; do ./bench_absl       $n 2>> absl.stat                  ; done | sort -gk4 | sed '2q;d' >> absl.point
for i in {0..2} ; do ./bench_robinmap $n ; done 2> robinmap.stat | sort -gk4 | sed '2q;d' > robinmap.point
for i in {0..2} ; do ./bench_khash $n ; done 2> khash.stat | sort -gk4 | sed '2q;d' > khash.point
#for i in {0..2} ; do ./bench_hash_trie $n ; done 2> hash_trie.stat | sort -gk4 | sed '2q;d' > hash_trie.point
#for i in {0..2} ; do ./bench_sparsepp           $n; done | sort -gk4 | sed '2q;d' >> sparsepp.point
#for i in {0..2} ; do ./bench_ktprime            $n; done | sort -gk4 | sed '2q;d' >> ktprime.point
for i in {0..2} ; do ./bench_sparsemap $n; done 2> google_sparse_hash_map.stat | sort -gk4 | sed '2q;d' > google_sparse_hash_map.point
for i in {0..2} ; do ./bench_unordered_map $n ; done 2> std_unordered_map.point | sort -gk4 | sed '2q;d' > std_unordered_map.point
#for i in {0..2} ; do ./bench_judy               $n; done | sort -gk4 | sed '2q;d' >> judy.point
#for i in {0..2} ; do ./bench_whash              $n; done | sort -gk4 | sed '2q;d' >> whash.point
for file in *.stat; do awk '{a[$1]+=$2;b[$1]+=$3;c[$1]+=$4;d[$1]+=$5;e[$1]+=$6;++n[$1]}END{for (i in a) print i,a[i]/n[i],b[i]/n[i],c[i]/n[i],d[i]/n[i],e[i]/n[i]}' $file | sort -n > ${file}u; done
