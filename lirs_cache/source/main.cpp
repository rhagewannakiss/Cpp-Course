#include "lirs_cache.hpp"

#include <cassert>
#include <iostream>
#include <vector>

int main() {
    int cache_size;
    int num_of_elems;

    std::cout << "Please input two positive integers: cache capacity & number of elements\n--> ";
    std::cin >> cache_size >> num_of_elems;
    assert(std::cin.good());

    if (std::cin.good() != 1 || cache_size <= 0 || num_of_elems <= 0) {
        std::cout << "Input error!\n";
        return 1;
    }

    std::cout << "\nCashe size: " << cache_size << "\nNum of elems: " << num_of_elems << '\n';
    lirs::cache_t cache{cache_size};

    std::cout << "\nPlease input elements:  \n--> ";
    for (int i = 0; i < num_of_elems; i++) {
        int page_id;
        std::cin >> page_id;
        assert(std::cin.good());

        cache.access(page_id);
        }

    #ifndef NDEBUG
    cache.dump_to_file();
    #endif //DEBUG

    std::cout << "\nHits: " << cache.hits << '\n';
    return 0;
}