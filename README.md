# patchmap
A fast and memory efficient hashmap using sorting to resolve collisions

This hashmap is inspired by the 1973 publication "Ordered hash tables".
But instead of resolving the collisions via an ordering of the keys themselves
this ordered_patch_map resolves the collisions with an ordering defined by the
hash-value of the keys.
As it is very easy to fulfill this ordering without collisions and can be upheld with
collisions too, this improves upon the upper bound when retrieving from
O(n) to O(log(n)) and enables an interpolation search that
performs really well for retrieving keys even if the table is almost full.

This is very similar to linear bidirectional probing with robin hood hashing
(see for example sherwood_map) but with interpolation search leading to improved
upper bounds while retaining the same average complexity and overall performance,
possibly exceeding most implementations at high load factors around 96%.

If you are interested using this container contact me and I will make it work for you.

If you are interested in understanding the patchmap or want to implement it in your
favourite programming language you should have a look at ordered_patch_map_v0.hpp.
This is a oversimplified the prototype, to ease the understanding.
ordered_patch_map.hpp contains many c++ specific things, generics, templates, etc.

Compile with -std=c++17 -DNDEBUG
