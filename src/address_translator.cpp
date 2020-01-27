/*
 * Address translator definition
 * Author: Parsa Bagheri
 */

#include "address_translator.hpp"
#include <cmath>
#include <sstream>
#include <iostream>

namespace cs {

    AddressTranslator::AddressTranslator(int cache_size, unsigned int address_size, int block_size,
            int num_sets, int blocks_per_set, bool debug) :
            cache_size(cache_size), address_size(address_size), block_size(block_size),
            num_sets(num_sets), blocks_per_set(blocks_per_set), debug(debug) {
        
        if (debug) {
            std::cerr << "======[ address translator ]======\n";
            std::cerr << "cache size = " << cache_size << "B\n"
                      << "block size = " << block_size << "B\n"
                      << "address size = " << address_size << "-bit\n\n";
        }

        /* error checking */
        if (block_size > cache_size) {
            if (debug)
                std::cerr << "error: block_size > cache_size\n";
            throw AddressTranslation();
        }
            
        if (cache_size % block_size) {
            if (debug)
                std::cerr << "error: cache_size % block_size != 0\n";
            throw AddressTranslation();
        }

        if (blocks_per_set * block_size * num_sets != cache_size) {
            if (debug)
                std::cerr << "error: blocks_per_set * block_size * num_sets != cache_size\n";
            throw AddressTranslation();
        }

        num_offset_bits = static_cast<int>(log2(static_cast<double>(block_size)));
        num_index_bits = static_cast<int>(log2(static_cast<double>(num_sets)));
        num_tag_bits = address_size - num_offset_bits - num_index_bits;
        if (num_tag_bits <= 0) {
            if (debug)
                std::cerr << "error: address size invalid\n";
            throw AddressTranslation();
        }

        if (debug) {
            std::cerr << "number of offset bits: " << num_offset_bits << "\n"
                      << "number of index bits: "  << num_index_bits << "\n"
                      << "number of tag bits: " << num_tag_bits << "\n\n";
        }
        
    }

    Addr AddressTranslator::translate (const char *hex) {

        if (debug)
            std::cerr << "translating: " << hex << "\n";

        int offset; /* if address starts with 0x or 0X offset it by two chars */
        if (strncmp(hex, "0x", 2) == 0) {
            offset = 2;
        } else if (strncmp(hex, "0X", 2) == 0) {
            offset = 2;
        } else {
            offset = 0;
        }

        int i = 0, j;
        char hex_address[(address_size / 4)+ 1];
        memset(hex_address, 0, (address_size / 4)+ 1);

        int hex_len = strlen(hex + offset);
        if ((hex_len * 4) > address_size) {
            if (debug) {
                std::cerr << "error: address size doesn't match the address provided!\n";
                std::cerr << "address size: " << address_size << "-bit != " << " address provided: " << hex_len * 4 << "-bit\n";
            }
            throw AddressTranslation();
        } else if ((hex_len * 4) < address_size) {
            for (i = 0; i < (address_size / 4)-hex_len; i++)
                hex_address[i] = '0';
        }


        for (j = offset; j < hex_len; j++, i++)
            hex_address[i] = hex[j];

        char bit_array[address_size + 1];
        memset(bit_array, 0, address_size + 1);
        get_bit_array(bit_array, hex_address);
        
        char tag_bin[num_tag_bits + 1];
        for (i = 0, j = 0; j < num_tag_bits; i++, j++) {
            tag_bin[j] = bit_array[i];
        }
        tag_bin[j] = '\0';

        int tag = bin_to_int(tag_bin);
        char index_bin[num_index_bits + 1];
        for (j = 0; j < num_index_bits; i++, j++)
            index_bin[j] = bit_array[i];
        index_bin[j] = '\0';

        int set = bin_to_int(index_bin);
        char offset_bin[num_offset_bits + 1];
        for (j = 0; j < num_offset_bits; i++, j++)
            offset_bin[j] = bit_array[i];
        offset_bin[j] = '\0';
        int offset_bits = bin_to_int(offset_bin);
        
        if (debug) {
            std::cerr << "tag: " << tag << ", index: " << set << ", offset: " << offset_bits << "\n";
        }
        
        return Addr(tag, set, offset_bits);
    }

    int AddressTranslator::bin_to_int(char *binary) {

        int len = strlen(binary);
        double decimal = 0.0;
        int index, i;
        for (index = len-1, i = 0; index >= 0; index--, i++) {
            if (binary[index] == '0') {
                continue;
            } else if (binary[index] == '1') {
                decimal += pow(2.0, static_cast<double>(i));
            } else {
                throw AddressTranslation();
            }
        }
        return static_cast<int>(decimal);
    }

    void AddressTranslator::get_bit_array(char *bit_array, const char *hex_address) {

        int i = 0;
        for (; hex_address[i] != '\0'; i++) {
            char ch = hex_address[i];
            switch (ch) {
                case '0':
                    strcat(bit_array, "0000");
                    break;
                case '1':
                    strcat(bit_array, "0001");
                    break;
                case '2':
                    strcat(bit_array, "0010");
                    break;
                case '3':
                    strcat(bit_array, "0011");
                    break;
                case '4':
                    strcat(bit_array, "0100");
                    break;
                case '5':
                    strcat(bit_array, "0101");
                    break;
                case '6':
                    strcat(bit_array, "0110");
                    break;
                case '7':
                    strcat(bit_array, "0111");
                    break;
                case '8':
                    strcat(bit_array, "1000");
                    break;
                case '9':
                    strcat(bit_array, "1001");
                    break;
                case 'a':
                    strcat(bit_array, "1010");
                    break;
                case 'b':
                    strcat(bit_array, "1011");
                    break;
                case 'c':
                    strcat(bit_array, "1100");
                    break;
                case 'd':
                    strcat(bit_array, "1101");
                    break;
                case 'e':
                    strcat(bit_array, "1110");
                    break;
                case 'f':
                    strcat(bit_array, "1111");
                    break;
                case 'A':
                    strcat(bit_array, "1010");
                    break;
                case 'B':
                    strcat(bit_array, "1011");
                    break;
                case 'C':
                    strcat(bit_array, "1100");
                    break;
                case 'D':
                    strcat(bit_array, "1101");
                    break;
                case 'E':
                    strcat(bit_array, "1110");
                    break;
                case 'F':
                    strcat(bit_array, "1111");
                    break;
                default:
                    throw AddressTranslation();
            }
        }
    }
}