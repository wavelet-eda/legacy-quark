#pragma once

#include <functional>
#include <stdint.h>
#include <iostream>
#include <exception>
#include <vector>
#include <set>
// #include <gmp.h>
#include <gmpxx.h>

#define likely(x)      __builtin_expect(!!(x), 1) 
#define unlikely(x)    __builtin_expect(!!(x), 0) 

enum class QuarkLexerSymbol : uint8_t {
    ASSIGN = '=',
    ADDITION = '+', 
    SUBTRACTION = '-', 
    MULTIPLICATION = '*', 
    DIVISION = '/', 
    MODULUS = '%',
    BITWISE_AND = '&', 
    BITWISE_NAND = 128, 
    BITWISE_OR = '|', 
    BITWISE_NOR = 129, 
    BITWISE_XOR = '^', 
    BITWISE_XNOR = 130,
    LSHIFT = 131, 
    RSHIFT = 132,
    ADDITION_ASSIGN = 133, 
    SUBTRACTION_ASSIGN = 134, 
    MULTIPLICATION_ASSIGN = 135, 
    DIVISION_ASSIGN = 136, 
    MODULUS_ASSIGN = 137,
    BITWISE_AND_ASSIGN = 138, 
    BITWISE_OR_ASSIGN = 139, 
    BITWISE_XOR_ASSIGN = 140, 
    LSHIFT_ASSIGN = 196, // Triple
    RSHIFT_ASSIGN = 197, // Triple
    COND_AND = 141, 
    COND_OR = 142, 
    COND_IMPLICATION = 143, 
    COND_EQUIVALENCE = 198, // Triple
    LESS = '<', 
    GREATER = '>', 
    LESS_EQUAL = 144, 
    GREATER_EQUAL = 145, 
    EQUAL = 146, 
    NOT_EQUAL = 147,
    INCREMENT = 148, 
    DECREMENT = 149, 
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
    DOUBLE_COLON = 150, 
    SEMICOLON = ';',
    // Unsupported/unused symbols
    DOLLAR = '$',
    POUND = '#', 
    TICK = '`',
    AT = '@',
    BACKSLASH = '\\',
    // Comment Symbols
    LINE_COMMENT = 151,
    BLOCK_COMMENT = 152,
    // Empty symbol
    WHITESPACE = ' '
};

enum class QuarkLexerCharacterType : uint8_t {
    CHAR_EOF = 0,
    NEWLINE = 1, 
    WHITESPACE = 2, 
    SYMBOL = 4, 
    ALPHA = 5, 
    NUMERAL = 6
};

enum class QuarkLexerTokenType : uint8_t {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    SYMBOL,
    STRING,
    DOC_COMMENT
};

class QuarkLexerException : public std::exception {
private:
    const char* msg;
    size_t line;
    size_t col;
    char character;
    
public:

    QuarkLexerException(const char* msg) : 
        std::exception(),
        msg(msg),
        line(0),
        col(0),
        character('_') {}

    QuarkLexerException(const char* msg,
        size_t line, size_t col, char character) : 
        std::exception(),
        msg(msg),
        line(line),
        col(col),
        character(character) {}

    size_t get_line() const { return line; }
    size_t get_col() const { return col; }
    char get_character() const { return character; }

    const char * what () const throw () { return msg; }
};

struct QuarkLexerNumber {
    unsigned int width;
    int base;
    mpz_class num;
    bool is_signed, is_ambiguous;

    QuarkLexerNumber() : 
            base(10), width(0), num(0),
            is_signed(false), is_ambiguous(true) {}

    QuarkLexerNumber(char c) :
            base(2), width(8), num((int) c),
            is_signed(false), is_ambiguous(false) {}

    QuarkLexerNumber(int base) : 
            base(base), width(0), num(0),
            is_signed(false), is_ambiguous(true) {}

