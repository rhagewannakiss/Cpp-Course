// Входные и выходные данные
// - На stdin размер кэша и кол-во элементов, потом сами элементы (целыми числами)
// пример: вход 2 6 1 2 1 2 1 2 выход 4

//TODO -  добавьте сравнение с идеальным кэшированием

#include "lirs_cache.hpp"

#include <cassert>
#include <fstream>
#include <unordered_map>
#include <memory>


std::unordered_map<int, lirs::page_t*> page_map;  //hash map (for both list and queue)

int slow_get_page_int(int key) {
    return key;
}


namespace lirs {
//------------------- LIST FUNCTIONS ----------------------
    void list_t::push_front_s(page_t* page) {
        assert(page != nullptr);

        if (page->is_in_stack) {
            remove_s(page);
        }


        page->prev_in_stack = nullptr;
        page->next_in_stack = head;

        if (head != nullptr) {
            head->prev_in_stack = page;
        }
        if (tail == nullptr) {
            tail = page;
        }

        head = page;
        page->is_in_stack = true;
        size++;
    }

    void list_t::push_front_q(page_t* page) {
        assert(page != nullptr);

        if (page->is_in_queue) {
            remove_s(page);
        }


        page->prev_in_queue = nullptr;
        page->next_in_queue = head;

        if (head != nullptr) {
            head->prev_in_queue = page;
        }
        if (tail == nullptr) {
            tail = page;
        }

        head = page;
        page->is_in_queue = true;
        size++;
    }

    void list_t::remove_s(page_t* page) {
        assert(page != nullptr);

        if (page->is_in_stack != true) {
            return;
        }

        if (page->prev_in_stack != nullptr) {
            page->prev_in_stack->next_in_stack = page->next_in_stack;
        } else {
            head = page->next_in_stack;
        }

        if (page->next_in_stack != nullptr) {
            page->next_in_stack->prev_in_stack = page->prev_in_stack;
        } else {
            tail = page->prev_in_stack;
        }

        page->next_in_stack = nullptr;
        page->prev_in_stack = nullptr;
        page->is_in_stack = false;

        size--;
    }

     void list_t::remove_q(page_t* page) {
        assert(page != nullptr);

        if (page->is_in_queue != true) {
            return;
        }

        if (page->prev_in_queue != nullptr) {
            page->prev_in_queue->next_in_queue = page->next_in_queue;
        } else {
            head = page->next_in_queue;
        }

        if (page->next_in_queue != nullptr) {
            page->next_in_queue->prev_in_queue = page->prev_in_queue;
        } else {
            tail = page->prev_in_queue;
        }

        page->next_in_queue = nullptr;
        page->prev_in_queue = nullptr;
        page->is_in_queue = true;

        size--;
    }


    page_t* list_t::pop_tail_s() {
        if (tail != nullptr) {
            auto ex = tail;
            remove_s(ex);
            return ex;
        }
        std::cerr << "Empty stack!";
        return nullptr;
    }

    page_t* list_t::pop_tail_q() {
        if (tail != nullptr) {
            auto ex = tail;
            remove_q(ex);
            return ex;
        }
        std::cerr << "Empty queue!";
        return nullptr;
    }

    void list_t::stack_dtor() {
        auto current = head;

        while (current != nullptr) {
            auto next = current->next_in_stack;
            current->prev_in_stack = nullptr;
            current->next_in_stack = nullptr;
            current = next;
        }

        head = nullptr;
        tail = nullptr;
        size = 0;
    }

    void list_t::queue_dtor() {
        auto current = head;

        while (current != nullptr) {
            auto next = current->next_in_queue;
            current->prev_in_queue = nullptr;
            current->next_in_queue = nullptr;
            current = next;
        }

        head = nullptr;
        tail = nullptr;
        size = 0;
    }

//--------------------- CACHE FUNCTIONS --------------------------

    status_t cache_t::access(int page_id) {
        page_t* page = get_page(page_id);
        assert(page != nullptr);

        if (page->is_in_cache == 1) {
            if (page->is_LIR == 1) {
                stack->remove_s(page);
                stack->push_front_s(page);

                hits++;
                return hit;
            } else {
                queue->remove_q(page);

                page->is_LIR = 1;
                stack->push_front_s(page);

                auto tmp = stack->tail;
                while (tmp->is_LIR != 1 && tmp != nullptr) {
                    tmp = tmp->prev_in_stack;
                }
                if (tmp != nullptr) {
                    tmp->is_LIR = 0;
                    queue->push_front_q(tmp);
                }

                manage_excess_pages_in_stack();
                hits++;
                return hit;
            }
        } else {
            stack->push_front_s(page);

            page->is_in_cache = 1;
            page->is_LIR      = 0;

            queue->push_front_q(page);
            if (queue->size >= max_HIR_cpct_) {
                auto ex = queue->pop_tail_q();
                ex->is_in_cache = 0;
            }

            manage_excess_pages_in_stack();

            return miss;
        }
    }

    page_t* cache_t::get_page(int page_id) {     //FIXME
        auto tmp = page_map.find(page_id);

        if (tmp != page_map.end()) {
            return tmp->second;
        }

        int page_content = slow_get_page_int(page_id);
        page_t* new_page = new page_t(page_id, page_content);

        new_page->is_LIR        = 0;
        new_page->is_in_cache   = 0;
        new_page->prev_in_stack = nullptr;
        new_page->next_in_stack = nullptr;
        new_page->prev_in_queue = nullptr;
        new_page->next_in_queue = nullptr;

        page_map[page_id] = new_page;

        return new_page;
    }

    void cache_t::manage_excess_pages_in_stack() {
        while (stack->tail != nullptr
            && stack->tail->is_LIR == 0
            && stack->tail->is_in_cache == 0) {
                stack->pop_tail_s();
            }
    }


    //-------------------------- DUMP --------------------------
    #ifdef DEBUG
        //TODO - добавить в CMake режим дебага и релиза - ?
        #include <fstream>

        void cache_t::dump_to_file() {
            std::ofstream dump("cache_dump.txt");
            if (! dump.is_open()) {
                std::cerr << "Error opening dump file\n";
                return;
            }
            dump << "------------ CACH DUMP ------------\n";

            dump << "*********** STACK ***********\n ";
            lirs::page_t* current_s = stack->head;
            int tmp_s = stack->size;
            while (tmp_s > 0) {
                dump << "page id: " << current_s->id
                     << " | is LIR: " << (current_s->is_LIR ? "yes" : "no")
                     << " | is in cache: " << (current_s->is_in_cache ? "yes" : "no")
                     << "\n";
                     tmp_s--;
            current_s = current_s->next_in_stack;
            }

            dump << "*********** QUEUE ***********\n ";
            lirs::page_t* current_q = queue->head;
            int tmp_q = queue->size;
            while (tmp_q > 0) {
                dump << "page id: " << current_q->id
                     << " | is LIR: " << (current_q->is_LIR ? "yes" : "no")
                     << " | is in cache: " << (current_q->is_in_cache ? "yes" : "no")
                     << "\n";
                     tmp_q--;
            current_q = current_q->next_in_queue;
            }

            dump << "bebra\n";
//TODO - добавить бенчи

            dump.close();
            std::cout << "Dump written to file named \"cache_dump\"\n";
        }

        #endif
}