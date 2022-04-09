#include "../inc/Operators.hpp"

std::string get_type(std::string str) {
    try {
        std::stoi(str);
    }
    catch(...) {
        return "string";
    }
    return "int";
}