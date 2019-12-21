#include "QuarkLexer.h"

#include <algorithm>
#include <functional>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace Quark;

unsigned int decode_base(char base_char) {
    switch (base_char) {
        case 'b': return 2;
        case 'o': return 8;
        case 'd': return 10;
        case 'h': return 16;
        default: return 0;
    }
}

bool is_operator_single(QuarkSymbol sym) {
    return (uint8_t) sym >= 32 and (uint8_t) sym < 128;
}

bool is_operator_double(QuarkSymbol sym) {
    return (uint8_t) sym >= 128 and (uint8_t) sym < 196;
}

bool is_operator_triple(QuarkSymbol sym) {
    return (uint8_t) sym >= 196;
}

QuarkSymbol get_double_symbol(QuarkSymbol first_op, QuarkSymbol second_op) {
    if (first_op == QuarkSymbol::WHITESPACE or 
            second_op == QuarkSymbol::WHITESPACE)
        return QuarkSymbol::WHITESPACE;

    if (not is_operator_single(first_op) or 
            not is_operator_single(second_op))
        throw QuarkLexerException("Concatenating non-single character operators!");

    switch (first_op) {
        case QuarkSymbol::ASSIGN:
            return (second_op == QuarkSymbol::ASSIGN) ?
                    QuarkSymbol::EQUAL : QuarkSymbol::WHITESPACE;
        case QuarkSymbol::ADDITION:
            switch (second_op) {
                case QuarkSymbol::ADDITION: return QuarkSymbol::INCREMENT;
                case QuarkSymbol::ASSIGN: return QuarkSymbol::ADDITION_ASSIGN;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::SUBTRACTION:
            switch (second_op) {
                case QuarkSymbol::SUBTRACTION: return QuarkSymbol::DECREMENT;
                case QuarkSymbol::ASSIGN: return QuarkSymbol::SUBTRACTION_ASSIGN;
                case QuarkSymbol::GREATER: return QuarkSymbol::COND_IMPLICATION;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::MULTIPLICATION:
            return (second_op == QuarkSymbol::ASSIGN) ?
                    QuarkSymbol::MULTIPLICATION_ASSIGN : QuarkSymbol::WHITESPACE;
        case QuarkSymbol::DIVISION:
            switch (second_op) {
                case QuarkSymbol::ASSIGN: return QuarkSymbol::DIVISION_ASSIGN;
                case QuarkSymbol::DIVISION: return QuarkSymbol::LINE_COMMENT;
                case QuarkSymbol::MULTIPLICATION: return QuarkSymbol::BLOCK_COMMENT;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::MODULUS:
            return (second_op == QuarkSymbol::ASSIGN) ?
                    QuarkSymbol::MODULUS_ASSIGN : QuarkSymbol::WHITESPACE;
        case QuarkSymbol::BITWISE_AND:
            switch (second_op) {
                case QuarkSymbol::BITWISE_AND: return QuarkSymbol::COND_AND;
                case QuarkSymbol::ASSIGN: return QuarkSymbol::BITWISE_AND_ASSIGN;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::BITWISE_OR:
            switch (second_op) {
                case QuarkSymbol::BITWISE_OR: return QuarkSymbol::COND_OR;
                case QuarkSymbol::ASSIGN: return QuarkSymbol::BITWISE_OR_ASSIGN;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::BITWISE_XOR:
            switch (second_op) {
                case QuarkSymbol::ASSIGN: return QuarkSymbol::BITWISE_XOR_ASSIGN;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::LESS:
            switch (second_op) {
                case QuarkSymbol::ASSIGN: return QuarkSymbol::LESS_EQUAL;
                case QuarkSymbol::LESS: return QuarkSymbol::LSHIFT;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::GREATER:
            switch (second_op) {
                case QuarkSymbol::ASSIGN: return QuarkSymbol::GREATER_EQUAL;
                case QuarkSymbol::GREATER: return QuarkSymbol::RSHIFT;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::INVERT:
            switch (second_op) {
                case QuarkSymbol::BITWISE_AND: return QuarkSymbol::BITWISE_NAND;
                case QuarkSymbol::BITWISE_OR: return QuarkSymbol::BITWISE_NOR;
                case QuarkSymbol::BITWISE_XOR: return QuarkSymbol::BITWISE_XNOR;
                default: return QuarkSymbol::WHITESPACE;
            }
        case QuarkSymbol::BANG:
            return (second_op == QuarkSymbol::ASSIGN) ?
                    QuarkSymbol::NOT_EQUAL : QuarkSymbol::WHITESPACE;
        case QuarkSymbol::COLON:
            return (second_op == QuarkSymbol::COLON) ?
                    QuarkSymbol::DOUBLE_COLON : QuarkSymbol::WHITESPACE;
        default: return QuarkSymbol::WHITESPACE;
    }
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

    if (first_op == QuarkSymbol::LESS) {
        if (second_op == QuarkSymbol::LESS) {
            if (third_op == QuarkSymbol::ASSIGN) {
                return QuarkSymbol::LSHIFT_ASSIGN;
            }
        } else if (second_op == QuarkSymbol::SUBTRACTION) {
            if (third_op == QuarkSymbol::GREATER) {
                return QuarkSymbol::COND_EQUIVALENCE;
            }
        }
    } else if (first_op == QuarkSymbol::GREATER) {
        if (second_op == QuarkSymbol::LESS) {
            if (third_op == QuarkSymbol::ASSIGN) {
                return QuarkSymbol::RSHIFT_ASSIGN;
            }
        }
    }
    return QuarkSymbol::WHITESPACE;
}

QuarkLexer::QuarkLexer(std::istream& in) : in(in) {

    // Initialize keyword lookup
    for (size_t i = 0; i < (sizeof(QUARK_KEYWORDS)/sizeof(QUARK_KEYWORDS[0])); i++) {
        keyword_lookup.insert(QUARK_KEYWORDS[i]);
    }
}

void QuarkLexer::process_single_quote(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column) {
    QuarkCharacter qc = get_next_character();
    if (qc.is_line_finish())
        throw QuarkLexerException("EOF or new-line found during character or number!", 
                get_current_line(), get_current_column() - 1);
    unsigned int number_base = decode_base(qc.get_char());

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
            while (not qc.is_whitespace() and not qc.is_symbol()) {
                num.insert_digit(qc);
                qc = get_next_character();
            }

            if (not qc.is_whitespace())
                reuse_character();

            token_parser(QuarkToken(line, column, num));
        }
    } else {
        expect_character('\'');
        token_parser(QuarkToken(line, column, QuarkNumber(2, 8, qc.get_char())));
    }
}

void QuarkLexer::process_number(std::function<void(QuarkToken)> token_parser,
        size_t line, size_t column, QuarkCharacter initial_numeral) {

    QuarkNumber num;

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
        unsigned int number_base = decode_base(qc.get_char());
        if (qc.get_char() == 's') {
            qc = get_next_character();
            number_base = decode_base(qc.get_char());
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

        qc = get_next_character();
        while (not qc.is_whitespace() and not qc.is_symbol()) {
            num.insert_digit(qc);
            qc = get_next_character();
        }

        if (not qc.is_whitespace())
            reuse_character();

    } else if (not qc.is_whitespace()) {
        reuse_character();
    }

    token_parser(QuarkToken(line, column, num));
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

QuarkCharacter QuarkLexer::get_next_character(bool enter_next_line) {
    if (current_col >= current_line.length()) {
        if (unlikely(utf_bytes_remaining)) {
            throw QuarkLexerException("New-line or EOF encountered during multi-byte UTF-8 character!",
                get_current_line(), get_current_column(), 
                QuarkCharacter('\n', QuarkCharacterType::NEWLINE));
        } else if (in.eof()) {
            return QuarkCharacter(true); // Indicate character is EOF
        } else {
            if (not enter_next_line)
                return QuarkCharacter(false);
            std::getline(in, current_line);
            QuarkCharacter qc(false); // Indicate character is new-line
            current_col = 0;
            current_row++;
            return qc;
        }
    } else {
        uint8_t c = current_line[current_col];
        if (unlikely(utf_bytes_remaining)) {
            if ((c >> 6) != 2)
                throw QuarkLexerException("Invalid n-th byte for multi-byte UTF-8 character!", 
                        get_current_line(), get_current_column(),
                        QuarkCharacter('\n', QuarkCharacterType::UNDEF));
            utf_bytes_remaining--;
            current_col++;
            return QuarkCharacter(c, QuarkCharacterType::ALPHA);
        } else {
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
            } else if ((c >> 5) == 6) { // Starts two-byte UTF-8 character
                utf_bytes_remaining = 1;
                current_col++;
                return QuarkCharacter(c, QuarkCharacterType::ALPHA);
            } else if ((c >> 4) == 14) { // Starts three-byte UTF-8 character
                utf_bytes_remaining = 2;
                current_col++;
                return QuarkCharacter(c, QuarkCharacterType::ALPHA);
            } else if ((c >> 3) == 30) { // Starts four-byte UTF-8 character
                utf_bytes_remaining = 3;
                current_col++;
                return QuarkCharacter(c, QuarkCharacterType::ALPHA);
            } else {
                throw QuarkLexerException("Non-UTF8 character encountered!",
                        get_current_line(), get_current_column(),
                        QuarkCharacter(c, QuarkCharacterType::UNDEF));
            }
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
    utf_bytes_remaining = 0;

    while (process(token_parser)); // Process all bytes in the file
}
