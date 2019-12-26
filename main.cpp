#include <iostream>
#include <fstream>
#include <string>
#include <chrono> 
#include <ctime>

#include "Quark.h"
#include "QuarkLexer.h"

using namespace Quark;

std::string get_symbol_str(QuarkSymbol sym) {

    if ((uint8_t) sym >= 32 and (uint8_t) sym < 128) {
        std::string result;
        result += (char) sym;
        return result;
    } else {
        switch (sym) {
            case QuarkSymbol::BITWISE_NAND: return "~&";
            case QuarkSymbol::BITWISE_NOR: return "~|";
            case QuarkSymbol::BITWISE_XNOR: return "~^";
            case QuarkSymbol::LSHIFT: return "<<";
            case QuarkSymbol::RSHIFT: return ">>";
            case QuarkSymbol::ADDITION_ASSIGN: return "+=";
            case QuarkSymbol::SUBTRACTION_ASSIGN: return "-=";
            case QuarkSymbol::MULTIPLICATION_ASSIGN: return "*=";
            case QuarkSymbol::DIVISION_ASSIGN: return "/=";
            case QuarkSymbol::MODULUS_ASSIGN: return "%=";
            case QuarkSymbol::BITWISE_AND_ASSIGN: return "&=";
            case QuarkSymbol::BITWISE_OR_ASSIGN: return "|=";
            case QuarkSymbol::BITWISE_XOR_ASSIGN: return "^=";
            case QuarkSymbol::LSHIFT_ASSIGN: return "<<=";
            case QuarkSymbol::RSHIFT_ASSIGN: return ">>=";
            case QuarkSymbol::COND_AND: return "&&";
            case QuarkSymbol::COND_OR: return "||";
            case QuarkSymbol::COND_IMPLICATION: return "->";
            case QuarkSymbol::COND_EQUIVALENCE: return "<->";
            case QuarkSymbol::LESS_EQUAL: return "<=";
            case QuarkSymbol::GREATER_EQUAL: return ">=";
            case QuarkSymbol::EQUAL: return "==";
            case QuarkSymbol::NOT_EQUAL: return "!=";
            case QuarkSymbol::INCREMENT: return "++";
            case QuarkSymbol::DECREMENT: return "--";
            case QuarkSymbol::DOUBLE_COLON: return "::";
            case QuarkSymbol::LINE_COMMENT: return "//";
            case QuarkSymbol::BLOCK_COMMENT: return "/*";
            default: return "";
        }
    }
}

int main(int argc, char const *argv[])
{
    std::cout << "Reading file...\n\n";


    auto start_time = std::chrono::high_resolution_clock::now();

    std::ifstream myfile (argv[1], std::ios::binary);

    QuarkLexer l(myfile);
    l.lex([](QuarkToken token) {
        switch (token.type) {
            case QuarkTokenType::KEYWORD:
                std::cout << "Keyword:: " << token.id << '\n';
                break;
            case QuarkTokenType::IDENTIFIER:
                std::cout << "Identifier:: " << token.id << '\n';
                break;
            case QuarkTokenType::NUMBER:
                std::cout << "Number:: Base: " << token.num.base << 
                        ", Signed: " << token.num.is_signed << 
                        ", Ambiguous: " << token.num.is_ambiguous << 
                        ", Width: " << token.num.get_width() << 
                        ", Value: " << token.num.num.get_str() <<  
                        '\n';
                break;
            case QuarkTokenType::SYMBOL:
                std::cout << "Symbol:: " << get_symbol_str(token.symbol) << '\n';
                break;
            case QuarkTokenType::STRING:
                std::cout << "String::" << '\n';
                break;
            case QuarkTokenType::DOC_COMMENT:
                std::cout << "Doc Comment:: " << token.str << '\n';
                break;
        }
    });

    // auto duration = ;

    auto ms = 
            std::chrono::duration_cast<
            std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start_time);
    // std::time_t duration_t = std::chrono::system_clock::to_time_t(duration);

    std::cout << ms.count() << '\n';

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