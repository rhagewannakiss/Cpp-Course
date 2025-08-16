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
    std::cerr << "accessing " << page_id << " page ptr=" << page << " in_cache=" << page->is_in_cache_ << "\n";
#endif // NDEBUG

    return (page->is_in_cache_) ? ((page->is_lir_)? lir_hit(page) : hir_hit(page)) : miss_case(page_id, page);
}

//-------------------- HIT & MISS FUNC. -------------------------
status_t cache_t::lir_hit(page_t* page) {
    assert(page != nullptr);

    push_front_s(page);
    manage_excess_pages_in_stack();
    hits_++;

#ifndef NDEBUG
    std::cerr << "S: ";
    for (auto p : stack_) std::cerr << p->id_ << (p->is_lir_ ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue_) std::cerr << p->id_ << (p->is_in_cache_ ? "r " : "nr ");
    std::cerr << "\n";
#endif // NDEBUG

    return status_t::hit;
}

status_t cache_t::hir_hit(page_t* page) {
    assert(page != nullptr);

    push_front_s(page);
    page->is_lir_ = true;
    lir_count_++;

    if (page->is_in_queue_) {
        queue_.erase(page->queue_it_);
        page->is_in_queue_ = false;
    }

    if (lir_count_ > max_lir_cap_) {
        demote_lir_to_hir();
    }

    manage_excess_pages_in_stack();
    hits_++;

#ifndef NDEBUG
    std::cerr << "S: ";
    for (auto p : stack_) std::cerr << p->id_ << (p->is_lir_ ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue_) std::cerr << p->id_ << (p->is_in_cache_ ? "r " : "nr ");
    std::cerr << "\n";
#endif // NDEBUG

    return status_t::hit;
}

status_t cache_t::miss_case(int page_id, page_t* page) {
    assert(page != nullptr);

    if (lir_count_ < max_lir_cap_) {  // if LIR-part of stack_ isn't full yet
        push_front_s(page);
        page->is_lir_ = true;
        page->is_in_cache_ = true;
        lir_count_++;

#ifndef NDEBUG
        std::cerr << "MISS: added " << page_id << " to S and Q; queue_.size=" << queue_.size() << " stack_.size=" << stack_.size() << "\n";
        std::cerr << "S: ";
        for (auto p : stack_) std::cerr << p->id_ << (p->is_lir_ ? "L " : "H ");
        std::cerr << " | Q: ";
        for (auto p : queue_) std::cerr << p->id_ << (p->is_in_cache_ ? "r " : "nr ");
        std::cerr << "\n";
#endif

        manage_excess_pages_in_stack();
        return status_t::miss;
    }

    if (page->is_in_stack_) {  // if the page was in stack_ sometime ago and became non-resident HIR after being demoted
        push_front_s(page);
        page->is_lir_ = true;
        page->is_in_cache_ = true;
        lir_count_++;

        if (page->is_in_queue_) {
            queue_.erase(page->queue_it_);
            page->is_in_queue_ = false;
        }

        if (lir_count_ > max_lir_cap_) {
            demote_lir_to_hir();
        }

#ifndef NDEBUG
        std::cerr << "MISS: added " << page_id << " to S and Q; queue_.size=" << queue_.size() << " stack_.size=" << stack_.size() << "\n";
        std::cerr << "S: ";
        for (auto p : stack_) std::cerr << p->id_ << (p->is_lir_ ? "L " : "H ");
        std::cerr << " | Q: ";
        for (auto p : queue_) std::cerr << p->id_ << (p->is_in_cache_ ? "r " : "nr ");
        std::cerr << "\n";
#endif

        manage_excess_pages_in_stack();
        return status_t::miss;
    }

    push_front_s(page);  // if the page is completely new we add it to the stack_ and promot to HIR
    page->is_lir_ = false;
    push_front_q(page);

#ifndef NDEBUG
    std::cerr << "MISS: added " << page_id << " to S and Q; queue_.size=" << queue_.size() << " stack_.size=" << stack_.size() << "\n";
    std::cerr << "S: ";
    for (auto p : stack_) std::cerr << p->id_ << (p->is_lir_ ? "L " : "H ");
    std::cerr << " | Q: ";
    for (auto p : queue_) std::cerr << p->id_ << (p->is_in_cache_ ? "r " : "nr ");
    std::cerr << "\n";
#endif

    manage_excess_pages_in_stack();
    return status_t::miss;
}

//------------------------------------------------------------


void cache_t::push_front_s(page_t* page) {
    assert(page != nullptr);

    if (page->is_in_stack_) {
        stack_.erase(page->stack_it_);
    }

    stack_.push_front(page);
    page->stack_it_ = stack_.begin();

    page->is_in_stack_ = true;
}

void cache_t::push_front_q(page_t* page) {
    assert(page != nullptr);

    if (page->is_in_queue_) {
        queue_.erase(page->queue_it_);
    }

    queue_.push_front(page);
    page->queue_it_    = queue_.begin();
    page->is_in_queue_ = true;
    page->is_in_cache_ = true;

    while (static_cast<int>(queue_.size()) > max_hir_cap_) {
        page_t* ex = queue_.back();
        queue_.pop_back();
        ex->is_in_queue_ = false;
        ex->is_in_cache_ = false;
    }
}

void cache_t::demote_lir_to_hir() {
    auto it = stack_.end();

    while (it != stack_.begin()) {
        it--;
        page_t* cand = *it;

        if (cand->is_lir_) {
            cand->is_lir_      = false;
            cand->is_in_stack_ = false;

            queue_.push_front(cand);
            cand->queue_it_ = queue_.begin();
            cand->is_in_queue_ = true;

            manage_queue();

            break;
        }
    }
}

void cache_t::manage_queue() {
    if (static_cast<int>(queue_.size()) > max_hir_cap_) {
        page_t* bye_bitch = queue_.back();
        queue_.pop_back();

        bye_bitch->is_in_cache_ = false;
        bye_bitch->is_in_queue_ = false;
     }
}

cache_t::page_t* cache_t::get_page(int page_id) {
    auto tmp = page_map_.find(page_id);

    if (tmp != page_map_.end()) {
        return tmp->second;
    }

    int page_content = slow_get_page_int(page_id);
    page_t* new_page = new page_t{page_id, page_content};

    new_page->is_lir_        = false;
    new_page->is_in_cache_   = false;

    page_map_[page_id] = new_page;

    return new_page;
}

void cache_t::manage_excess_pages_in_stack() {
    while (!stack_.empty()
        && !stack_.back()->is_lir_
        && !stack_.back()->is_in_cache_) {
            page_t* last = stack_.back();
            stack_.pop_back();
            last->is_in_stack_ = false;
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

    dump << "*********** stack_ ***********\n ";
    for (auto s_it = stack_.begin(); s_it != stack_.end(); s_it++) {
        page_t* current_page = *s_it;

        dump << "page id_: " << current_page->id_
             << " | is LIR: " << (current_page->is_lir_ ? "yes" : "no")
             << " | is in cache: " << (current_page->is_in_cache_ ? "yes" : "no")
             << "\n";
    }

    dump << "\n\n*********** queue_ ***********\n ";
    for (auto q_it = queue_.begin(); q_it != queue_.end(); q_it++) {
        page_t* current_page = *q_it;

        dump << "page id_: " << current_page->id_
             << " | is LIR: " << (current_page->is_lir_ ? "yes" : "no")
             << " | is in cache: " << (current_page->is_in_cache_ ? "yes" : "no")
             << "\n";
    }

    dump.close();
    std::cout << "Dump written to file named \"cache_dump\"\n";

#endif // NDEBUG
}
} // namespace lirs