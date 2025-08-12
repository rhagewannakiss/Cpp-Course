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

// - - - - - - - - - - PAGE - - - - - - - - - -
    class page_t {
    private:
        int     id;
        int     content;

        bool    is_LIR;
        bool    is_in_cache;
        bool    is_in_stack;
        bool    is_in_queue;

        std::list<page_t*>::iterator stack_it;  //iterators for stack and qeue
        std::list<page_t*>::iterator queue_it;

    public:
        page_t(int id_, int content_)
        : id(id_)
        , content(content_)
        , is_LIR(false)
        , is_in_cache(false)
        , is_in_stack(false)
        , is_in_queue(false)
        {}

    friend class list_t;
    friend class cache_t;
    };

// - - - - - - - - - - CACHE - - - - - - - - - -

    class cache_t {
    private:
        int max_LIRS_cap;
        int max_LIR_cap;
        int max_HIR_cap;

        std::list <lirs::page_t*>              stack;
        std::list <lirs::page_t*>              queue;
        std::unordered_map<int, lirs::page_t*> page_map;  //hash map (for both list and queue)


        void     manage_excess_pages_in_stack();
    public:
        int hits;

        cache_t::cache_t(int size_, std::list* <lirs::page_t*> stack_, std::list* <lirs::page_t*> queue_)
        : max_LIRS_cap(size_)
        , stack(stack_)
        , queue(queue_)
        {
            max_HIR_cap = std::max(1, static_cast<int>(0.01 * size_));     //In the recommended setting, LIRS allocates 99% of the cache space to LIR blocks and 1% to resident HIR blocks.
            max_LIR_cap = max_LIRS_cap - max_HIR_cap;
            hits = 0;
        }

        cache_t::~cache_t() {
            for (auto it = page_map.begin(); it != page_map.end(); it++) {
                delete it->second;
            }
        }

        status_t access(int page_id);
        page_t*  get_page(int page_id);
        void     manage_queue();


        #ifndef NDEBUG

        void dump_to_file();

        #endif
    };


} // namespace lirs

#endif //LIRS_CACHE_HPP_