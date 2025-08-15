#ifndef LIRS_CACHE_HPP_
#define LIRS_CACHE_HPP_

#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <iterator>


int slow_get_page_int(int key);

namespace lirs {
enum status_t {
    miss = 0,
    hit  = 1
};

// - - - - - - - - - - CACHE - - - - - - - - - -

class cache_t {
public:
    struct page_t {
        int     id;
        int     content;

        bool    is_LIR;
        bool    is_in_cache;
        bool    is_in_stack;
        bool    is_in_queue;

        std::list<page_t*>::iterator stack_it;  //iterators for stack and qeue
        std::list<page_t*>::iterator queue_it;

        page_t(int id_, int content_)
        : id(id_)
        , content(content_)
        , is_LIR(false)
        , is_in_cache(false)
        , is_in_stack(false)
        , is_in_queue(false)
        {}
    };
    int hits;

    cache_t(int size_)
    : max_LIRS_cap(size_)
    {
        //max_HIR_cap = std::max(1, max_LIRS_cap / 2);
        max_HIR_cap = std::max(1, max_LIRS_cap - 1);
        max_LIR_cap = max_LIRS_cap - max_HIR_cap;
        hits = 0;
    }

   ~cache_t() {
        for (auto it = page_map.begin(); it != page_map.end(); it++) {
            delete it->second;
        }
    }

    status_t access(int page_id);
    page_t*  get_page(int page_id);
    void     manage_queue();
    void     push_front_s(page_t* page);
    void     push_front_q(page_t* page);
    void     demote_LIR_to_HIR();

    void dump_to_file();

private:
    int max_LIRS_cap;
    int max_LIR_cap;
    int max_HIR_cap;

    int lir_count = 0;

    std::list <page_t*>              stack;
    std::list <page_t*>              queue;
    std::unordered_map<int, page_t*> page_map;  //hash map (for both list and queue)


    void     manage_excess_pages_in_stack();
};
} // namespace lirs

#endif //LIRS_CACHE_HPP_