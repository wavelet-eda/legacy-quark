#include "QuarkLexer.h"

#include <algorithm>
#include <functional>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace Quark;

bool is_operator_single(QuarkSymbol sym) {
    return (uint8_t) sym >= 32 and (uint8_t) sym < 128;
}

bool is_operator_double(QuarkSymbol sym) {
    return (uint8_t) sym >= 128 and (uint8_t) sym < 196;
}

bool is_operator_triple(QuarkSymbol sym) {
    return (uint8_t) sym >= 196;
}

struct double_operator_t {
    char second;
    QuarkSymbol result;
};

struct triple_operator_t {
    char second, third;
    QuarkSymbol result;
};

bool OPERATOR_TABLE_INIT = false;
double_operator_t DOUBLE_OPERATOR_TABLE [256][3];
triple_operator_t TRIPLE_OPERATOR_TABLE [256][2];

QuarkSymbol get_double_symbol(QuarkSymbol first_op, QuarkSymbol second_op) {
    if (first_op == QuarkSymbol::WHITESPACE or 
            second_op == QuarkSymbol::WHITESPACE)
        return QuarkSymbol::WHITESPACE;

    if (not is_operator_single(first_op) or 
            not is_operator_single(second_op))
        throw QuarkLexerException("Concatenating non-single character operators!");

    for (size_t j = 0; j < (sizeof(DOUBLE_OPERATOR_TABLE[0])/sizeof(double_operator_t)); j++) {
        double_operator_t dt = DOUBLE_OPERATOR_TABLE[(char)first_op][j];
        if (dt.second == (char) second_op) {
            return dt.result;
        } else if (dt.second == 0) {
            return QuarkSymbol::WHITESPACE;
        }
    }

    return QuarkSymbol::WHITESPACE;
}

QuarkSymbol get_triple_symbol(QuarkSymbol first_op, QuarkSymbol second_op, QuarkSymbol third_op) {
    if (first_op == QuarkSymbol::WHITESPACE or 
            second_op == QuarkSymbol::WHITESPACE or
            third_op == QuarkSymbol::WHITESPACE)
        return QuarkSymbol::WHITESPACE;

    if (not is_operator_single(first_op) or 
            not is_operator_single(second_op) or
            not is_operator_single(third_op))
        throw QuarkLexerException("Concatenating non-single character operators!");

    for (size_t j = 0; j < (sizeof(TRIPLE_OPERATOR_TABLE[0])/sizeof(triple_operator_t)); j++) {
        triple_operator_t tt = TRIPLE_OPERATOR_TABLE[(char)first_op][j];
        if (tt.second == (char) second_op and tt.third == (char) third_op) {
            return tt.result;
        } else if (tt.second == 0) {
            return QuarkSymbol::WHITESPACE;
        }
    }

    return QuarkSymbol::WHITESPACE;
}

