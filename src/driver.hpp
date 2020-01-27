/*
 * L1 classes for different cache systems
 * Author: Parsa Bagheri
 */

#ifndef CACHE_SIM_DRIVER_HPP
#define CACHE_SIM_DRIVER_HPP

#include <ostream>
#include <vector>
#include <array>
#include "cache.hpp"

namespace cs {

    enum cache_type {
        write_back = 255,
        write_through
    };

    /*
     * configuration struct
     * used to configure one single cache
     */
    struct config {
        int instruction; /* write_back or write_through */
        int data; /* write_back or write_through */
        size_t total_size;
        size_t block_size;
        size_t address_size;
        int hit_time;
        int miss_penalty;
        int blocks_per_set;
        bool debug;
    };

    enum {
        DATA_READ,
        DATA_WRITE,
        INSTRUCTION_READ
    };

    class BaseCacheDriver {
    public:
        virtual int exec(int instruction, std::string address) = 0;
        virtual void summary(std::ostream &out) = 0;
        virtual ~BaseCacheDriver() = default;
    };


    /*
     * A multi-level cache driver
     * takes an array of config structs, each entry corresponds to a level in cache
     * with configuration[0] being level 1, configuration[1] level 2 and so on
     */
    class CacheDriver : BaseCacheDriver {
        /*
         * driver for caches on each level
         * takes a config struct to initialize instruction and data internal caches
         */
        class Driver : public BaseCacheDriver {
        public:
            virtual double hit_plus_missrate() = 0;
            virtual double get_miss_penalty() = 0;
        };

        class L1 : public Driver {
        friend class CacheDriver;
            cs::Cache *i_cache;
            cs::Cache *d_cache;
            int _hit_time, _miss_penalty;
            bool _debug;
        public:
            explicit L1(config& configuration);
            ~L1() override ;

            int exec(int instruction, std::string address) override ;
            double hit_plus_missrate () override;
            double get_miss_penalty() override { return _miss_penalty; }
            void summary(std::ostream &out) override ;
            static void init (int conf, config& configuration, Cache **cache);
        };

        class L2 : public Driver {
        friend class CacheDriver;
            cs::Cache *cache;
            int _hit_time, _miss_penalty;
            bool _debug;
        public:
            explicit L2(config& configuration);
            ~L2() override ;

            double hit_plus_missrate () override;
            double get_miss_penalty() override { return _miss_penalty; }
            int exec(int instruction, std::string address) override ;
            void summary(std::ostream &out) override ;
        };


        std::vector<Driver *>_levels;
    public:

        explicit CacheDriver (std::vector<config>&);
        ~CacheDriver () override ;
        int exec(int instruction, std::string address) override ;
        double AMAT ();
        void summary(std::ostream &out) override ;
    private:
        double l1_amat();
        double l2_amat();
    };

}

#endif //CACHE_SIM_DRIVER_HPP
