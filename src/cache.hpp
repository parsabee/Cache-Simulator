/*
 * Author: Parsa Bagheri
 */

#ifndef CACHE_SIM_CACHE_HPP
#define CACHE_SIM_CACHE_HPP

#include <cstddef>
#include <unordered_set>
#include <unordered_map>
#include <string>

#include "memory.hpp"
#include "errors.hpp"
#include "address_translator.hpp"

namespace cs { /* cache simulator */

    enum {
        HIT,
        MISS
    };

    class CacheSet {
    protected:
        int _cap; /* number of cache lines in the set */
        int _size;
        bool _debug;
        std::unordered_map<int, int> _tags; /* the blocks in each set, and number of times its referenced */
    public:
        CacheSet(int num_blocks, int capacity, bool debug)
            : _cap(capacity), _debug(debug), _size(0), _tags(std::unordered_map<int, int>())
        {}

        /*
         * looks up addr in set
         *
         * true if found, false otherwise
         */
        bool has_addr(int tag);

        /*
         * fetches a block with tag `tag',
         *  if tag wasn't found, it's line is brought to the cache
         *  if cache is full, select a victim by victim policy
         * return a bool, true if tag was found, false otherwise
         */
        bool fetch(int tag, int& hits, int& misses, std::unordered_set<int> *dirties = nullptr);
        void insert(int tag);
    protected:
        virtual int select_victim(int& misses);
    };

/*
 * abstract cache type
 */
    class Cache : public Memory {
    protected:
        size_t _total_size, _block_size;
        int _bps; /* blocks per set */
        int _num_sets;
        AddressTranslator *_at;
        int _hits, _misses;
        bool _debug;
        int _hit_time, _miss_penalty;
        /*
         * following is a reference to higher level memory
         * could be main memory or higher level cache
         */
        const Memory *_main_memory;
        CacheSet **_sets;

    public:
        double get_hits() { return (double)_hits;}
        double get_misses() { return (double)_misses;}

        /* cache hit and miss rates */
        double get_hit_rate();
        double get_miss_rate();

        void summary(std::ostream& out);

        virtual int read (const char *addr) = 0;
        virtual int write (const char *addr) = 0;
        virtual std::string type () = 0 ;
        double average_memory_access_time();
        virtual ~Cache();
    protected:
        Cache(size_t total_size, size_t block_size, size_t address_size,
              int blocks_per_set, int hit_time, int miss_penalty,
              const Memory *memory, bool debug);
    };

/*
 * write-through cache system
 */
    class WriteThrough : public Cache {
    public:
        WriteThrough(size_t total_size, size_t block_size, size_t address_size,
                int blocks_per_set, int hit_time, int miss_penalty,
                const Memory *mem = nullptr, bool debug = false)
          :Cache(total_size, block_size, address_size, blocks_per_set, hit_time, miss_penalty, mem, debug) {
            if (debug) {
                std::cerr << "[SUCCESS] write-through cache system initialized\n";
                std::cerr << "================================================\n\n";
            }
        }

        int read (const char *addr) override;
        int write (const char *addr) override;
        std::string type () override { return "WriteThrough"; }
    };

/*
 * write-back cache system
 */
    class WriteBack : public Cache {
        std::unordered_set<int> _dirties;

    public:
        WriteBack(size_t total_size, size_t block_size, size_t address_size,
                     int blocks_per_set, int hit_time, int miss_penalty,
                     const Memory *mem = nullptr, bool debug = false)
                :Cache(total_size, block_size, address_size, blocks_per_set, hit_time, miss_penalty, mem, debug) {
            if (debug) {
                std::cerr << "[SUCCESS] write-back cache system initialized\n";
                std::cerr << "=============================================\n\n";
            }
        }

        int read (const char *addr) override;
        int write (const char *addr) override;
        std::string type () override { return "WriteBack"; }
    };
} /* cs namespace */

#endif /*CACHE_SIM_CACHE_HPP*/