    void insert_msb(char c) {
        int digit;
        if (c >= '0' and c <= '9') digit = c - '0';
        else if (c >= 'a' and c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' and c <= 'Z') digit = c - 'A' + 10;
        else throw QuarkLexerException("Invalid numeral found!");

        if (digit >= base)
            throw QuarkLexerException("Numeral character outside of base!");

        num *= base;
        num += digit;

        if (not is_ambiguous and mpz_sizeinbase(num.get_mpz_t(), 2) > width)
            throw QuarkLexerException("Number larger than defined bit-width!");
    }

    void convert_fixed_width(int base, bool is_signed, int line, int col) {
        if (not num.fits_uint_p())
            throw QuarkLexerException("Constant width takes more than 32-bits!");

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

struct QuarkLexerToken {
    QuarkLexerTokenType type;
    QuarkLexerSymbol symbol;
    QuarkLexerNumber num;
    std::string str;
    size_t id;
    size_t line, col;

    QuarkLexerToken(size_t line, size_t col, QuarkLexerSymbol symbol) :
            line(line), col(col),
            type(QuarkLexerTokenType::SYMBOL), symbol(symbol) {}

    QuarkLexerToken(size_t line, size_t col, QuarkLexerTokenType type, size_t id) :
            line(line), col(col), type(type), id(id) {}

    QuarkLexerToken(size_t line, size_t col, QuarkLexerNumber num) :
            line(line), col(col),
            type(QuarkLexerTokenType::NUMBER), num(num) {}

    QuarkLexerToken(size_t line, size_t col, QuarkLexerTokenType type, std::string str) :
            line(line), col(col),
            type(type), str(str) {}
};

struct QuarkLexerCharacter {
    char c;
    size_t line, col;
    QuarkLexerCharacterType char_type;

    QuarkLexerCharacter(bool eof, size_t line, size_t col) : 
            c('\n'), line(line), col(col),
            char_type(eof ? QuarkLexerCharacterType::CHAR_EOF : QuarkLexerCharacterType::NEWLINE) {}

    QuarkLexerCharacter(char c, size_t line, size_t col,
            QuarkLexerCharacterType char_type) :
            c(c), line(line), col(col), char_type(char_type) {}

    bool is_whitespace() {
        return (((int) char_type) >> 2) == 0;
    }

    bool is_symbol() {
        return char_type == QuarkLexerCharacterType::SYMBOL;
    }

    bool is_line_finish() {
        return ((int) char_type) < 2;
    }

    bool is_eof() {
        return char_type == QuarkLexerCharacterType::CHAR_EOF;
    }

    bool is_hex() {
        return (c >= '0' and c <= '9') or 
                (c >= 'a' and c <= 'f') or
                (c >= 'A' and c <= 'F');
    }
};

class QuarkLexer
{
private:
    std::istream& in;
    std::string current_line;
    size_t current_row; // Rows are zero-indexed where line numbers are one-indexed
    size_t current_col;
    unsigned int utf_bytes_remaining;

    std::vector<std::string> identifiers;
    std::vector<QuarkLexerToken> tokens;
    std::set<std::string> keyword_lookup;

    QuarkLexerCharacter get_next_character(bool enter_next_line);
    QuarkLexerCharacter get_next_character() { return get_next_character(true); };
    void reuse_character();
    void expect_character(char c);

    size_t add_identifier(std::string new_identifier);

    bool process(std::function<void(QuarkLexerToken)> token_parser);
    void process_single_quote(std::function<void(QuarkLexerToken)> token_parser);
    void process_number(std::function<void(QuarkLexerToken)> token_parser, char first_numeral);
    void process_string(std::function<void(QuarkLexerToken)> token_parser);
    void process_identifier(std::function<void(QuarkLexerToken)> token_parser, char initial_char);
    void process_symbol(std::function<void(QuarkLexerToken)> token_parser, char initial_symbol);
    void process_line_comment(std::function<void(QuarkLexerToken)> token_parser);
    void process_block_comment(std::function<void(QuarkLexerToken)> token_parser);

public:

    static const char ASCII_NULL = 0; // warning
    static const char ASCII_TAB = 9; // whitespace
    static const char ASCII_NEWLINE = 10; // whitespace, new line
    static const char ASCII_VT = 11; // whitespace, warning
    static const char ASCII_FF = 12; // whitespace, warning
    static const char ASCII_CR = 13; // whitespace, warning
    static const char ASCII_SPACE = 32; // whitespace
    static const char ASCII_DEL = 127; // whitespace

    const std::string KEYWORDS [33] = {
        "quark", "import", "module", "parameter", "fixed", "input", "output", 
        "reg", "latch", "future", "function", "return", 
        "bit", "signed", "unsigned", "safe", 
        "overflow", "concat", "clog2", "width", "length", "resize", "cast", 
        "struct", "typedef", "enum", 
        "channel", "interface", 
        "if", "else", "for", "break", "continue"
    };

    QuarkLexer(std::istream& in);
    ~QuarkLexer() {};
    void lex(std::function<void(QuarkLexerToken)> token_parser);
    void lex();

    // std::vector<QuarkLexerException> get_warnings() { return warnings; }
    std::vector<QuarkLexerToken> get_tokens() { return tokens; }
    std::vector<std::string> get_identifiers() { return identifiers; }
    std::string get_identifier(size_t id) {return identifiers[id]; }
    std::string get_keyword(size_t id) { return *std::next(keyword_lookup.begin(), id); }
};
