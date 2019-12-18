#include <iostream>
#include <fstream>
#include <string>

#include "QuarkLexer.h"

std::string get_symbol_str(QuarkLexerSymbol sym) {

    if ((uint8_t) sym >= 32 and (uint8_t) sym < 128) {
        std::string result;
        result += (char) sym;
        return result;
    } else {
        switch (sym) {
            case QuarkLexerSymbol::BITWISE_NAND: return "~&";
            case QuarkLexerSymbol::BITWISE_NOR: return "~|";
            case QuarkLexerSymbol::BITWISE_XNOR: return "~^";
            case QuarkLexerSymbol::LSHIFT: return "<<";
            case QuarkLexerSymbol::RSHIFT: return ">>";
            case QuarkLexerSymbol::ADDITION_ASSIGN: return "+=";
            case QuarkLexerSymbol::SUBTRACTION_ASSIGN: return "-=";
            case QuarkLexerSymbol::MULTIPLICATION_ASSIGN: return "*=";
            case QuarkLexerSymbol::DIVISION_ASSIGN: return "/=";
            case QuarkLexerSymbol::MODULUS_ASSIGN: return "%=";
            case QuarkLexerSymbol::BITWISE_AND_ASSIGN: return "&=";
            case QuarkLexerSymbol::BITWISE_OR_ASSIGN: return "|=";
            case QuarkLexerSymbol::BITWISE_XOR_ASSIGN: return "^=";
            case QuarkLexerSymbol::LSHIFT_ASSIGN: return "<<=";
            case QuarkLexerSymbol::RSHIFT_ASSIGN: return ">>=";
            case QuarkLexerSymbol::COND_AND: return "&&";
            case QuarkLexerSymbol::COND_OR: return "||";
            case QuarkLexerSymbol::COND_IMPLICATION: return "->";
            case QuarkLexerSymbol::COND_EQUIVALENCE: return "<->";
            case QuarkLexerSymbol::LESS_EQUAL: return "<=";
            case QuarkLexerSymbol::GREATER_EQUAL: return ">=";
            case QuarkLexerSymbol::EQUAL: return "==";
            case QuarkLexerSymbol::NOT_EQUAL: return "!=";
            case QuarkLexerSymbol::INCREMENT: return "++";
            case QuarkLexerSymbol::DECREMENT: return "--";
            case QuarkLexerSymbol::DOUBLE_COLON: return "::";
            case QuarkLexerSymbol::LINE_COMMENT: return "//";
            case QuarkLexerSymbol::BLOCK_COMMENT: return "/*";
            default: return "";
        }
    }
}

int main(int argc, char const *argv[])
{
    std::cout << "Reading file...\n\n";

    std::ifstream myfile ("test.qk", std::ios::binary);

    QuarkLexer l(myfile);
    l.lex([](QuarkLexerToken token) {
        switch (token.type) {
            case QuarkLexerTokenType::KEYWORD:
                std::cout << "Keyword:: " << token.id << '\n';
                break;
            case QuarkLexerTokenType::IDENTIFIER:
                std::cout << "Identifier:: " << token.id << '\n';
                break;
            case QuarkLexerTokenType::NUMBER:
                std::cout << "Number:: Base: " << token.num.base << 
                        ", Signed: " << token.num.is_signed << 
                        ", Ambiguous: " << token.num.is_ambiguous << 
                        ", Width: " << token.num.get_width() << 
                        ", Value: " << token.num.num.get_str() <<  
                        '\n';
                break;
            case QuarkLexerTokenType::SYMBOL:
                std::cout << "Symbol:: " << get_symbol_str(token.symbol) << '\n';
                break;
            case QuarkLexerTokenType::STRING:
                std::cout << "String::" << '\n';
                break;
            case QuarkLexerTokenType::DOC_COMMENT:
                std::cout << "Doc Comment:: " << token.str << '\n';
                break;
        }
    });

    // std::cout << "\xF0\x9F\x98\x81" << '\n';
    // QuarkLexerNumber num;
    // num.insert_msb('8', 0, 0);
    // std::cout << "Pre-swap: " << num.num.get_str() << '\n';
    // num.convert_fixed_width(16, false, 0, 0);
    // std::cout << "Post-swap: " << num.num.get_str() << ", Base: " << num.base << '\n';
    // num.insert_msb('f', 0, 0);
    // num.insert_msb('f', 0, 0);
    // std::cout << num.num.get_str() << '\n';

    return 0;
}