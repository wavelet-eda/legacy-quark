#include "QuarkLexer.h"

#include <algorithm>
#include <functional>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

unsigned int decode_base(char base_char) {
    switch (base_char) {
        case 'b': return 2;
        case 'o': return 8;
        case 'd': return 10;
        case 'h': return 16;
        default: return 0;
    }
}

bool is_operator_single(QuarkLexerSymbol sym) {
    return (uint8_t) sym >= 32 and (uint8_t) sym < 128;
}

bool is_operator_double(QuarkLexerSymbol sym) {
    return (uint8_t) sym >= 128 and (uint8_t) sym < 196;
}

bool is_operator_triple(QuarkLexerSymbol sym) {
    return (uint8_t) sym >= 196;
}

QuarkLexerSymbol get_double_symbol(
        QuarkLexerSymbol first_op, 
        QuarkLexerSymbol second_op) {
    if (first_op == QuarkLexerSymbol::WHITESPACE or 
            second_op == QuarkLexerSymbol::WHITESPACE)
        return QuarkLexerSymbol::WHITESPACE;

    if (not is_operator_single(first_op) or 
            not is_operator_single(second_op))
        throw QuarkLexerException("Concatenating non-single character operators!");

    switch (first_op) {
        case QuarkLexerSymbol::ASSIGN:
            return (second_op == QuarkLexerSymbol::ASSIGN) ?
                    QuarkLexerSymbol::EQUAL : QuarkLexerSymbol::WHITESPACE;
        case QuarkLexerSymbol::ADDITION:
            switch (second_op) {
                case QuarkLexerSymbol::ADDITION: return QuarkLexerSymbol::INCREMENT;
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::ADDITION_ASSIGN;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::SUBTRACTION:
            switch (second_op) {
                case QuarkLexerSymbol::SUBTRACTION: return QuarkLexerSymbol::DECREMENT;
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::SUBTRACTION_ASSIGN;
                case QuarkLexerSymbol::GREATER: return QuarkLexerSymbol::COND_IMPLICATION;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::MULTIPLICATION:
            return (second_op == QuarkLexerSymbol::ASSIGN) ?
                    QuarkLexerSymbol::MULTIPLICATION_ASSIGN : QuarkLexerSymbol::WHITESPACE;
        case QuarkLexerSymbol::DIVISION:
            switch (second_op) {
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::DIVISION_ASSIGN;
                case QuarkLexerSymbol::DIVISION: return QuarkLexerSymbol::LINE_COMMENT;
                case QuarkLexerSymbol::MULTIPLICATION: return QuarkLexerSymbol::BLOCK_COMMENT;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::MODULUS:
            return (second_op == QuarkLexerSymbol::ASSIGN) ?
                    QuarkLexerSymbol::MODULUS_ASSIGN : QuarkLexerSymbol::WHITESPACE;
        case QuarkLexerSymbol::BITWISE_AND:
            switch (second_op) {
                case QuarkLexerSymbol::BITWISE_AND: return QuarkLexerSymbol::COND_AND;
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::BITWISE_AND_ASSIGN;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::BITWISE_OR:
            switch (second_op) {
                case QuarkLexerSymbol::BITWISE_OR: return QuarkLexerSymbol::COND_OR;
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::BITWISE_OR_ASSIGN;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::BITWISE_XOR:
            switch (second_op) {
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::BITWISE_XOR_ASSIGN;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::LESS:
            switch (second_op) {
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::LESS_EQUAL;
                case QuarkLexerSymbol::LESS: return QuarkLexerSymbol::LSHIFT;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::GREATER:
            switch (second_op) {
                case QuarkLexerSymbol::ASSIGN: return QuarkLexerSymbol::GREATER_EQUAL;
                case QuarkLexerSymbol::GREATER: return QuarkLexerSymbol::RSHIFT;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::INVERT:
            switch (second_op) {
                case QuarkLexerSymbol::BITWISE_AND: return QuarkLexerSymbol::BITWISE_NAND;
                case QuarkLexerSymbol::BITWISE_OR: return QuarkLexerSymbol::BITWISE_NOR;
                case QuarkLexerSymbol::BITWISE_XOR: return QuarkLexerSymbol::BITWISE_XNOR;
                default: return QuarkLexerSymbol::WHITESPACE;
            }
        case QuarkLexerSymbol::BANG:
            return (second_op == QuarkLexerSymbol::ASSIGN) ?
                    QuarkLexerSymbol::NOT_EQUAL : QuarkLexerSymbol::WHITESPACE;
        case QuarkLexerSymbol::COLON:
            return (second_op == QuarkLexerSymbol::COLON) ?
                    QuarkLexerSymbol::DOUBLE_COLON : QuarkLexerSymbol::WHITESPACE;
        default: return QuarkLexerSymbol::WHITESPACE;
    }
}

QuarkLexerSymbol get_triple_symbol(
        QuarkLexerSymbol first_op, 
        QuarkLexerSymbol second_op,
        QuarkLexerSymbol third_op) {
    if (first_op == QuarkLexerSymbol::WHITESPACE or 
            second_op == QuarkLexerSymbol::WHITESPACE or
            third_op == QuarkLexerSymbol::WHITESPACE)
        return QuarkLexerSymbol::WHITESPACE;

    if (not is_operator_single(first_op) or 
            not is_operator_single(second_op) or
            not is_operator_single(third_op))
        throw QuarkLexerException("Concatenating non-single character operators!");

    if (first_op == QuarkLexerSymbol::LESS) {
        if (second_op == QuarkLexerSymbol::LESS) {
            if (third_op == QuarkLexerSymbol::ASSIGN) {
                return QuarkLexerSymbol::LSHIFT_ASSIGN;
            }
        } else if (second_op == QuarkLexerSymbol::SUBTRACTION) {
            if (third_op == QuarkLexerSymbol::GREATER) {
                return QuarkLexerSymbol::COND_EQUIVALENCE;
            }
        }
    } else if (first_op == QuarkLexerSymbol::GREATER) {
        if (second_op == QuarkLexerSymbol::LESS) {
            if (third_op == QuarkLexerSymbol::ASSIGN) {
                return QuarkLexerSymbol::RSHIFT_ASSIGN;
            }
        }
    }
    return QuarkLexerSymbol::WHITESPACE;
}

char convert_escaped_char(char escape_char) {
    switch (escape_char) {
        case 'a' : return '\a';
        case 'b' : return '\b';
        case 'e' : return '\e';
        case 'f' : return '\f';
        case 'n' : return '\n';
        case 'r' : return '\r';
        case 't' : return '\t';
        case 'v' : return '\v';
        case '\'' : return '\'';
        case '\"' : return '\"';
        case '\\' : return '\\';
        default: throw QuarkLexerException("Invalid escaped character in string!");
    }
    return 0;
}

QuarkLexer::QuarkLexer(std::istream& in) : in(in) {

    // Initialize keyword lookup
    for (size_t i = 0; i < (sizeof(KEYWORDS)/sizeof(KEYWORDS[0])); i++) {
        keyword_lookup.insert(KEYWORDS[i]);
    }
}

void QuarkLexer::process_single_quote(std::function<void(QuarkLexerToken)> token_parser) {
    QuarkLexerCharacter char_c = get_next_character();
    if (char_c.is_line_finish())
        throw QuarkLexerException("EOF or new-line found during character or number!");
    char original_char = char_c.c;
    unsigned int number_base = decode_base(char_c.c);

    if (char_c.c == '\\') { // Escaped character
        
        char_c = get_next_character();
        if (char_c.is_line_finish())
            throw QuarkLexerException("EOF or new-line found during character or number!");
        
        expect_character('\'');

        token_parser(QuarkLexerToken(0, 0, QuarkLexerNumber(
                convert_escaped_char(char_c.c))));
        
    } else if (number_base > 0) {

        char_c = get_next_character();
        if (char_c.is_line_finish())
            throw QuarkLexerException("EOF or new-line found during character or number!");


        if (char_c.c == '\'') { // Normal character
            token_parser(QuarkLexerToken(0, 0, QuarkLexerNumber(char_c.c)));
        } else {
            QuarkLexerNumber num((int) number_base);
            while (not char_c.is_whitespace() and not char_c.is_symbol()) {
                num.insert_msb(char_c.c);
                char_c = get_next_character();
            }

            if (not char_c.is_whitespace())
                reuse_character();

            token_parser(QuarkLexerToken(0, 0, num));
        }
    } else {
        expect_character('\'');
        token_parser(QuarkLexerToken(0, 0, QuarkLexerNumber(char_c.c)));
    }
}

void QuarkLexer::process_number(std::function<void(QuarkLexerToken)> token_parser,
        char first_numeral) {

    QuarkLexerNumber num;

    QuarkLexerCharacter qc = get_next_character();
    
    if (first_numeral == '0' and qc.char_type == QuarkLexerCharacterType::NUMERAL)
        throw QuarkLexerException("First digit of a number cannot be zero!");
    
    num.insert_msb(first_numeral);
    while (qc.char_type == QuarkLexerCharacterType::NUMERAL) {
        num.insert_msb(qc.c);
        qc = get_next_character();
    }
    
    if (qc.c == '\'') {
        qc = get_next_character();
        unsigned int number_base = decode_base(qc.c);
        if (qc.c == 's') {
            qc = get_next_character();
            number_base = decode_base(qc.c);
            if (number_base > 0) {
                num.convert_fixed_width(number_base, true, 0, 0);
            } else {
                throw QuarkLexerException("Invalid number base!");
            }
        } else if (number_base > 0) {
            num.convert_fixed_width(number_base, false, 0, 0);
        } else {
            throw QuarkLexerException("Invalid number base!");
        }

        qc = get_next_character();
        while (not qc.is_whitespace() and not qc.is_symbol()) {
            num.insert_msb(qc.c);
            qc = get_next_character();
        }

        if (not qc.is_whitespace())
            reuse_character();

    } else if (not qc.is_whitespace()) {
        reuse_character();
    }

    token_parser(QuarkLexerToken(0, 0, num));
}

void QuarkLexer::process_string(std::function<void(QuarkLexerToken)> token_parser) {
    std::string string_token = "";
    QuarkLexerCharacter lc = get_next_character();
    while (lc.c != '\"') {
        if (lc.c == '\\') {
            lc = get_next_character();
            if (lc.char_type == QuarkLexerCharacterType::CHAR_EOF) {
                throw QuarkLexerException("EOF found during string!");
            } else if (lc.char_type != QuarkLexerCharacterType::NEWLINE) {
                string_token += convert_escaped_char(lc.c);
            }
        } else if (lc.char_type == QuarkLexerCharacterType::NEWLINE or
                lc.char_type == QuarkLexerCharacterType::CHAR_EOF) {
            throw QuarkLexerException("EOF or unescaped new-line found during string!");
        } else {
            string_token += lc.c;
        }
        lc = get_next_character();
    }
    token_parser(QuarkLexerToken(0, 0, QuarkLexerTokenType::STRING, string_token));
}

void QuarkLexer::process_identifier(std::function<void(QuarkLexerToken)> token_parser, 
        char initial_char) {
    std::string identifier_string;
    identifier_string += initial_char;
    QuarkLexerCharacter lc = get_next_character();
    while (lc.char_type == QuarkLexerCharacterType::ALPHA or 
            lc.char_type == QuarkLexerCharacterType::NUMERAL) {
        identifier_string += lc.c;
        lc = get_next_character();
    }

    // Check if it is a keyword
    auto found = keyword_lookup.find(identifier_string);
    if (found != keyword_lookup.end()) {
        size_t keyword_id = std::distance(keyword_lookup.begin(), found);
        token_parser(QuarkLexerToken(0, 0, QuarkLexerTokenType::KEYWORD, keyword_id));
    } else {
        size_t identifier_id = add_identifier(identifier_string);
        token_parser(QuarkLexerToken(0, 0, QuarkLexerTokenType::IDENTIFIER, identifier_id));
    }

    // The character that ended the identifier was a symbol (deduction) so use it again
    if (not lc.is_whitespace()) {
        reuse_character();
    }
}

void QuarkLexer::process_symbol(std::function<void(QuarkLexerToken)> token_parser,
        char first_symbol) {
    QuarkLexerCharacter second_char = get_next_character(false);
    
    if (second_char.is_symbol()) {
        QuarkLexerSymbol double_symbol = get_double_symbol(
                (QuarkLexerSymbol) first_symbol, 
                (QuarkLexerSymbol) second_char.c);

        if (double_symbol == QuarkLexerSymbol::LINE_COMMENT) {
            process_line_comment(token_parser);
            return;
        } else if (double_symbol == QuarkLexerSymbol::BLOCK_COMMENT) {
            process_block_comment(token_parser);
            return;
        }

        QuarkLexerCharacter third_char = get_next_character(false);
            
        QuarkLexerSymbol triple_symbol = QuarkLexerSymbol::WHITESPACE;
        if (third_char.is_symbol()) {
            triple_symbol = get_triple_symbol(
                    (QuarkLexerSymbol) first_symbol, 
                    (QuarkLexerSymbol) second_char.c,
                    (QuarkLexerSymbol) third_char.c);

            if (triple_symbol != QuarkLexerSymbol::WHITESPACE) {
                token_parser(QuarkLexerToken(0, 0, triple_symbol));
                return;
            }
        }

        if (not third_char.is_whitespace()) {
            reuse_character(); // Reuse third symbol
        }

        if (double_symbol != QuarkLexerSymbol::WHITESPACE) {
            token_parser(QuarkLexerToken(0, 0, double_symbol));
            return;
        }
    } 

    if (not second_char.is_whitespace()) {
        reuse_character();
    }

    token_parser(QuarkLexerToken(0, 0, (QuarkLexerSymbol) first_symbol));
}

void QuarkLexer::process_line_comment(std::function<void(QuarkLexerToken)> token_parser) {
    QuarkLexerCharacter comment_char = get_next_character();
    if (comment_char.c == '!') { // Document comment
        std::string doc_comment_str;
        comment_char = get_next_character();
        while (not comment_char.is_line_finish()) {
            doc_comment_str += comment_char.c;
            comment_char = get_next_character();
        }
        token_parser(QuarkLexerToken(0, 0, QuarkLexerTokenType::DOC_COMMENT, doc_comment_str));
    } else {
        comment_char = get_next_character();
        while (not comment_char.is_line_finish()) {
            comment_char = get_next_character();
        }
    }
}

void QuarkLexer::process_block_comment(std::function<void(QuarkLexerToken)> token_parser) {
    QuarkLexerCharacter comment_char = get_next_character();
    if (comment_char.c == '!') { // Document comment
        std::string doc_comment_str;
        comment_char = get_next_character();
        while (not comment_char.is_eof()) {
            if (comment_char.c == '*') {
                comment_char = get_next_character();
                if (comment_char.is_eof())
                    throw QuarkLexerException("Encountered EOF during block comment!");
                if (comment_char.c == '/') {
                    token_parser(QuarkLexerToken(0, 0, QuarkLexerTokenType::DOC_COMMENT, doc_comment_str));
                    return;
                } else {
                    doc_comment_str += '*';
                    doc_comment_str += comment_char.c;
                }
            } else {
                doc_comment_str += comment_char.c;
            }
            comment_char = get_next_character();
        }
        throw QuarkLexerException("Encountered EOF during block comment!");
    } else {
        comment_char = get_next_character();
        while (not comment_char.is_eof()) {
            if (comment_char.c == '*') {
                comment_char = get_next_character();
                if (comment_char.is_eof())
                    throw QuarkLexerException("Encountered EOF during block comment!");
                if (comment_char.c == '/') {
                    return;
                }
            }
            comment_char = get_next_character();
        }
        throw QuarkLexerException("Encountered EOF during block comment!");
    }
}

bool QuarkLexer::process(std::function<void(QuarkLexerToken)> token_parser) {
    QuarkLexerCharacter c = get_next_character();
    switch (c.char_type) {
        case QuarkLexerCharacterType::ALPHA:
            process_identifier(token_parser, c.c);
            return true;
        case QuarkLexerCharacterType::NUMERAL:
            process_number(token_parser, c.c);
            return true;
        case QuarkLexerCharacterType::SYMBOL:
            if (c.c == '\'') {
                process_single_quote(token_parser);
            } else if (c.c == '\"') {
                process_string(token_parser);
            } else {
                process_symbol(token_parser, c.c);
            }
            return true;
        case QuarkLexerCharacterType::CHAR_EOF: 
            return false;
    }
    return true;
}

QuarkLexerCharacter QuarkLexer::get_next_character(bool enter_next_line) {
    if (current_col >= current_line.length()) {
        if (unlikely(utf_bytes_remaining)) {
            throw QuarkLexerException("New-line or EOF encountered during multi-byte UTF-8 character!",
                current_row + 1, current_col, '\n');
        } else if (in.eof()) {
            return QuarkLexerCharacter(true, current_row + 1, current_col); // Indicate character is EOF
        } else {
            if (not enter_next_line)
                return QuarkLexerCharacter(false, current_row + 1, current_col);
            std::getline(in, current_line);
            QuarkLexerCharacter qc(false, current_row++, current_col); // Indicate character is new-line
            current_col = 0;
            return qc;
        }
    } else {
        uint8_t c = current_line[current_col];
        current_col++;
        if (unlikely(utf_bytes_remaining)) {
            if ((c >> 6) != 2)
                throw QuarkLexerException("Invalid n-th byte for multi-byte UTF-8 character!", 
                        current_row + 1, current_col, c);
            utf_bytes_remaining--;
            return QuarkLexerCharacter(c, current_row + 1, current_col,
                    QuarkLexerCharacterType::ALPHA);
        } else {
            if (likely(c < ASCII_DEL)) { // Normal ASCII characters
                if (c <= ASCII_SPACE) { // Whitespace and control
                    switch (c) {
                        case ASCII_VT:
                        case ASCII_FF:
                        case ASCII_CR: // Warning whitespace
                            throw QuarkLexerException("Unusual control character found (possible Windows line-ending)!",
                                current_row + 1, current_col, c);
                        case ASCII_SPACE:
                        case ASCII_TAB: // Normal whitespace
                            return QuarkLexerCharacter(c, current_row + 1, current_col,
                                QuarkLexerCharacterType::WHITESPACE);
                        default:
                            throw QuarkLexerException("Invalid control character found!",
                                    current_row + 1, current_col, c);
                    }
                } else if (c >= '0' and c <= '9') { // Number
                    return QuarkLexerCharacter(c, current_row + 1, current_col,
                            QuarkLexerCharacterType::NUMERAL);
                } else if ((c >= 'a' and c <= 'z') || (c >= 'A' and c <= 'Z') || (c == '_')) { // Letter
                    return QuarkLexerCharacter(c, current_row + 1, current_col,
                            QuarkLexerCharacterType::ALPHA);
                } else { // Symbol
                    return QuarkLexerCharacter(c, current_row + 1, current_col,
                            QuarkLexerCharacterType::SYMBOL);
                }
            } else if (c == ASCII_DEL) {
                throw QuarkLexerException("Invalid control character found!",
                        current_row + 1, current_col, c);
            } else if ((c >> 5) == 6) { // Starts two-byte UTF-8 character
                utf_bytes_remaining = 1;
                return QuarkLexerCharacter(c, current_row + 1, current_col,
                        QuarkLexerCharacterType::ALPHA);
            } else if ((c >> 4) == 14) { // Starts three-byte UTF-8 character
                utf_bytes_remaining = 2;
                return QuarkLexerCharacter(c, current_row + 1, current_col,
                        QuarkLexerCharacterType::ALPHA);
            } else if ((c >> 3) == 30) { // Starts four-byte UTF-8 character
                utf_bytes_remaining = 3;
                return QuarkLexerCharacter(c, current_row + 1, current_col,
                        QuarkLexerCharacterType::ALPHA);
            } else {
                throw QuarkLexerException("Non-UTF8 character encountered!",
                        current_row + 1, current_col, c);
            }
        }
    }
}

void QuarkLexer::reuse_character() {
    if (in.eof() or current_col == 0)
        throw QuarkLexerException("Trying to reuse character immediately after new-line or at EOF!");
    current_col--;
}

void QuarkLexer::expect_character(char c) {
    QuarkLexerCharacter qc = get_next_character();
    if (qc.is_line_finish())
        throw QuarkLexerException("Newline or end-of-file was found while expecting character!");
    if (qc.c != c)
        throw QuarkLexerException("Wrong character was found while expecting character!");
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
    lex([this](QuarkLexerToken token) {
        this->tokens.push_back(token);
    });
}

void QuarkLexer::lex(std::function<void(QuarkLexerToken)> token_parser) {
    tokens.clear();
    identifiers.clear();

    current_row = -1;
    current_col = 0;
    utf_bytes_remaining = 0;

    while (process(token_parser)); // Process all bytes in the file
}
