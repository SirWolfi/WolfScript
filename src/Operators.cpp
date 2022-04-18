#include "../inc/Operators.hpp"

WOLF_SCRIPT_SOURCE_FILE

std::string operator_tls::get_type(std::string str) {
    try {
        std::stod(str);
    }
    catch(...) {
        return "string";
    }
    return "number";
}

std::string operator_tls::remove_commas(std::string str) {
    bool all_zero = true;
    for(auto i : str) {
        all_zero &= (i == '0' || i == '.');
    }
    if(all_zero) {
        return "0";
    }

    for(int i = str.size()-1; i != -1; --i) {
        if(str[i] == '.') {
            str.erase(str.begin()+i);
            break;
        }
        else if(str[i] == '0') {
            str.erase(str.begin()+i);
        }
        else {
            break;
        }
    }

    return str;
}

std::vector<std::string> operator_tls::merge_operator_names(std::vector<std::string> vec) {
    std::vector<std::string> ret;
    int last_found_idx = -1;
    bool changed = false;
    bool push = true;
    for(size_t i = 0; i < vec.size(); ++i) {
        changed = false;
        push = true;
        for(auto j : opersnames) {
            if(vec[i] == std::string(1,j)) {
                if(last_found_idx != -1) {
                    ret[last_found_idx] += vec[i];
                    changed = true;
                    push = false;
                }
                else {
                    last_found_idx = ret.size();
                    push = true;
                    changed = true;
                }
                break;
            }
        }
        if(!changed) {
            last_found_idx = -1;
        }
        if(push) {
            ret.push_back(vec[i]);
        }
    }
    if(last_found_idx != -1) {
        ret[last_found_idx] += vec.back();
    }
    return ret;
}

Operator* operator_tls::get_native_operatorptr(std::string name, bool also_blocked) {
    for(auto& i : operators) {
        if(i.name == name && (!i.blocked || also_blocked)) {
            return &i;
        }
    }
    return nullptr;
}
Operator operator_tls::get_native_operator(std::string name, bool also_blocked) {
    Operator* p = get_native_operatorptr(name,also_blocked);
    if(p == nullptr) {
        return Operator{};
    }
    return *p;
}

Operator_custom* operator_tls::get_custom_operatorptr(std::string name, bool also_blocked) {
    for(auto& i : custom_operators) {
        if(i.name == name && (!i.blocked || also_blocked)) {
            return &i;
        }
    }
    return nullptr;
}
Operator_custom operator_tls::get_custom_operator(std::string name, bool also_blocked) {
    Operator_custom* p = get_custom_operatorptr(name,also_blocked);
    if(p == nullptr) {
        return Operator_custom{};
    }
    return *p;
}

bool operator_tls::add_operator(std::string name, std::string body) {
    if(!valid_operator_name(name) || is_operator(name)) {
        return false;
    }

    Operator_custom nw;
    nw.name = name;
    nw.body = body;

    custom_operators.push_back(nw);

    return true;
}

bool operator_tls::valid_operator_name(std::string name) {
    for(auto i : name) {
        switch(i) {
            default:
                return false;
            case '*':
            case ':':
            case '?':
            case '!':
            case '%':
            case '&':
            case '/':
            case '+':
            case '-':
            case ';':
            case '<':
            case '>':
            case '|':
            case '^':
            case '=':
            case '\\':
                continue;
        }
    }
    return true;
}

int operator_tls::is_operator(std::string name) {
    Operator* op;
    Operator_custom* cp;

    op = get_native_operatorptr(name);
    if(op != nullptr) {
        return 1;
    }

    cp = get_custom_operatorptr(name);
    if(cp != nullptr) {
        return 2;
    }

    return 0;
}