QuarkLexer::QuarkLexer(std::istream& in) : in(in) {

    // Initialize operator lookup
    if (not OPERATOR_TABLE_INIT) {
        OPERATOR_TABLE_INIT = true;

        for (size_t i = 0; i < 256; i++) {
            for (size_t j = 0; j < (sizeof(DOUBLE_OPERATOR_TABLE[0])/sizeof(double_operator_t)); j++) {
                DOUBLE_OPERATOR_TABLE[i][j] = {0, QuarkSymbol::WHITESPACE};
            }

            for (size_t j = 0; j < (sizeof(TRIPLE_OPERATOR_TABLE[0])/sizeof(triple_operator_t)); j++) {
                TRIPLE_OPERATOR_TABLE[i][j] = {0, 0, QuarkSymbol::WHITESPACE};
            }
        }

        DOUBLE_OPERATOR_TABLE['='][0] = {'=', QuarkSymbol::EQUAL};
        DOUBLE_OPERATOR_TABLE['+'][0] = {'=', QuarkSymbol::ADDITION_ASSIGN};
        DOUBLE_OPERATOR_TABLE['+'][1] = {'+', QuarkSymbol::INCREMENT};
        DOUBLE_OPERATOR_TABLE['-'][0] = {'=', QuarkSymbol::SUBTRACTION_ASSIGN};
        DOUBLE_OPERATOR_TABLE['-'][1] = {'-', QuarkSymbol::DECREMENT};
        DOUBLE_OPERATOR_TABLE['-'][2] = {'>', QuarkSymbol::COND_IMPLICATION};
        DOUBLE_OPERATOR_TABLE['*'][0] = {'=', QuarkSymbol::MULTIPLICATION_ASSIGN};
        DOUBLE_OPERATOR_TABLE['/'][0] = {'=', QuarkSymbol::DIVISION_ASSIGN};
        DOUBLE_OPERATOR_TABLE['/'][1] = {'/', QuarkSymbol::LINE_COMMENT};
        DOUBLE_OPERATOR_TABLE['/'][2] = {'*', QuarkSymbol::BLOCK_COMMENT};
        DOUBLE_OPERATOR_TABLE['%'][0] = {'=', QuarkSymbol::MODULUS_ASSIGN};
        DOUBLE_OPERATOR_TABLE['&'][0] = {'=', QuarkSymbol::BITWISE_AND_ASSIGN};
        DOUBLE_OPERATOR_TABLE['&'][1] = {'&', QuarkSymbol::COND_AND};
        DOUBLE_OPERATOR_TABLE['|'][0] = {'=', QuarkSymbol::BITWISE_OR_ASSIGN};
        DOUBLE_OPERATOR_TABLE['|'][1] = {'|', QuarkSymbol::COND_OR};
        DOUBLE_OPERATOR_TABLE['^'][0] = {'=', QuarkSymbol::BITWISE_XOR_ASSIGN};
        DOUBLE_OPERATOR_TABLE['<'][0] = {'=', QuarkSymbol::LESS_EQUAL};
        DOUBLE_OPERATOR_TABLE['<'][1] = {'<', QuarkSymbol::LSHIFT};
        DOUBLE_OPERATOR_TABLE['>'][0] = {'=', QuarkSymbol::GREATER_EQUAL};
        DOUBLE_OPERATOR_TABLE['>'][1] = {'>', QuarkSymbol::RSHIFT};
        DOUBLE_OPERATOR_TABLE['!'][0] = {'=', QuarkSymbol::NOT_EQUAL};
        DOUBLE_OPERATOR_TABLE['~'][0] = {'&', QuarkSymbol::BITWISE_NAND};
        DOUBLE_OPERATOR_TABLE['~'][1] = {'|', QuarkSymbol::BITWISE_NOR};
        DOUBLE_OPERATOR_TABLE['~'][2] = {'^', QuarkSymbol::BITWISE_XNOR};
        DOUBLE_OPERATOR_TABLE[':'][0] = {':', QuarkSymbol::DOUBLE_COLON};

        TRIPLE_OPERATOR_TABLE['<'][0] = {'<', '=', QuarkSymbol::LSHIFT_ASSIGN};
        TRIPLE_OPERATOR_TABLE['<'][1] = {'-', '>', QuarkSymbol::COND_EQUIVALENCE};
        TRIPLE_OPERATOR_TABLE['>'][0] = {'>', '=', QuarkSymbol::RSHIFT_ASSIGN};
    }

    keyword_lookup = get_quark_keywords();
}

