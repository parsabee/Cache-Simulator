/*
 * Address translator,
 * convert hex to binary and extract tag, index and offset bits
 * 
 * Author: Parsa Bagheri
 */

#ifndef CACHE_SIM_ADDRESS_TRANSLATOR_HPP
#define CACHE_SIM_ADDRESS_TRANSLATOR_HPP

#include "errors.hpp"

namespace cs {

/*
 * struct containing tag, set, and offset of an address
 */
    struct Addr {
        const int tag, set, offset;
        Addr(int tag, int set, int offset): tag(tag), set(set), offset(offset) {}
    };

    class AddressTranslator {

        /* constructor parameters */
        int cache_size;
        int address_size;
        int block_size;
        int num_sets;
        int blocks_per_set;
        bool debug;

        int num_tag_bits;
        int num_index_bits;
        int num_offset_bits;

    public:
    /*
     * Constructor for AddressTranslator object
     * throws AddressTranslation exception when inconsistent arguments are passed
     */
        AddressTranslator(int cache_size, unsigned int address_size, int block_size,
                          int num_sets, int blocks_per_set, bool debug = false);

    /*
     * the only public method of AddressTranslator
     * takes a hex string address and returns an Addr object
     */
        Addr translate (const char *address);

    private:
    /*
     * convert a hex string to a binary string
     *
     * bit_array is an empty array passed as an argument, which will be filled
     * hex_address is the hex string that we want to convert.
     */
        void get_bit_array(char *bit_array, const char *hex_address);

    /*
     * converts binary to decimal
     */
        int bin_to_int(char *binary);
    };

}
#endif //CACHE_SIM_ADDRESS_TRANSLATOR_HPP
