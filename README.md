# ordered_patch_map
A fast and memory efficient hashmap using sorting to resolve collisions

This hashmap is inspired by the 1973 publication "Ordered hash tables".
But instead of resolving the collisions via an ordering of the keys themselves
this ordered_patch_map resolves the collisions with an ordering defined by the
hash-value of the keys.
As it is very easy to fulfill this ordering without collisions and can be upheld with
collisions too, this enables an interpolation search that performs really well for
retrieving keys even if the table is close to 100% full.

The overhead when inserting is proportional to 1+1/(1-α) with α being the load factor.

This is very similar to linear probing with robin hood hashing but with the possibility
of interpolation search leading to an improved upper bound when retrieving keys of
O(log(n)) compared to O(n) in case of the usual hashmap implementation.

If you are interested using this container contact me.
