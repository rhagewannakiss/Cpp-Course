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

    std::cerr << "accessing " << page_id << " page ptr=" << page << " in_cache=" << page->is_in_cache << "\n";


    if (page->is_in_cache == true) {
        if (page->is_LIR == true) { //LIR-hit
            push_front_s(page);
            manage_excess_pages_in_stack();
            hits++;

            #ifndef NDEBUG
                std::cerr << "S: ";
                for (auto p : stack) std::cerr << p->id << (p->is_LIR ? "L " : "H ");
                std::cerr << " | Q: ";
                for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
                std::cerr << "\n";
            #endif

            return hit;
        } else {                 // HIR-hit
            push_front_s(page);
            page->is_LIR = true;
            lir_count++;

            if (page->is_in_queue == true) {
                queue.erase(page->queue_it);
                page->is_in_queue = false;
            }

            if (lir_count > max_LIR_cap) {
                demote_LIR_to_HIR();
            }

            manage_excess_pages_in_stack();
            hits++;

            #ifndef NDEBUG
            std::cerr << "S: ";
            for (auto p : stack) std::cerr << p->id << (p->is_LIR ? "L " : "H ");
            std::cerr << " | Q: ";
            for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
            std::cerr << "\n";
            #endif

            return hit;
        }

    } else {                     // miss
        if (lir_count < max_LIR_cap) {  //if the amount of LIR pages in stack isn't equal or more than the max LIR capacity
            push_front_s(page);
            page->is_LIR = true;;
            page->is_in_cache = true;
            lir_count++;

            #ifndef NDEBUG
                std::cerr << "MISS: added " << page_id << " to S and Q; queue.size=" << queue.size() << " stack.size=" << stack.size() << "\n";
                std::cerr << "S: ";
                for (auto p : stack) std::cerr << p->id << (p->is_LIR ? "L " : "H ");
                std::cerr << " | Q: ";
                for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
                std::cerr << "\n";
            #endif

            manage_excess_pages_in_stack();
            return miss;
        }

        if (page->is_in_stack == true) {  //if the page was in stack sometime ago and became non-resident HIR after being demoted
            push_front_s(page);
            page->is_LIR = true;
            page->is_in_cache = true;
            lir_count++;

            if (page->is_in_queue == true) {
                queue.erase(page->queue_it);
                page->is_in_queue = false;
            }

            if (lir_count > max_LIR_cap) {
                demote_LIR_to_HIR();
            }

            #ifndef NDEBUG
                std::cerr << "MISS: added " << page_id << " to S and Q; queue.size=" << queue.size() << " stack.size=" << stack.size() << "\n";
                std::cerr << "S: ";
                for (auto p : stack) std::cerr << p->id << (p->is_LIR ? "L " : "H ");
                std::cerr << " | Q: ";
                for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
                std::cerr << "\n";
            #endif

            manage_excess_pages_in_stack();
            return miss;
        }

        push_front_s(page);  //if the page is completely new we add it to the stack and promot to HIR
        page->is_LIR = false;
        push_front_q(page);

        #ifndef NDEBUG
            std::cerr << "MISS: added " << page_id << " to S and Q; queue.size=" << queue.size() << " stack.size=" << stack.size() << "\n";
            std::cerr << "S: ";
            for (auto p : stack) std::cerr << p->id << (p->is_LIR ? "L " : "H ");
            std::cerr << " | Q: ";
            for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
            std::cerr << "\n";
        #endif

        manage_excess_pages_in_stack();
        return miss;
    }
}



void cache_t::push_front_s(page_t* page) {
    assert(page != nullptr);

    if (page->is_in_stack) {
        stack.erase(page->stack_it);
    }

    stack.push_front(page);
    page->stack_it = stack.begin();

    page->is_in_stack = true;
}

void cache_t::push_front_q(page_t* page) {
    assert(page != nullptr);

    if (page->is_in_queue) {
        queue.erase(page->queue_it);
    }

    queue.push_front(page);
    page->queue_it    = queue.begin();
    page->is_in_queue = true;
    page->is_in_cache = true;

    while (static_cast<int>(queue.size()) > max_HIR_cap) {
        page_t* ex = queue.back();
        queue.pop_back();
        ex->is_in_queue = false;
        ex->is_in_cache = false;
    }
}

/*void cache_t::demote_LIR_to_HIR() {
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        cache_t::page_t* cand = *it;

        if (cand->is_LIR) {
            cand->is_LIR = false;
            lir_count--;

            push_front_q(cand);
            break;
        }
    }
}*/
void cache_t::demote_LIR_to_HIR() {
    auto it = stack.end();

    while (it != stack.begin()) {
        --it;
        page_t* cand = *it;

        if (cand->is_LIR) {
            cand->is_LIR      = false;
            cand->is_in_stack = false;

            queue.push_front(cand);
            cand->queue_it = queue.begin();
            cand->is_in_queue = true;

            manage_queue();

            break;
        }
    }
}

void cache_t::manage_queue() {
    if (static_cast<int>(queue.size()) > max_HIR_cap) {
        page_t* bye_bitch = queue.back();
        queue.pop_back();

        bye_bitch->is_in_cache = false;
        bye_bitch->is_in_queue = false;
     }
}

cache_t::page_t* cache_t::get_page(int page_id) {
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

void cache_t::dump_to_file() {
    #ifndef NDEBUG

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

    dump << "\n\n*********** QUEUE ***********\n ";
    for (auto q_it = queue.begin(); q_it != queue.end(); q_it++) {
        page_t* current_page = *q_it;

        dump << "page id: " << current_page->id
             << " | is LIR: " << (current_page->is_LIR ? "yes" : "no")
             << " | is in cache: " << (current_page->is_in_cache ? "yes" : "no")
             << "\n";
    }

    dump.close();
    std::cout << "Dump written to file named \"cache_dump\"\n";

    #endif
}
} //namespace lirs