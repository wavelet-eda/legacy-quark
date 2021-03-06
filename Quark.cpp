#include "Quark.h"

using namespace Quark;

const std::string QUARK_KEYWORDS [34] = {
    "quark", "import", "module", "parameter", "fixed", "input", "output", 
    "reg", "latch", "future", "function", "return", 
    "bit", "signed", "unsigned", "safe", 
    "overflow", "concat", "clog2", "width", "length", "resize", "cast", 
    "struct", "typedef", "enum", "strict",
    "channel", "interface", 
    "if", "else", "for", "break", "continue"
};

std::set<std::string> Quark::get_quark_keywords() {
    std::set<std::string> keyword_lookup;
    for (size_t i = 0; i < (sizeof(QUARK_KEYWORDS)/sizeof(QUARK_KEYWORDS[0])); i++) {
        keyword_lookup.insert(QUARK_KEYWORDS[i]);
    }
    return keyword_lookup;
}

char QuarkCharacter::get_escaped_char() {
    switch (c) {
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
        default: throw QuarkException("Invalid escaped character in string!");
    }
    return 0;
}

int QuarkCharacter::get_numeric_value(int base) {
    int num;
    if (c >= '0' and c <= '9') {
        num = c - '0';
    } else if (c >= 'a' and c <= 'f') {
        num = c - 'a' + 10;
    } else if (c >= 'A' and c <= 'F') {
        num = c - 'A' + 10;
    } else {
        return -1;
    }

    return (num < base) ? num : -1;
}
