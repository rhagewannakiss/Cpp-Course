#ifndef LIRS_CACHE_HPP_
#define LIRS_CACHE_HPP_

#include <iostream>
#include <vector>
#include <list>

inline int max_LIRS_cpct_ = 256;
inline int max_LIR_cpct_  = max_LIRS_cpct_/2;
inline int max_HIR_cpct_  = max_LIRS_cpct_/2;

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

        page_t* prev_in_stack;
        page_t* next_in_stack;

        page_t* prev_in_queue;
        page_t* next_in_queue;

    public:
        page_t(int id_, int content_)
        : id(id_)
        , content(content_)
        , is_LIR(false)
        , is_in_cache(false)
        , is_in_stack(false)
        , is_in_queue(false)
        , prev_in_stack(nullptr)
        , next_in_stack(nullptr)
        , prev_in_queue(nullptr)
        , next_in_queue(nullptr)
        {}

    friend class list_t;
    friend class cache_t;
    };

// - - - - - - - - - - DOUBLE LINKED LIST - - - - - - - - - -

    class list_t {
    private:
        page_t* head;
        page_t* tail;
        int     size;

    public:
        list_t(page_t* head_, page_t* tail_, int size_)
        : head(head_)
        , tail(tail_)
        , size(size_)
        {}

        void    push_front_s(page_t* page);
        void    push_front_q(page_t* page);
        void    remove_s(page_t* page);
        void    remove_q(page_t* page);
        void    stack_dtor();
        void    queue_dtor();
        page_t* pop_tail_s();
        page_t* pop_tail_q();


        friend class cache_t;
    };

// - - - - - - - - - - CACHE - - - - - - - - - -

    class cache_t {
    private:
        int                        size;
        std::vector<lirs::page_t>* content;
        lirs::list_t*              stack;
        lirs::list_t*              queue;

    public:
        int                        hits;


        cache_t(int size_, std::vector<lirs::page_t>* content_, lirs::list_t* stack_, lirs::list_t* queue_)
        : size(size_)
        , content(content_)
        , stack(stack_)
        , queue(queue_)
        {}

        status_t access(int page_id);
        page_t*  get_page(int page_id);
        void     manage_excess_pages_in_stack();


        #ifdef DEBUG
//TODO - добавить в CMake режим дебага и релиза
        void dump_to_file();

        #endif  //DEBUG
    };
} // namespace lirs

#endif //LIRS_CACHE_HPP_