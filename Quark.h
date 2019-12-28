#pragma once

#include <functional>
#include <stdint.h>
#include <iostream>
#include <exception>
#include <vector>
#include <set>
#include <gmpxx.h>

#define likely(x)      __builtin_expect(!!(x), 1) 
#define unlikely(x)    __builtin_expect(!!(x), 0) 

namespace Quark {

    std::set<std::string> get_quark_keywords();

    const char QUARK_ASCII_VT = 11;
    const char QUARK_ASCII_FF = 12;
    const char QUARK_ASCII_CR = 13;
    const char QUARK_ASCII_DEL = 127;

    enum class QuarkSymbol : uint8_t {
        ASSIGN = '=',
        ADDITION = '+', 
        SUBTRACTION = '-', 
        MULTIPLICATION = '*', 
        DIVISION = '/', 
        MODULUS = '%',
        BITWISE_AND = '&', 
        BITWISE_NAND = 128, // ~&
        BITWISE_OR = '|',
        BITWISE_NOR = 129, // ~|
        BITWISE_XOR = '^', 
        BITWISE_XNOR = 130, // ~^
        LSHIFT = 131, // <<
        RSHIFT = 132, // >>
        ADDITION_ASSIGN = 133, // += 
        SUBTRACTION_ASSIGN = 134, // -=
        MULTIPLICATION_ASSIGN = 135, // *=
        DIVISION_ASSIGN = 136, // /=
        MODULUS_ASSIGN = 137, // %=
        BITWISE_AND_ASSIGN = 138,  // &=
        BITWISE_OR_ASSIGN = 139, // |=
        BITWISE_XOR_ASSIGN = 140, // ^=
        LSHIFT_ASSIGN = 196, // <<=
        RSHIFT_ASSIGN = 197, // >>=
        COND_AND = 141, // &&
        COND_OR = 142, // ||
        COND_IMPLICATION = 143, // ->
        COND_EQUIVALENCE = 198, // <->
        LESS = '<', 
        GREATER = '>', 
        LESS_EQUAL = 144, // <=
        GREATER_EQUAL = 145, // >=
        EQUAL = 146, // ==
        NOT_EQUAL = 147, // !=
        INCREMENT = 148, // ++
        DECREMENT = 149, // --
        INVERT = '~', 
        BANG = '!',
        LPAREN = '(', 
        RPAREN = ')', 
        LRECTANGLE = '[', 
        RRECTANGLE = ']', 
        LBRACE = '{', 
        RBRACE = '}',
        QUESTION = '?',
        COLON = ':',
        DOT = '.',
        COMMA = ',',
        DOUBLE_COLON = 150, // ::
        SEMICOLON = ';',
        // Unsupported/unused symbols
        DOLLAR = '$',
        POUND = '#', 
        TICK = '`',
        AT = '@',
        BACKSLASH = '\\',
        // Comment Symbols
        LINE_COMMENT = 151, // //
        BLOCK_COMMENT = 152, // /*
        // Empty symbol
        WHITESPACE = ' '
    };

    enum class QuarkCharacterType : uint8_t {
        CHAR_EOF = 0,
        NEWLINE = 1, 
        WHITESPACE = 2, 
        SYMBOL = 4, 
        ALPHA = 5, 
        NUMERAL = 6,
        UNDEF = 7
    };

    class QuarkCharacter {
    private:
        char c;
        QuarkCharacterType ctype;

    public:
        QuarkCharacter() : c(0), ctype(QuarkCharacterType::UNDEF) {}

        QuarkCharacter(bool eof) : 
                c('\n'), ctype(eof ? QuarkCharacterType::CHAR_EOF : 
                    QuarkCharacterType::NEWLINE) {}

        QuarkCharacter(char c, QuarkCharacterType ctype) :
                c(c), ctype(ctype) {}

        char get_char() {
            return c;
        }

        QuarkCharacterType get_type() {
            return ctype;
        }

        bool is_whitespace() {
            return (((int) ctype) >> 2) == 0;
        }

        bool is_symbol() {
            return ctype == QuarkCharacterType::SYMBOL;
        }

        bool is_line_finish() {
            return ((int) ctype) < 2;
        }

        bool is_eof() {
            return ctype == QuarkCharacterType::CHAR_EOF;
        }

        unsigned int decode_base() {
            switch (c) {
                case 'b': return 2;
                case 'o': return 8;
                case 'd': return 10;
                case 'h': return 16;
                default: return 0;
            }
        }

        int get_numeric_value(int base);
        char get_escaped_char();
    };

