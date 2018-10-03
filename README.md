# ordered_patch_map
A fast and memory efficient hashmap using sorting to resolve collisions

This hashmap is inspired by the 1973 publication "Ordered hash tables".
But instead of resolving the collisions via an ordering of the keys themselves
this ordered_patch_map resolves the collisions with an ordering defined by the
hash-value of the keys.
As it is very easy to fulfill this ordering without collisions and can be upheld with
collisions too, this improves upon the upper bound of O(n^2) to O(n) when inserting
and when retrieving from O(n) to O(log(n) and enables an interpolation search that
performs really well for retrieving keys even if the table is close to 100% full.

This is very similar to linear probing with robin hood hashing
(see for example sherwood_map) but with interpolation search leading to improved
upper bounds while retaining the same average complexity and overall performance,
possibly exceeding most implementations at high load factors around 96%.

If you are interested using this container contact me and I will make it work for you,
right now I just want to see if it's worth it to put in the effort.

Compile with -mpclmul -std=c++17
