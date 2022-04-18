#ifndef OPERATORS_HPP
#define OPERATORS_HPP

#include "Tools.hpp"
#include "Structs.hpp"
#include "Global.hpp"

#include <math.h>

WOLF_SCRIPT_HEADER_BEGIN

struct Operator {
    std::string name;
    std::string(*fun)(std::string,std::string,bool&);

    bool blocked = false;
};

struct Operator_custom {
    std::string name;
    std::string body;

    bool blocked = false;
};

namespace operator_tls {
    std::string get_type(std::string str);
    std::string remove_commas(std::string str);

    std::vector<std::string> merge_operator_names(std::vector<std::string> vec);

    Operator* get_native_operatorptr(std::string name, bool also_blocked = false);
    Operator get_native_operator(std::string name, bool also_blocked = false);

    Operator_custom* get_custom_operatorptr(std::string name, bool also_blocked = false);
    Operator_custom get_custom_operator(std::string name, bool also_blocked = false);

    bool add_operator(std::string name, std::string body); // returns on error: false
    bool valid_operator_name(std::string name);

    // 0 -> Not an operator
    // 1 -> native operator
    // 2 -> custom operator
    int is_operator(std::string name);

} // namespace operator_tls

inline const std::vector<char> opersnames = {
    '*',
    ':',
    '?',
    '!',
    '%',
    '&',
    '/',
    '+',
    '-',
    ';',
    '<',
    '>',
    '|',
    '^',
    '=',
};

inline std::vector<Operator> operators = {
    {"+",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel != typer) {
            return left + right;
        }

        if(typel == "string") {
            return left + right;
        }
        if(typel == "number" && typer == "number") {
            return std::to_string(std::stod(left) + std::stod(right));
        }
        failed = true;
        return "";
    }},
    {"-",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return std::to_string(std::stod(left) - std::stod(right));
        }
        failed = true;
        return "";
    }},
    {"*",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return std::to_string(std::stod(left) * std::stod(right));
        }
        failed = true;
        return "";
    }},
    {"/",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);
	
        if(typel == "number" && typer == "number") {
	        if(std::stod(right) == 0) {
                failed = true;
                return "";
	        }
            
            std::string end = std::to_string(std::stod(left) / std::stod(right));

            end = operator_tls::remove_commas(end);
            
            return end;
        }

        failed = true;
        return "";
    }},
    {"%",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return std::to_string(std::stoi(left) % std::stoi(right));
        }
        failed = true;
        return "";
    }},
    {"^",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return std::to_string(std::pow(std::stod(left), std::stod(right)));
        }
        failed = true;
        return "";
    }},
    {"|",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return std::to_string(std::stoi(left) | std::stoi(right));
        }
        failed = true;
        return "";
    }},
    {"&",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return std::to_string(std::stoi(left) & std::stoi(right));
        }
        failed = true;
        return "";
    }},


    {"==",[](std::string left,std::string right,bool& failed)->std::string {
        return (left == right) ? "true":"false";
    }},
    {"!=",[](std::string left,std::string right,bool& failed)->std::string {
        return left != right ? "true":"false";
    }},
    {">",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return (std::stod(left) > std::stod(right)) ? "true":"false";
        }
        failed = true;
        return "";
    }},
    {"<",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return (std::stod(left) < std::stod(right)) ? "true":"false";
        }
        failed = true;
        return "";
    }},
    {"<=",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return (std::stod(left) <= std::stod(right)) ? "true":"false";
        }
        failed = true;
        return "";
    }},
    {">=",[](std::string left,std::string right,bool& failed)->std::string {
        std::string typel = operator_tls::get_type(left), typer = operator_tls::get_type(right);

        if(typel == "number" && typer == "number") {
            return (std::stod(left) >= std::stod(right)) ? "true":"false";
        }
        failed = true;
        return "";
    }}
};

inline std::vector<Operator_custom> custom_operators {

};

WOLF_SCRIPT_HEADER_END

#endif // ifndef OPERATORS_HPP