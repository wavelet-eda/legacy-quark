#include "QuarkParser.h"

// #include <algorithm>
// #include <functional>
// #include <stdint.h>
// #include <iostream>
// #include <fstream>
// #include <stdexcept>

using namespace Quark;

QuarkParser::QuarkParser() {
    keyword_lookup = get_quark_keywords();
}

QuarkNode QuarkParser::parse(std::vector<QuarkToken> tokens) {

}

QuarkNode QuarkParser::parse(QuarkNode starting_node, std::vector<QuarkToken> tokens) {

}
