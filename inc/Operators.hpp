#ifndef OPERATORS_HPP
#define OPERATORS_HPP

#include "Tools.hpp"
#include "Structs.hpp"
#include "Global.hpp"

struct Operator {
    std::string name;
    std::string(*fun)(std::string,std::string,bool&);
};

std::string get_type(std::string str);

inline std::vector<char> opersnames = {
    '+',
    '-'
};

inline std::vector<Operator> operators = {
    {"+",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = get_type(left), typer = get_type(right);

        if(typel != typer) {
            return left + right;
        }

        if(typel == "string") {
            return left + right;
        }
        if(typel == "int" && typer == "int") {
            return std::to_string(std::stoi(left) + std::stoi(right));
        }
        failed = true;
        return "";
    }},
    {"-",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = get_type(left), typer = get_type(right);

        if(typel == "int" && typer == "int") {
            return std::to_string(std::stoi(left) - std::stoi(right));
        }
        failed = true;
        return "";
    }}
};



#endif // ifndef OPERATORS_HPP