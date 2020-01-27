#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <getopt.h> /* getopt() */
#include "driver.hpp"
#include "errors.hpp"

void usage() {
    std::cerr << "usage: cache-sim [-hvd] -i input-file -c config-level -s associativity\n";
}

void version() {
    std::cerr << "chache-sim version 1.0.0\n";
}

void help () {
    std::cerr << "usage: cache-sim [-hvd] -i input-file -c config-level -s associativity\n\n";
    std::cerr << "options:\n";
    std::cerr << "  -c, --config             configuration level: 1 | 2 | 3\n";
    std::cerr << "  -s, --associativity      set associativity: divisible by 2\n";
    std::cerr << "  -i, --input              input trace file\n";
    std::cerr << "  -d, --debug\n";
    std::cerr << "  -h, --help\n";
    std::cerr << "  -v, --version\n";
}

int main(int argc, char *argv[]) {
    int status = 1;
    bool debug = false;
    try {

    /*
     * parsing options
     */
        std::ifstream in;
        std::string config, set = "";

        static struct option longopts[] {
                { "config", required_argument, nullptr, 'c'},
                { "associativity", required_argument, nullptr, 's'},
                { "input", required_argument, nullptr, 'i'},
                { "debug", no_argument, nullptr, 'd'},
                { "help", no_argument, nullptr, 'h'},
                { "version", no_argument, nullptr, 'v'},
                {nullptr, 0, nullptr, 0}
        };

        int ch;
        while ((ch = getopt_long(argc, argv, "hvds:c:i:", longopts, nullptr)) != -1) {
            switch (ch) {
                case 'c':
                    config = optarg;
                    break;
                case 's':
                    set = optarg;
                    break;
                case 'i':
                    in.open(optarg);
                    break;
                case 'd':
                    debug = true;
                    break;
                case 'v':
                    version();
                    exit(0);
                case 'h':
                    help();
                    exit(0);
                default:
                    usage();
                    exit(1);
            }
        }

        if (!in.is_open()) {
            throw CSException("invalid input file");
        }

        if (set == "") {
            throw CSException("invalid set associativity");
        }

        std::vector<cs::config> configs;
        if (config == "1") {
            int num_sets = std::stoi(set, 0);
            configs = {
                    {cs::write_through, cs::write_through, 1024, 32, 32, 1, 100, num_sets, debug}
            };
        } else if (config == "2") {
            int num_sets = std::stoi(set, 0);
            configs = {
                    {cs::write_back, cs::write_back, 1024, 32, 32, 1, 100, num_sets, debug}
            };
        } else if (config == "3") {
            int num_sets = std::stoi(set, 0);
            configs = {
                    {cs::write_back, cs::write_back, 1024, 32, 32, 1, 100, 2, debug},
                    {0,              cs::write_back, 16384, 128, 32, 1, 100, num_sets, debug}
            };
        } else {
            throw CSException("invalid configuration");
        }

        /*
         * creating cache driver
         */
        cs::CacheDriver cache_wt(configs);

        std::string line;
        while (getline(in, line)) {
            std::string type = line.substr(0, line.find(' '));
            std::string addr = line.substr(line.find(' ') + 1);
            if (type == "" || addr == "") {
                std::string str = "invalid line -- " + line;
                throw CSException(str.c_str());
            }

            if (type == "0") {
                if (debug) {
                    std::cerr << "[data read] "<< addr << "\n";
                }
                (void) cache_wt.exec(0, addr);

            } else if (type == "1") {
                if (debug) {
                    std::cerr << "[data write] " << addr << "\n";
                }
                (void) cache_wt.exec(1, addr);

            } else if (type == "2") {
                if (debug) {
                    std::cerr << "[instruction read] " << addr << "\n";
                }
                (void) cache_wt.exec(2, addr);
            }
        }
        cache_wt.summary(std::cout);
        status = 0;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
    }
    exit(status);
}
