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

    class QuarkLexerException : public std::exception {
    private:
        const char* msg;
        size_t line, col;
        QuarkCharacter qc;
        
    public:

        QuarkLexerException(const char* msg) : 
                std::exception(), msg(msg), line(0), col(0) {}

        QuarkLexerException(const char* msg, size_t line, size_t col) : 
                std::exception(), msg(msg), line(line), col(col) {}

        QuarkLexerException(const char* msg, size_t line, size_t col, QuarkCharacter qc) : 
                std::exception(), msg(msg), line(line), col(col), qc(qc) {}

        QuarkLexerException(QuarkException e, size_t line, size_t col) :
                std::exception(), msg(e.what()), line(line), col(col), qc(e.get_char()) {}

        size_t get_line() const { return line; }
        size_t get_col() const { return col; }
        QuarkCharacter get_char() const { return qc; }

        const char * what () const throw () { return msg; }
    };

    class QuarkLexer {
    private:
        std::istream& in;
        std::string current_line;
        size_t current_row; // Rows are zero-indexed where line numbers are one-indexed
        size_t current_col;
        unsigned int utf_bytes_remaining;

        std::vector<std::string> identifiers;
        std::vector<QuarkToken> tokens;
        std::set<std::string> keyword_lookup;

        QuarkCharacter get_next_character(bool enter_next_line);
        QuarkCharacter get_next_character() { return get_next_character(true); };
        void reuse_character();
        void expect_character(char c);

        size_t get_current_line() { return current_row + 1; }
        size_t get_current_column() { return current_col + 1; }

        size_t add_identifier(std::string new_identifier);

        bool process(std::function<void(QuarkToken)> token_parser);
        void process_single_quote(std::function<void(QuarkToken)> token_parser,
                size_t line, size_t column);
        void process_number(std::function<void(QuarkToken)> token_parser,
                size_t line, size_t column, QuarkCharacter first_numeral);
        void process_string(std::function<void(QuarkToken)> token_parser,
                size_t line, size_t column);
        void process_identifier(std::function<void(QuarkToken)> token_parser,
                size_t line, size_t column, QuarkCharacter initial_char);
        void process_symbol(std::function<void(QuarkToken)> token_parser,
                size_t line, size_t column, QuarkCharacter initial_symbol);
        void process_line_comment(std::function<void(QuarkToken)> token_parser,
                size_t line, size_t column);
        void process_block_comment(std::function<void(QuarkToken)> token_parser,
                size_t line, size_t column);

        const std::string QUARK_KEYWORDS [33] = {
            "quark", "import", "module", "parameter", "fixed", "input", "output", 
            "reg", "latch", "future", "function", "return", 
            "bit", "signed", "unsigned", "safe", 
            "overflow", "concat", "clog2", "width", "length", "resize", "cast", 
            "struct", "typedef", "enum", 
            "channel", "interface", 
            "if", "else", "for", "break", "continue"
        };

    public:

        QuarkLexer(std::istream& in);
        ~QuarkLexer() {};
        void lex(std::function<void(QuarkToken)> token_parser);
        void lex();

        std::vector<QuarkToken> get_tokens() { return tokens; }
        std::vector<std::string> get_identifiers() { return identifiers; }
        std::string get_identifier(size_t id) {return identifiers[id]; }
        std::string get_keyword(size_t id) { return *std::next(keyword_lookup.begin(), id); }
    };
}