    class QuarkException : public std::exception {
    private:
        const char* msg;
        QuarkCharacter qc;

    public:

        QuarkException(const char* msg) : 
            std::exception(), msg(msg) {}
        QuarkException(const char* msg, QuarkCharacter qc) : 
            std::exception(), msg(msg), qc(qc) {}

        QuarkCharacter get_char() const { return qc; }

        const char * what () const throw () { return msg; }
    };

    struct QuarkNumber {
        unsigned int base, width;
        mpz_class num;
        bool is_signed, is_ambiguous;

        QuarkNumber() : 
                base(10), width(0), num(0),
                is_signed(false), is_ambiguous(true) {}

        QuarkNumber(unsigned int base) : 
                base(base), width(0), num(0),
                is_signed(false), is_ambiguous(true) {}

        QuarkNumber(unsigned int base, unsigned int width) : 
                base(base), width(width), num(0),
                is_signed(false), is_ambiguous(true) {}

        QuarkNumber(unsigned int base, unsigned int width, unsigned int value) : 
                base(base), width(width), num(value),
                is_signed(false), is_ambiguous(false) {

            if (mpz_sizeinbase(num.get_mpz_t(), 2) > width)
                throw QuarkException("Number larger than defined bit-width!");
        }

        void insert_digit(QuarkCharacter qc) {
            int digit = qc.get_numeric_value(base);
            
            if (digit < 0)
                throw QuarkException("Invalid digit used in number!", qc);
            
            num *= base;
            num += digit;

            if (not is_ambiguous and mpz_sizeinbase(num.get_mpz_t(), 2) > width)
                throw QuarkException("Number larger than defined bit-width!", qc);
        }

        void insert_digit(unsigned int digit) {
            if (digit >= base)
                throw QuarkException("Invalid digit used in number!", QuarkCharacter());
            
            num *= base;
            num += digit;

            if (not is_ambiguous and mpz_sizeinbase(num.get_mpz_t(), 2) > width)
                throw QuarkException("Number larger than defined bit-width!", QuarkCharacter());
        }

        void convert_fixed_width(unsigned int base, bool is_signed) {
            if (not num.fits_uint_p())
                throw QuarkException("Constant width takes more than 32-bits!");

            this->is_signed = is_signed;
            this->base = base;
            is_ambiguous = false;
            width = num.get_ui();
            num = 0;
        }

        int get_width() {
            return is_ambiguous ? mpz_sizeinbase(num.get_mpz_t(), 2) : width;
        }
    };

    enum class QuarkTokenType : uint8_t {
        KEYWORD,
        IDENTIFIER,
        SYMBOL,
        STRING,
        DOC_COMMENT,
        NUMBER,
        NUMBER_WILDCARD,
        NUMBER_FLOAT,
        NUMBER_DOUBLE
    };

    struct QuarkToken {
        QuarkTokenType type;
        size_t line, col;
        QuarkNumber num;
        QuarkNumber num_mask;
        QuarkSymbol symbol;
        size_t id;
        std::string str;
        float f;
        double d;

        QuarkToken(size_t line, size_t col, QuarkNumber num) :
                line(line), col(col),
                type(QuarkTokenType::NUMBER), num(num) {}

        QuarkToken(size_t line, size_t col, QuarkNumber num, QuarkNumber nm) :
                line(line), col(col), num(num) {
            if (nm.num.get_ui() != 0) {
                num_mask = nm;
                type = QuarkTokenType::NUMBER_WILDCARD;
            } else {
                num_mask = 0;
                type = QuarkTokenType::NUMBER;
            }
        }

        QuarkToken(size_t line, size_t col, float f) :
                line(line), col(col),
                type(QuarkTokenType::NUMBER_FLOAT), f(f) {}

        QuarkToken(size_t line, size_t col, double d) :
                line(line), col(col),
                type(QuarkTokenType::NUMBER_DOUBLE), d(d) {}

        QuarkToken(size_t line, size_t col, QuarkSymbol symbol) :
                line(line), col(col),
                type(QuarkTokenType::SYMBOL), symbol(symbol) {}

        QuarkToken(size_t line, size_t col, QuarkTokenType type, size_t id) :
                line(line), col(col), type(type), id(id) {}

        QuarkToken(size_t line, size_t col, QuarkTokenType type, std::string str) :
                line(line), col(col),
                type(type), str(str) {}
    };

    enum class QuarkNodeType : uint8_t {
        MODULE
    };

    class QuarkNode {

    };

}
