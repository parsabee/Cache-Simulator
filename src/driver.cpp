/*
 * Definition of driver classes
 * Author: Parsa Bagheri
 */
#include <iostream>
#include "driver.hpp"
#include "errors.hpp"

namespace cs {

    int CacheDriver::L1::exec(int instruction, std::string address) {
        int retval = MISS;
        switch (instruction) {
            case INSTRUCTION_READ:
                retval = i_cache->read(address.c_str());
                break;
            case DATA_READ:
                retval = d_cache->read(address.c_str());
                break;
            case DATA_WRITE:
                retval = d_cache->write(address.c_str());
                break;
            default:
                throw CSException("unknown memory reference");

        }
        return retval;
    }


    double CacheDriver::L1::hit_plus_missrate() {
        double i_hits = i_cache->get_hits();
        double i_misses = i_cache->get_misses();
        double d_hits = d_cache->get_hits();
        double d_misses = d_cache->get_misses();
        double total = i_hits + d_hits + i_misses + d_misses;
        double miss_rate = (i_misses + d_misses)/total;
        return _hit_time + miss_rate;
    }

    void CacheDriver::L1::summary(std::ostream &out) {
        out << "instruction cache summary:\n";
        this->i_cache->summary(out);
        out << "data cache summary:\n";
        this->d_cache->summary(out);
        out << "\n";
    }

    CacheDriver::L1::~L1() {
        delete i_cache;
        delete d_cache;
    }

    CacheDriver::L1::L1(config& configuration)
            : _hit_time(configuration.hit_time), _miss_penalty(configuration.miss_penalty), _debug(configuration.debug) {
        if (configuration.instruction == 0 || configuration.data == 0) {
            throw CSException("instruction or data caches type is not specified -- write_through or write_back");

        } else {
            init(configuration.data, configuration, &d_cache);
            init(configuration.instruction, configuration, &i_cache);
        }
    }

    void CacheDriver::L1::init (int conf, config& configuration, Cache **cache) {
        switch (conf) {
            case write_back:
                *cache = new cs::WriteBack(configuration.total_size, configuration.block_size,
                                           configuration.address_size, configuration.blocks_per_set,
                                           configuration.hit_time, configuration.miss_penalty,
                                           nullptr, configuration.debug);
                break;
            case write_through:
                *cache = new cs::WriteThrough(configuration.total_size, configuration.block_size,
                                              configuration.address_size, configuration.blocks_per_set,
                                              configuration.hit_time, configuration.miss_penalty,
                                              nullptr, configuration.debug);
                break;
            default:
                throw CSException("unknown configuration");
        }
    }

    CacheDriver::L2::L2(config &configuration)
            : _hit_time(configuration.hit_time), _miss_penalty(configuration.miss_penalty) {
        if (configuration.instruction != 0 && configuration.data != 0) {
            throw CSException("Level 2 cache cannot have instruction or data sub-caches");

        } else {
            CacheDriver::L1::init(configuration.data, configuration, &cache);
        }
    }

    CacheDriver::L2::~L2() {
        delete cache;
    }

    int CacheDriver::L2::exec(int instruction, std::string address) {
        int retval = MISS;
        switch (instruction) {
            case INSTRUCTION_READ:
                retval = cache->read(address.c_str());
                break;
            case DATA_READ:
                retval = cache->read(address.c_str());
                break;
            case DATA_WRITE:
                retval = cache->write(address.c_str());
                break;
            default:
                throw CSException("unknown memory reference");
        }
        return retval;
    }

    void CacheDriver::L2::summary(std::ostream &out) {
        this->cache->summary(out);
    }

    double CacheDriver::L2::hit_plus_missrate() {
        double hits = cache->get_hits();
        double misses = cache->get_misses();
        return 10 * _hit_time + 10 * (misses/(hits + misses));
    }

    CacheDriver::CacheDriver (std::vector<config>& configurations) {
        int i = 0;
        for (auto & configuration : configurations) {
            if (i == 0) {
                _levels.push_back(new L1(configuration));
            } else if (i == 1) {
                _levels.push_back(new L2(configuration));
            } else {
                throw CSException("no support for more than 2 levels");
            }
            i++;
        }
    }

    CacheDriver::~CacheDriver() {
        for (auto i : _levels) {
            delete i;
        }
        _levels.clear();
    }

    int CacheDriver::exec(int instruction, std::string address) {
        /* going through every level, breaking once we have a hit */
        int retval = MISS;
        for (auto i : _levels) {
            retval = i->exec(instruction, address);
            if ( retval == HIT )
                break;
        }
        return retval;
    }

    double CacheDriver::AMAT () {
        switch (_levels.size()) {
            case 1:
                return l1_amat();
            case 2:
                return l2_amat();
            default:
                throw CSException("no support for levels beyond 2");
        }
    }

    void CacheDriver::summary(std::ostream &out) {
        out << "Summary of every level:\n\n";
        int l = 1;
        for (auto i : _levels) {
            out << "level " << l++ << "\n";
            i->summary(out);
        }
        out << "overall average memory access time: " << AMAT() << "\n";
    }

    double CacheDriver::l1_amat() {
        L1 *l1 = dynamic_cast<L1 *>(_levels[0]);
        double i_hits = l1->i_cache->get_hits();
        double i_misses = l1->i_cache->get_misses();
        double d_hits = l1->d_cache->get_hits();
        double d_misses = l1->d_cache->get_misses();
        double total = i_hits + d_hits + i_misses + d_misses;
        double miss_rate = (i_misses + d_misses)/total;
        return l1->_hit_time + miss_rate * l1->_miss_penalty;
    }

    double CacheDriver::l2_amat() {
        L1 *l1 = dynamic_cast<L1 *>(_levels[0]);
        double i_hits = l1->i_cache->get_hits();
        double i_misses = l1->i_cache->get_misses();
        double d_hits = l1->d_cache->get_hits();
        double d_misses = l1->d_cache->get_misses();
        double l1_miss_rate = (i_misses + d_misses)/(i_misses + d_misses + i_hits + d_hits);
        L2 *l2 = dynamic_cast<L2 *>(_levels[1]);
        double l2_miss_rate = l2->cache->get_miss_rate();
        return l1->_hit_time + l1_miss_rate * (l2->_hit_time + l2_miss_rate * l2->_miss_penalty);
    }
}