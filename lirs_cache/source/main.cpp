#include "lirs_cache.hpp"

#include <iostream>
#include <vector>

int main() {
    int cache_size = 0;
    int num_of_elems = 0;

    std::cin >> cache_size >> num_of_elems;

    std::vector<int> elems = {};
    elems.reserve(num_of_elems);

    std::cout << "Cashe size: " << cache_size << "\nNum of elems: " << num_of_elems << "\n";

    return 0;
}
