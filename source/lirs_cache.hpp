#ifndef LIRS_CACHE_HPP_
#define LIRS_CACHE_HPP_

#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <iterator>


int slow_get_page_int(int key);

namespace lirs {

enum class status_t {
    miss = 0,
    hit  = 1
};

// - - - - - - - - - - CACHE - - - - - - - - - -

class cache_t {
public:
//------------------- PAGE ----------------------
    struct page_t {
        int     id;
        int     content;

        bool    is_lir;
        bool    is_in_cache;
        bool    is_in_stack;
        bool    is_in_queue;

        std::list<page_t*>::iterator stack_it;  //iterators for stack and qeue
        std::list<page_t*>::iterator queue_it;

        page_t(int id_, int content_)
        : id(id_)
        , content(content_)
        , is_lir(false)
        , is_in_cache(false)
        , is_in_stack(false)
        , is_in_queue(false)
        {}
    };
//------------------------------------------------

    cache_t(int size_)
    : max_lirs_cap_(size_)
    , max_hir_cap_(std::max(1, max_lirs_cap_ / 2))
    , max_lir_cap_(max_lirs_cap_ - max_hir_cap_)
    , lir_count_(0)
    , hits_(0)
    {}

    ~cache_t() {
        for (auto it = page_map.begin(); it != page_map.end(); it++) {
            delete it->second;
        }
    }

    int get_hits() const {
        return hits_;
    }

    status_t access(int page_id);

    void dump_to_file();

private:
    int const max_lirs_cap_;
    int const max_hir_cap_;
    int const max_lir_cap_;
    int       lir_count_;
    int       hits_;

    std::list <page_t*>              stack;
    std::list <page_t*>              queue;
    std::unordered_map<int, page_t*> page_map;  //hash map (for both list and queue)


    status_t miss_case(int page_id, page_t* page);
    status_t lir_hit(int page_id, page_t* page);
    status_t hir_hit(int page_id, page_t* page);

    page_t*  get_page(int page_id);

    void     push_front_s(page_t* page);
    void     push_front_q(page_t* page);

    void     demote_lir_to_hir();
    void     manage_queue();
    void     manage_excess_pages_in_stack();
};
} // namespace lirs

#endif //LIRS_CACHE_HPP_
