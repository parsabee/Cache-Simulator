/*
 * Author: Parsa Bagheri
 */

#include <iostream>
#include "cache.hpp"

namespace cs {

    bool CacheSet::fetch(int tag, int& hits, int& misses, std::unordered_set<int> *dirties) {

        bool has = false;
        if (!has_addr(tag)) {
            if (_size >= _cap) {
                int victim = select_victim(misses);
                if (dirties != nullptr) {
                    if (dirties->count(victim) != 0) {
                        if (_debug)
                            std::cerr << "     miss -- victim was dirty -- writing back to memory\n";
                        misses++;
                    }
                }
                _tags.erase(_tags.find(victim));
                _size--;
            }
            _tags.insert({tag, 1});
            _size++;
        } else {
            _tags.at(tag)++;
            has = true;
        }
        return has;
    }

    void CacheSet::insert(int tag) {
        if (has_addr(tag)) {
            _tags.at(tag)++;
        }
    }

    int CacheSet::select_victim(int& misses) {
        int min_freq = INT32_MAX;
        int key = _tags.begin()->first;
        for (auto it: _tags) {
            if (it.second < min_freq) {
                min_freq = it.second;
                key = it.first;
            }
        }
        if (_debug)
            std::cerr << "cache full -- victim selected by lru :   " << key << "  #ref: " << min_freq << "\n";
        return key;
    }

    bool CacheSet::has_addr(int tag) {
        return _tags.count(tag) != 0;
    }

    double Cache::get_hit_rate() {
        if (_hits + _misses == 0)
            return 0.0;
        return ((double)_hits)/(double)(_hits + _misses);
    }

    double Cache::get_miss_rate() {
        if (_hits + _misses == 0)
            return 0.0;
        return ((double)_misses)/(double)(_hits + _misses);
    }

    double Cache::average_memory_access_time() {
        return (double) _hit_time + (get_miss_rate() * (double) _miss_penalty);
    }

    void Cache::summary(std::ostream& out) {
        out << "summary of " << type() << " cache:\n";
        out << "  number of memory accesses: " << _misses + _hits << "\n";
        out << "  number of hits: " << _hits << "\n";
        out << "  number of misses: " << _misses << "\n";
        out << "  hit rate: " << get_hit_rate() << "\n";
        out << "  miss rate: " << get_miss_rate() << "\n";
        out << "\n";
    }

    Cache::Cache(size_t total_size,
                 size_t block_size,
                 size_t address_size,
                 int blocks_per_set,
                 int hit_time,
                 int miss_penalty,
                 const Memory *memory,
                 bool debug)
        : _total_size(total_size), _block_size(block_size), _num_sets(int(total_size/(block_size*blocks_per_set))),
          _bps(blocks_per_set), _hit_time(hit_time), _miss_penalty(miss_penalty),
          _main_memory(memory), _hits(0), _misses(0), _debug(debug){

        if (_debug) {
            std::cerr << "======[ initializing cache ]======\n"
                      << "total size = " << _total_size << "B\n"
                      << "block size = " << _block_size << "B\n"
                      << "set size: " << blocks_per_set * block_size << "B\n"
                      << "number of sets = " << _num_sets << "\n"
                      << "number of blocks per set = " << _bps << "\n\n";
        }

    /*
     * creating address translator
     */
        _at = new AddressTranslator(total_size,address_size,block_size,
                                    _num_sets,_bps,_debug);

    /*
     * creating our sets
     */
        _sets = new CacheSet*[_num_sets];
        for (int i = 0; i < _num_sets; i++)
            _sets[i] = new CacheSet(_bps, _block_size, _debug);
    }

    Cache::~Cache() {
        for (int i = 0; i < _num_sets; i++)
            delete _sets[i];
        delete [] _sets;
        delete _at;
    }

    int WriteThrough::read (const char *addr) {
        Addr address = _at->translate(addr);
        if (_sets[address.set]->fetch(address.tag, _hits, _misses)) {
            if (_debug)
                std::cerr << "     read hit\n\n";
            _hits++;
            return HIT;
        } else {
            if (_debug)
                std::cerr << "     read miss\n\n";
            _sets[address.set]->insert(address.tag);
            _misses++;
            return MISS;
        }
    }

    int WriteThrough::write (const char *addr) {
        Addr address = _at->translate(addr);
        if (_sets[address.set]->fetch(address.tag, _hits, _misses)) {
            if (_debug)
                std::cerr << "     write hit\n";
            _hits++;
            if (_debug)
                std::cerr << "     write miss -- writing through\n\n";
            _misses++; /* we are also writing through to the memory */
            return MISS;
        } else {
            if (_debug)
                std::cerr << "     write miss -- writing through\n\n";
            _misses++;
            return MISS;
        }
    }

    int WriteBack::read (const char *addr) {
        Addr address = _at->translate(addr);
        if (_sets[address.set]->fetch(address.tag, _hits, _misses, &_dirties)) {
            if (_debug)
                std::cerr << "     read hit\n\n";
            _hits++;
            return HIT;
        } else {
            if (_debug)
                std::cerr << "     read miss\n\n";
            _sets[address.set]->insert(address.tag);  /* up the reference count*/
            _misses++;
            return MISS;
        }
    }

    int WriteBack::write (const char *addr) {
        Addr address = _at->translate(addr);
        if (_sets[address.set]->fetch(address.tag, _hits, _misses, &_dirties)) {
            if (_debug)
                std::cerr << "     write hit -- write back --  " << address.tag << " set dirty\n\n";
            _hits++;
            if (_dirties.count(address.tag) != 0)
                _dirties.insert(address.tag); /* marking this block as dirty */
            return HIT;
        } else {
            /*
             * We're not bringing new cache line in when there's a write miss,
             * we just write main memory -- no write allocate
             */
            if (_debug)
                std::cerr << "     write miss -- write allocate\n\n";
            _misses++;
            if (_dirties.count(address.tag) != 0)
                _dirties.insert(address.tag); /* marking this block as dirty */
            _sets[address.set]->insert(address.tag); /* up the reference count*/
            return MISS;
        }
    }
}