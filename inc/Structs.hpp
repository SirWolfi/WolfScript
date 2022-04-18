#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <string>
#include <vector>
#include <map>

#include "InI++/Inipp.hpp"
#include "ArgParser/ArgParser.h"
#include "Tools.hpp"

WOLF_SCRIPT_HEADER_BEGIN

struct Function;
struct Class;
struct List;

struct Scope {
    int parent = -1;

    int index = 0;
    std::map<std::string,std::string> variables;
    std::vector<Class> class_instances;
    std::map<std::string,List> lists;
    std::vector<Function> functions;
    std::vector<Class> classes;
    std::string current_file;
    Class* current_owner = nullptr;

    size_t deepness = 0;
    size_t instruction = 0;
    bool last_if_result = false;

    bool freed = false; // internal stuff, do not touch!
};

struct Function {
    std::string name;
    std::vector<std::string> params;
    std::string body;
    std::string from_file;
    std::vector<std::string> in_namespace;
    bool is_virtual = false;
    
    Scope scope;
    Class* owner = nullptr;

    bool failed = false;
    bool blocked = false;
};

struct Class {
    std::string name;
    std::map<std::string,std::string> members;
    std::vector<Function> methods;
    std::vector<Class> extends;
    bool is_private = false;
    std::string bind_to_file = "";


    Function get_method(std::string name);

    std::string get_member(std::string name);

    void operator=(Class cls) {
        name = cls.name;
        members = cls.members;
        methods = cls.methods;
        extends = cls.extends;
    }
};

template<typename Ty>
class AccessStack {
    std::vector<Ty> src;
public:

    inline Ty& top() {
        return src[0];
    }

    inline Ty& back() {
        return src.back();
    }

    inline Ty& operator[](size_t idx) {
        return src[idx];
    }

    inline size_t size() const {
        return src.size();
    }

    inline Ty pop() {
        Ty lst = top();
        src.erase(src.begin());
        return lst;
    }

    inline void push(Ty elem) {
        src.insert(src.begin(),elem);
    }

    inline bool empty() const {
        return size() == 0;
    }

    inline std::vector<Ty>& data() {
        return src;
    }
};

std::vector<std::string> replace_vars(std::vector<std::string> vec);
struct List {
    std::vector<std::string> elements;

    inline bool has(std::string str) {
        for(auto i : elements) {
            if(i == str) {
                return true;
            }
        }
        return false;
    }

    inline static bool is(std::string str) {
        return Tools::br_check(str,'[',']');
    }

    inline void from_string(std::string str) {
        elements.clear();
        if(!is(str)) {
            return;
        }
        str.pop_back();
        str.erase(str.begin());
        elements = replace_vars(IniHelper::tls::split_by(str,{','},{},{},true,true,true));
    }

    inline std::string to_string() {
        std::string ret = "[";
        for(auto i : elements) {
            if(Tools::is_empty(i) && i.size() != 0) {
                i = "\"" + i + "\"";
            }
            ret += i + ',';
        }
        if(ret.size() > 1) {
            ret.pop_back();
        }
        ret += ']';
        return ret;
    }
};

struct Command {
    std::string name;
    std::vector<std::string> aliases;
    ArgParser parser;
    int (*fun)(ParsedArgs pargs);
    bool replace_v = true;
    bool blocked = false;
    bool save_scope = false;
};

WOLF_SCRIPT_HEADER_END

#endif // ifndef STRUCTS_HPP