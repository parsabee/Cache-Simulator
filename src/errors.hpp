//
// Created by Parsa Bagheri on 1/19/20.
//

#ifndef CACHE_SIM_ERRORS_HPP
#define CACHE_SIM_ERRORS_HPP
#include <exception>

struct CSException : public std::exception
{
protected:
    const char *error;
public:
    CSException(const char *error = "CacheException") : error(error) {}
    const char * what() const throw() {
        return error;
    }
};

struct AddressTranslation : public CSException {
    AddressTranslation () : CSException("AddressTranslation") {}
};

struct AddressExists : public CSException {
    AddressExists () : CSException("AddressExists") {}
};

#endif //CACHE_SIM_ERRORS_HPP
