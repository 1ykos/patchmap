# patchmap
A fast and memory efficient hashmap using sorting to resolve collisions

```C++
#include <iostream>
#include "patchmap.hpp"
int main() {
  whash::patchmap<int,int> hash_table;
  hash_table[7] = 77;
  for (const auto& elem : hash_table) {
    std::cout << elem.first << " " << elem.second << std::endl;
  }
}
```
```bash
> g++ -std=c++17 -lboost_container -DNDEBUG main.cpp
```

This hashmap is inspired by the 1973 publication "Ordered hash tables".
But instead of resolving the collisions via an ordering defined by the keys directly
the patchmap resolves the collisions with an ordering defined by the
hash-value of the keys.
Without collisions keys in a hash table are alreay in hash-order.
When there are collisions in the patchmap this order is upheld by inserting
the key-value pairs at the appropriate position, displacing keys that compare greater
to the right and keys that compare less to the left.
This improves the commonly encountered worst-case complexity of O(n) for lookups to
O(log(n)) as a binary search can be employed. In addition, when the hash values are
evenly distributed, which is the case for the hash functions supplied with this library,
this allowes for an asymptotic complexity of O(log(-log(1-a))) because an interpolation search
can be used to retrieve the entries, performing really well even if the table is almost full.

This is very similar to linear bidirectional probing with robin hood hashing
(see for example sherwood_map) but with interpolation search leading to improved
upper bounds while retaining the same average complexity and overall performance,
exceeding other implementations at high load factors around 96% at the cost of O((1-a)¯²)
reordering operations for insertions and deletions however.

If you are interested using this container contact me and I will make it work for you,
if it does just work out of the box.

If you are interested in understanding the patchmap or want to implement it in your
favourite programming language you should have a look at patchmap_v0.hpp.
This is a oversimplified the prototype, to ease the understanding without templates
and binary search instead of interpolation search.
