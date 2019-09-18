#!/bin/zsh
n=$((2**27))
for i in {0..2} ; do ./bench_densemap           $n; done | sort -gk4 | sed '2q;d' >> google_dense_hash_map.point
for i in {0..2} ; do ./bench_f14                $n; done | sort -gk4 | sed '2q;d' >> f14.point
for i in {0..2} ; do ./bench_flatmap            $n; done | sort -gk4 | sed '2q;d' >> ska_flat_hash_map.point
for i in {0..2} ; do ./bench_patchmap           $n; done | sort -gk4 | sed '2q;d' >> ordered_patch_map.point
for i in {0..2} ; do ./bench_patchmap_sparse    $n; done | sort -gk4 | sed '2q;d' >> ordered_patch_map.point
for i in {0..2} ; do ./bench_patchmap_expansive $n; done | sort -gk4 | sed '2q;d' >> ordered_patch_map.point
for i in {0..2} ; do ./bench_bytell             $n; done | sort -gk4 | sed '2q;d' >> bytell_hash_map.point
for i in {0..2} ; do ./bench_absl               $n; done | sort -gk4 | sed '2q;d' >> absl.point
for i in {0..2} ; do ./bench_robinmap           $n; done | sort -gk4 | sed '2q;d' >> robinmap.point
for i in {0..2} ; do ./bench_khash              $n; done | sort -gk4 | sed '2q;d' >> khash.point
#for i in {0..2} ; do ./bench_sparsepp           $n; done | sort -gk4 | sed '2q;d' >> sparsepp.point
#for i in {0..2} ; do ./bench_ktprime            $n; done | sort -gk4 | sed '2q;d' >> ktprime.point
#for i in {0..2} ; do ./bench_sparsemap          $n; done | sort -gk4 | sed '2q;d' >> google_sparse_hash_map.point
#for i in {0..2} ; do ./bench_unordered_map      $n; done | sort -gk4 | sed '2q;d' >> std_unordered_map.point
#for i in {0..2} ; do ./bench_judy               $n; done | sort -gk4 | sed '2q;d' >> judy.point
#for i in {0..2} ; do ./bench_whash              $n; done | sort -gk4 | sed '2q;d' >> whash.point
