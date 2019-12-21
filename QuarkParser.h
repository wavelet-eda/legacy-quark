#pragma once

#include <functional>
#include <stdint.h>
#include <iostream>
#include <exception>
#include <vector>
#include <set>
// #include <gmp.h>
#include <gmpxx.h>

#include "Quark.h"

namespace Quark {

    class QuarkParserException : public std::exception {
    private:
        const char* msg;
        size_t line, col;
        
    public:

        QuarkParserException(const char* msg) : 
                std::exception(), msg(msg), line(0), col(0) {}

        QuarkParserException(const char* msg, size_t line, size_t col) : 
                std::exception(), msg(msg), line(line), col(col) {}

        QuarkParserException(QuarkException e, size_t line, size_t col) :
                std::exception(), msg(e.what()), line(line), col(col) {}

        size_t get_line() const { return line; }
        size_t get_col() const { return col; }

        const char * what () const throw () { return msg; }
    };

    class QuarkParser {
    private:

    public:

        QuarkParser();
        ~QuarkParser() {};
        QuarkNode parse();
    };
}