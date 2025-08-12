// Входные и выходные данные
// - На stdin размер кэша и кол-во элементов, потом сами элементы (целыми числами)
// пример: вход 2 6 1 2 1 2 1 2 выход 4

//TODO -  добавьте сравнение с идеальным кэшированием

#include "lirs_cache.hpp"

#include <cassert>
#include <fstream>
#include <list>
#include <iterator>


int slow_get_page_int(int key) {
    return key;
}


namespace lirs {
    status_t cache_t::access(int page_id) {
        page_t* page = get_page(page_id);
        assert(page != nullptr);

        if (page->is_in_cache == true) {
            if (page->is_LIR == true) { //LIR-hit
                if (page->is_in_stack) {
                    stack.erase(page->stack_it);
                }

                stack.push_front(page);
                page->stack_it = stack.begin();

                page->is_in_stack = true;

                hits++;
                return hit;
            } else {                 // HIR-hit
                if (page->is_in_queue == true) {
                    queue.erase(page->queue_it);
                    page->is_in_queue = false;
                }

                stack.push_front(page);
                page->stack_it    = stack.begin();
                page->is_in_stack = true;
                page->is_LIR      = true;

                for (auto it = stack.end(); it != stack.begin(); ) {
                    it--;

                    page_t* bye_bitch = *it;
                    if (bye_bitch->is_LIR == true) {
                        bye_bitch->is_LIR      = false;
                        bye_bitch->is_in_stack = false;

                        queue.push_front(bye_bitch);
                        bye_bitch->queue_it = queue.begin();
                        bye_bitch->is_in_queue = true;

                        manage_queue();
                    }
                }


                manage_excess_pages_in_stack();
                hits++;
                return hit;
            }
        } else {                     // miss
            stack.push_front(page);
            page->stack_it    = stack.begin();
            page->is_in_stack = true;
            page->is_in_cache = true;
            page->is_LIR      = false;

            queue.push_front(page);
            page->queue_it    = queue.begin();
            page->is_in_queue = true;

            manage_queue();

            manage_excess_pages_in_stack();
            return miss;
        }
    }



    void cache_t::manage_queue() {
        if (queue.size() >= max_HIR_cap) {
            page_t* bye_bitch = queue.back();
            queue.pop_back();

            bye_bitch->is_in_cache = false;
            bye_bitch->is_in_queue = false;
         }
    }

    page_t* cache_t::get_page(int page_id) {
        auto tmp = page_map.find(page_id);

        if (tmp != page_map.end()) {
            return tmp->second;
        }

        int page_content = slow_get_page_int(page_id);
        page_t* new_page = new page_t(page_id, page_content);

        new_page->is_LIR        = false;
        new_page->is_in_cache   = false;

        page_map[page_id] = new_page;

        return new_page;
    }

    void cache_t::manage_excess_pages_in_stack() {
        while (stack.empty()             == false
            && stack.back()->is_LIR      == false
            && stack.back()->is_in_cache == false) {
                page_t* last = stack.back();
                stack.pop_back();
                last->is_in_stack = false;
        }
    }


    //-------------------------- DUMP --------------------------
    #ifndef NDEBUG
        #include <fstream>

        void cache_t::dump_to_file() {
            std::ofstream dump("cache_dump.txt");
            if (! dump.is_open()) {
                std::cerr << "Error opening dump file\n";
                return;
            }

            dump << "------------ CACH DUMP ------------\n";

            dump << "*********** STACK ***********\n ";
            for (auto s_it = stack.begin(); s_it != stack.end(); s_it++) {
                page_t* current_page = *s_it;

                dump << "page id: " << current_page->id
                     << " | is LIR: " << (current_page->is_LIR ? "yes" : "no")
                     << " | is in cache: " << (current_page->is_in_cache ? "yes" : "no")
                     << "\n";
            }

            dump << "*********** QUEUE ***********\n ";
            for (auto q_it = queue.begin(); q_it != queue.end(); q_it++) {
                page_t* current_page = *q_it;

                dump << "page id: " << current_page->id
                     << " | is LIR: " << (current_page->is_LIR ? "yes" : "no")
                     << " | is in cache: " << (current_page->is_in_cache ? "yes" : "no")
                     << "\n";
            }

            dump.close();
            std::cout << "Dump written to file named \"cache_dump\"\n";
        }

        #endif
}