void QuarkLexer::process_single_quote(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column) {
    QuarkCharacter qc = get_next_character();
    if (qc.is_line_finish())
        throw QuarkLexerException("EOF or new-line found during character or number!", 
                get_current_line(), get_current_column() - 1);
    unsigned int number_base = qc.decode_base();

    if (qc.get_char() == '\\') { // Escaped character
        
        qc = get_next_character();
        if (qc.is_line_finish())
            throw QuarkLexerException("EOF or new-line found during character or number!", 
                    get_current_line(), get_current_column() - 1);
        
        expect_character('\'');

        token_parser(QuarkToken(line, column, QuarkNumber(2, 8, qc.get_escaped_char())));
        
    } else if (number_base > 0) {

        qc = get_next_character();
        if (qc.is_line_finish())
            throw QuarkLexerException("EOF or new-line found during character or number!", 
                    get_current_line(), get_current_column() - 1);


        if (qc.get_char() == '\'') { // Normal character
            token_parser(QuarkToken(line, column, QuarkNumber(2, 8, qc.get_char())));
        } else {
            
            QuarkNumber num((int) number_base);
            QuarkNumber num_mask = num;

            reuse_character();
            process_integer_literal(num, num_mask);
            token_parser(QuarkToken(line, column, num, num_mask));
        }
    } else {
        expect_character('\'');
        token_parser(QuarkToken(line, column, QuarkNumber(2, 8, qc.get_char())));
    }
}

void QuarkLexer::process_number(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column, QuarkCharacter initial_numeral) {

    QuarkNumber num, num_mask;

    QuarkCharacter qc = get_next_character();
    
    if (initial_numeral.get_char() == '0' and qc.get_type() == QuarkCharacterType::NUMERAL)
        throw QuarkLexerException("First digit of a number cannot be zero!", 
                get_current_line(), get_current_column() - 1);
    
    num.insert_digit(initial_numeral);
    while (qc.get_type() == QuarkCharacterType::NUMERAL) {
        num.insert_digit(qc);
        qc = get_next_character();
    }
    
    if (qc.get_char() == '\'') {
        qc = get_next_character();
        unsigned int number_base = qc.decode_base();
        if (qc.get_char() == 's') {
            qc = get_next_character();
            number_base = qc.decode_base();
            if (number_base > 0) {
                num.convert_fixed_width(number_base, true);
            } else {
                throw QuarkLexerException("Invalid number base!",
                        get_current_line(), get_current_column() - 1);
            }
        } else if (number_base > 0) {
            num.convert_fixed_width(number_base, false);
        } else {
            throw QuarkLexerException("Invalid number base!",
                    get_current_line(), get_current_column() - 1);
        }

        process_integer_literal(num, num_mask);

    } else if (qc.get_char() == '.') {

        float f = num.num.get_ui();
        double d = num.num.get_ui();
        int divider = 10;

        qc = get_next_character();
        while (qc.get_type() == QuarkCharacterType::NUMERAL) {
            f += (float) qc.get_numeric_value(10) / divider;
            d += (double) qc.get_numeric_value(10) / divider;
            divider *= 10;
            qc = get_next_character();
        }

        if (qc.get_char() == 'd') {
            token_parser(QuarkToken(line, column, d));
            return;
        } else if (qc.get_char() == 'f') {
            token_parser(QuarkToken(line, column, f));
            return;
        } else {
            throw QuarkLexerException("Expecting a float (f) or double (d) qualifier after floating point number!",
                    get_current_line(), get_current_column() - 1);
        }

    } else if (not qc.is_whitespace()) {
        reuse_character();
    }

    token_parser(QuarkToken(line, column, num, num_mask));
}

void QuarkLexer::process_integer_literal(QuarkNumber &num, QuarkNumber &mask) {
    mask = num;

    QuarkCharacter qc = get_next_character();
    while (not qc.is_whitespace() and (not qc.is_symbol() or (qc.get_char() == '?'))) {
        if (qc.get_char() == '?') {
            num.insert_digit(0);
            mask.insert_digit(mask.base - 1);
        } else {
            num.insert_digit(qc);
            mask.insert_digit(0);
        }
        qc = get_next_character();
    }

    if (not qc.is_whitespace())
        reuse_character();

    if (mask.num.get_ui() != 0 and num.base == 10)
        throw QuarkLexerException("Cannot provide wildcard mask for base-10 number!",
                get_current_line(), get_current_column() - 1);
}

