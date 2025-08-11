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

    if (std::cin.good() != 1 || cache_size == 0 || num_of_elems <= 0) {
        std::cout << "Input error!\n";
        return 1;
    }

    std::vector<lirs::page_t> elems;
    elems.reserve(cache_size);    //FIXME - conversion warning  -- static cast?

    std::cout << "\nCashe size: " << cache_size << "\nNum of elems: " << num_of_elems << '\n';

    lirs::list_t stack{nullptr, nullptr, 0};
    lirs::list_t queue{nullptr, nullptr, 0};

    lirs::cache_t cache{cache_size, &elems, &stack, &queue};

    std::cout << "\nPlease input elements:  \n--> ";
    for (int i = 0; i < num_of_elems; i++) {
        int page_id = 0;
        std::cin >> page_id;
        assert(std::cin.good());

        cache.access(page_id);
        }

    #ifdef DEBUG
    cache.dump_to_file();
    #endif //DEBUG

    stack.stack_dtor();
    queue.queue_dtor();

    std::cout << "\nHits: " << cache.hits << '\n';
    return 0;
}