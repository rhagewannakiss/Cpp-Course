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
        int     id_;
        int     content_;

        bool    is_lir_;
        bool    is_in_cache_;
        bool    is_in_stack_;
        bool    is_in_queue_;

        std::list<page_t*>::iterator stack_it_;  //iterators for stack_ and qeue
        std::list<page_t*>::iterator queue_it_;

        page_t(int id, int content)
            : id_{id},
            content_{content},
            is_lir_{false},
            is_in_cache_{false},
            is_in_stack_{false},
            is_in_queue_{false}
        {}
    };
//------------------------------------------------

    cache_t(int size_)
        : max_lirs_cap_{size_},
        max_hir_cap_{std::max(1, max_lirs_cap_ / 2)},
        max_lir_cap_{max_lirs_cap_ - max_hir_cap_},
        lir_count_{0},
        hits_{0}
    {}

    ~cache_t() {
        for (auto it = page_map_.begin(); it != page_map_.end(); it++) {
            delete it->second;
        }
    }

    int get_hits() const {
        return hits_;
    }

    status_t access(int page_id);

    void dump_to_file();

private:
    const int max_lirs_cap_;
    const int max_hir_cap_;
    const int max_lir_cap_;

    int lir_count_;
    int hits_;

    std::list <page_t*>              stack_;
    std::list <page_t*>              queue_;
    std::unordered_map<int, page_t*> page_map_;


    status_t miss_case(int page_id, page_t* page);
    status_t lir_hit(page_t* page);
    status_t hir_hit(page_t* page);
    void     access_dump(page_t* page, int page_id);
    void     miss_dump(page_t* page, int page_id);
    void     hit_dump();


    page_t*  get_page(int page_id);

    void     push_front_s(page_t* page);
    void     push_front_q(page_t* page);

    void     demote_lir_to_hir();
    void     manage_queue();
    void     manage_excess_pages_in_stack();
};
} // namespace lirs

#endif // LIRS_CACHE_HPP_