void QuarkLexer::process_string(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column) {
    std::string string_token = "";
    QuarkCharacter lc = get_next_character();
    while (lc.get_char() != '\"') {
        if (lc.get_char() == '\\') {
            lc = get_next_character();
            if (lc.get_type() == QuarkCharacterType::CHAR_EOF) {
                throw QuarkLexerException("EOF found during string!",
                        get_current_line(), get_current_column() - 1);
            } else if (lc.get_type() != QuarkCharacterType::NEWLINE) {
                string_token += lc.get_escaped_char();
            }
        } else if (lc.get_type() == QuarkCharacterType::NEWLINE or
                lc.get_type() == QuarkCharacterType::CHAR_EOF) {
            throw QuarkLexerException("EOF or unescaped new-line found during string!", 
                    get_current_line(), get_current_column() - 1);
        } else {
            string_token += lc.get_char();
        }
        lc = get_next_character();
    }
    token_parser(QuarkToken(line, column, QuarkTokenType::STRING, string_token));
}

void QuarkLexer::process_identifier(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column, QuarkCharacter initial_char) {
    std::string identifier_string;
    identifier_string += initial_char.get_char();
    QuarkCharacter lc = get_next_character();
    while (lc.get_type() == QuarkCharacterType::ALPHA or 
            lc.get_type() == QuarkCharacterType::NUMERAL) {
        identifier_string += lc.get_char();
        lc = get_next_character();
    }

    // Check if it is a keyword
    auto found = keyword_lookup.find(identifier_string);
    if (found != keyword_lookup.end()) {
        size_t keyword_id = std::distance(keyword_lookup.begin(), found);
        token_parser(QuarkToken(line, column, QuarkTokenType::KEYWORD, keyword_id));
    } else {
        size_t identifier_id = add_identifier(identifier_string);
        token_parser(QuarkToken(line, column, QuarkTokenType::IDENTIFIER, identifier_id));
    }

    // The character that ended the identifier was a symbol (deduction) so use it again
    if (not lc.is_whitespace()) {
        reuse_character();
    }
}

// TODO: Enforce spaces between distinct operators (not needed operators and parens, brackets, etc.)
void QuarkLexer::process_symbol(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column, QuarkCharacter first_char) {
    QuarkCharacter second_char = get_next_character(false);
    
    if (second_char.is_symbol()) {
        QuarkSymbol double_symbol = get_double_symbol(
                (QuarkSymbol) first_char.get_char(), 
                (QuarkSymbol) second_char.get_char());

        if (double_symbol == QuarkSymbol::LINE_COMMENT) {
            process_line_comment(token_parser, line, column);
            return;
        } else if (double_symbol == QuarkSymbol::BLOCK_COMMENT) {
            process_block_comment(token_parser, line, column);
            return;
        }

        QuarkCharacter third_char = get_next_character(false);
            
        QuarkSymbol triple_symbol = QuarkSymbol::WHITESPACE;
        if (third_char.is_symbol()) {
            triple_symbol = get_triple_symbol(
                    (QuarkSymbol) first_char.get_char(), 
                    (QuarkSymbol) second_char.get_char(),
                    (QuarkSymbol) third_char.get_char());

            if (triple_symbol != QuarkSymbol::WHITESPACE) {
                token_parser(QuarkToken(line, column, triple_symbol));
                return;
            }
        }

        if (not third_char.is_whitespace()) {
            reuse_character(); // Reuse third symbol
        }

        if (double_symbol != QuarkSymbol::WHITESPACE) {
            token_parser(QuarkToken(line, column, double_symbol));
            return;
        }
    } 

    if (not second_char.is_whitespace()) {
        reuse_character();
    }

    token_parser(QuarkToken(line, column, (QuarkSymbol) first_char.get_char()));
}

