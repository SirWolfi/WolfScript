#include "../inc/Structs.hpp"

Function Class::get_method(std::string name) {
    for(auto i : methods) {
        if(i.name == name) {
            return i;
        }
    }
    return Function();
}

std::string Class::get_member(std::string name) {
    for(auto i : members) {
        if(i.first == name) {
            return i.second;
        }
    }
    return "";
}