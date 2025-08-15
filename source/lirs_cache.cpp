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

#ifndef NDEBUG
    std::cerr << "accessing " << page_id << " page ptr=" << page << " in_cache=" << page->is_in_cache << "\n";
#endif //NDEBUG

    if (page->is_in_cache) {
        if (page->is_lir) {
            //LIR-hit
            return lir_hit(page_id, page);

        } else {
            // HIR-hit
            return hir_hit(page_id, page);
        }

    } else {
        // miss
        return miss_case(page_id, page);
    }
}

//-------------------- HIT & MISS FUNC. -------------------------
status_t cache_t::lir_hit(int page_id, page_t* page) {
    assert(page != nullptr);

    push_front_s(page);
    manage_excess_pages_in_stack();
    hits_++;

#ifndef NDEBUG
    std::cerr << "S: ";
    for (auto p : stack) std::cerr << p->id << (p->is_lir ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
    std::cerr << "\n";
#endif //NDEBUG

    return status_t::hit;
}

status_t cache_t::hir_hit(int page_id, page_t* page) {
    assert(page != nullptr);

    push_front_s(page);
    page->is_lir = true;
    lir_count_++;

    if (page->is_in_queue) {
        queue.erase(page->queue_it);
        page->is_in_queue = false;
    }

    if (lir_count_ > max_lir_cap_) {
        demote_lir_to_hir();
    }

    manage_excess_pages_in_stack();
    hits_++;

#ifndef NDEBUG
    std::cerr << "S: ";
    for (auto p : stack) std::cerr << p->id << (p->is_lir ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
    std::cerr << "\n";
#endif //NDEBUG

    return status_t::hit;
}

status_t cache_t::miss_case(int page_id, page_t* page) {
    assert(page != nullptr);

    if (lir_count_ < max_lir_cap_) {  //if LIR-part of stack isn't full yet
        push_front_s(page);
        page->is_lir = true;
        page->is_in_cache = true;
        lir_count_++;

#ifndef NDEBUG
    std::cerr << "MISS: added " << page_id << " to S and Q; queue.size=" << queue.size() << " stack.size=" << stack.size() << "\n";
    std::cerr << "S: ";
    for (auto p : stack) std::cerr << p->id << (p->is_lir ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
    std::cerr << "\n";
#endif

        manage_excess_pages_in_stack();
        return status_t::miss;
    }

    if (page->is_in_stack) {  //if the page was in stack sometime ago and became non-resident HIR after being demoted
        push_front_s(page);
        page->is_lir = true;
        page->is_in_cache = true;
        lir_count_++;

        if (page->is_in_queue) {
            queue.erase(page->queue_it);
            page->is_in_queue = false;
        }

        if (lir_count_ > max_lir_cap_) {
            demote_lir_to_hir();
        }

#ifndef NDEBUG
    std::cerr << "MISS: added " << page_id << " to S and Q; queue.size=" << queue.size() << " stack.size=" << stack.size() << "\n";
    std::cerr << "S: ";
    for (auto p : stack) std::cerr << p->id << (p->is_lir ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
    std::cerr << "\n";
#endif

        manage_excess_pages_in_stack();
        return status_t::miss;
    }

    push_front_s(page);  //if the page is completely new we add it to the stack and promot to HIR
    page->is_lir = false;
    push_front_q(page);

#ifndef NDEBUG
    std::cerr << "MISS: added " << page_id << " to S and Q; queue.size=" << queue.size() << " stack.size=" << stack.size() << "\n";
    std::cerr << "S: ";
    for (auto p : stack) std::cerr << p->id << (p->is_lir ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue) std::cerr << p->id << (p->is_in_cache ? "r " : "nr ");
    std::cerr << "\n";
#endif

    manage_excess_pages_in_stack();
    return status_t::miss;
}

//------------------------------------------------------------


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

    while (static_cast<int>(queue.size()) > max_hir_cap_) {
        page_t* ex = queue.back();
        queue.pop_back();
        ex->is_in_queue = false;
        ex->is_in_cache = false;
    }
}

void cache_t::demote_lir_to_hir() {
    auto it = stack.end();

    while (it != stack.begin()) {
        it--;
        page_t* cand = *it;

        if (cand->is_lir) {
            cand->is_lir      = false;
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
    if (static_cast<int>(queue.size()) > max_hir_cap_) {
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
    page_t* new_page = new page_t{page_id, page_content};

    new_page->is_lir        = false;
    new_page->is_in_cache   = false;

    page_map[page_id] = new_page;

    return new_page;
}

void cache_t::manage_excess_pages_in_stack() {
    while (!stack.empty()
        && !stack.back()->is_lir
        && !stack.back()->is_in_cache) {
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
             << " | is LIR: " << (current_page->is_lir ? "yes" : "no")
             << " | is in cache: " << (current_page->is_in_cache ? "yes" : "no")
             << "\n";
    }

    dump << "\n\n*********** QUEUE ***********\n ";
    for (auto q_it = queue.begin(); q_it != queue.end(); q_it++) {
        page_t* current_page = *q_it;

        dump << "page id: " << current_page->id
             << " | is LIR: " << (current_page->is_lir ? "yes" : "no")
             << " | is in cache: " << (current_page->is_in_cache ? "yes" : "no")
             << "\n";
    }

    dump.close();
    std::cout << "Dump written to file named \"cache_dump\"\n";

    #endif
}
} //namespace lirs