void QuarkLexer::process_line_comment(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column) {
    QuarkCharacter comment_char = get_next_character();
    if (comment_char.get_char() == '!') { // Document comment
        std::string doc_comment_str;
        comment_char = get_next_character();
        while (not comment_char.is_line_finish()) {
            doc_comment_str += comment_char.get_char();
            comment_char = get_next_character();
        }
        token_parser(QuarkToken(line, column, QuarkTokenType::DOC_COMMENT, doc_comment_str));
    } else {
        comment_char = get_next_character();
        while (not comment_char.is_line_finish()) {
            comment_char = get_next_character();
        }
    }
}

void QuarkLexer::process_block_comment(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column) {
    QuarkCharacter comment_char = get_next_character();
    if (comment_char.get_char() == '!') { // Document comment
        std::string doc_comment_str;
        comment_char = get_next_character();
        while (not comment_char.is_eof()) {
            if (comment_char.get_char() == '*') {
                comment_char = get_next_character();
                if (comment_char.is_eof())
                    throw QuarkLexerException("Encountered EOF during block comment!", 
                            get_current_line(), get_current_column() - 1);
                if (comment_char.get_char() == '/') {
                    token_parser(QuarkToken(line, column, QuarkTokenType::DOC_COMMENT, doc_comment_str));
                    return;
                } else {
                    doc_comment_str += '*';
                    doc_comment_str += comment_char.get_char();
                }
            } else {
                doc_comment_str += comment_char.get_char();
            }
            comment_char = get_next_character();
        }
        throw QuarkLexerException("Encountered EOF during block comment!", 
                    get_current_line(), get_current_column() - 1);
    } else {
        comment_char = get_next_character();
        while (not comment_char.is_eof()) {
            if (comment_char.get_char() == '*') {
                comment_char = get_next_character();
                if (comment_char.is_eof())
                    throw QuarkLexerException("Encountered EOF during block comment!", 
                            get_current_line(), get_current_column() - 1);
                if (comment_char.get_char() == '/') {
                    return;
                }
            }
            comment_char = get_next_character();
        }
        throw QuarkLexerException("Encountered EOF during block comment!", 
                get_current_line(), get_current_column() - 1);
    }
}

bool QuarkLexer::process(std::function<void(QuarkToken)> token_parser) {
    size_t line = get_current_line();
    size_t column = get_current_column();
    QuarkCharacter qc = get_next_character();
    switch (qc.get_type()) {
        case QuarkCharacterType::ALPHA:
            process_identifier(token_parser, line, column, qc);
            return true;
        case QuarkCharacterType::NUMERAL:
            process_number(token_parser, line, column, qc);
            return true;
        case QuarkCharacterType::SYMBOL:
            if (qc.get_char() == '\'') {
                process_single_quote(token_parser, line, column);
            } else if (qc.get_char() == '\"') {
                process_string(token_parser, line, column);
            } else {
                process_symbol(token_parser, line, column, qc);
            }
            return true;
        case QuarkCharacterType::CHAR_EOF: 
            return false;
    }
    return true;
}

ssize_t validate_utf8_string(std::string s) {
    for (size_t i = 0; i < s.length(); i++) {
        if ((((uint8_t)s[i]) >> 5) == 6) { // Starts two-byte UTF-8 character
            if ((i + 1) >= s.length()) return -(ssize_t)i;
            if ((((uint8_t)s[++i]) >> 6) != 2) return -(ssize_t)i;
        } else if ((((uint8_t)s[i]) >> 4) == 14) { // Starts three-byte UTF-8 character
            if ((i + 2) >= s.length()) return -(ssize_t)i;
            if ((((uint8_t)s[++i]) >> 6) != 2) return -(ssize_t)i;
            if ((((uint8_t)s[++i]) >> 6) != 2) return -(ssize_t)i;
        } else if ((((uint8_t)s[i]) >> 3) == 30) { // Starts four-byte UTF-8 character
            if ((i + 3) >= s.length()) return -(ssize_t)i;
            if ((((uint8_t)s[++i]) >> 6) != 2) return -(ssize_t)i;
            if ((((uint8_t)s[++i]) >> 6) != 2) return -(ssize_t)i;
            if ((((uint8_t)s[++i]) >> 6) != 2) return -(ssize_t)i;
        }
    }
    return 1;
}

QuarkCharacter QuarkLexer::get_next_character(bool enter_next_line) {
    if (unlikely(current_col >= current_line.length())) {
        if (in.eof()) {
            return QuarkCharacter(true); // Indicate character is EOF
        } else {
            if (not enter_next_line)
                return QuarkCharacter(false);
            std::getline(in, current_line);

            ssize_t utf8_validation = validate_utf8_string(current_line);
            if (utf8_validation <= 0)
                throw QuarkLexerException("Invalid UTF-8 line!", get_current_line() + 1, 
                        -utf8_validation, QuarkCharacter(0, QuarkCharacterType::UNDEF));

            current_col = 0;
            current_row++;

            return QuarkCharacter(false); // Indicate character is new-line
        }
    } else {
        uint8_t c = current_line[current_col];
        if (likely(c < QUARK_ASCII_DEL)) { // Normal ASCII characters
            if (c == ' ' or c == '\t') {
                current_col++;
                return QuarkCharacter(c, QuarkCharacterType::WHITESPACE);
            } else if (c <= ' ') {
                throw QuarkLexerException("Invalid control character found!",
                        get_current_line(), get_current_column(),
                        QuarkCharacter(c, QuarkCharacterType::UNDEF));
            } else if (c >= '0' and c <= '9') { // Number
                current_col++;
                return QuarkCharacter(c, QuarkCharacterType::NUMERAL);
            } else if ((c >= 'a' and c <= 'z') || (c >= 'A' and c <= 'Z') || (c == '_')) { // Letter
                current_col++;
                return QuarkCharacter(c, QuarkCharacterType::ALPHA);
            } else { // Symbol
                current_col++;
                return QuarkCharacter(c, QuarkCharacterType::SYMBOL);
            }
        } else if (c == QUARK_ASCII_DEL) {
            throw QuarkLexerException("Invalid control character found!",
                    get_current_line(), get_current_column(),
                    QuarkCharacter(c, QuarkCharacterType::UNDEF));
        } else { // c >= 128, must be UTF-8
            current_col++;
            return QuarkCharacter(c, QuarkCharacterType::ALPHA);
        }
    }
}

void QuarkLexer::reuse_character() {
    if (in.eof() or current_col == 0)
        throw QuarkLexerException("Trying to reuse character immediately after new-line or at EOF!",
                get_current_line(), get_current_column());
    current_col--;
}

void QuarkLexer::expect_character(char c) {
    size_t line = get_current_line();
    size_t column = get_current_column();
    QuarkCharacter qc = get_next_character();
    if (qc.is_line_finish())
        throw QuarkLexerException("Newline or end-of-file was found while expecting character!",
                line, column, qc);
    if (qc.get_char() != c)
        throw QuarkLexerException("Wrong character was found while expecting character!",
                line, column, qc);
}

size_t QuarkLexer::add_identifier(std::string new_identifier) {
    auto found = std::find(identifiers.begin(), identifiers.end(), new_identifier);
    if (found != identifiers.end()) {
        return std::distance(identifiers.begin(), found);
    } else {
        identifiers.push_back(new_identifier);
        return identifiers.size() - 1;
    }
}

void QuarkLexer::lex() {
    tokens.clear();
    lex([this](QuarkToken token) {
        this->tokens.push_back(token);
    });
}

void QuarkLexer::lex(std::function<void(QuarkToken)> token_parser) {
    tokens.clear();
    identifiers.clear();

    current_row = -1;
    current_col = 0;

    while (process(token_parser)); // Process all bytes in the file
}
