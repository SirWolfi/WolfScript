#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "Global.hpp"
#include "Tools.hpp"
#include "Structs.hpp"
#include "Operators.hpp"

std::string replace_vars(std::string str);

std::vector<std::string> replace_vars(std::vector<std::string> vec);

std::string handle_sexpr(std::string expr, bool& failed);

bool handle_bexpr(std::string expr, bool& failed);

int run(std::vector<std::string> lines, bool main = false, std::map<std::string,std::string> add = {}, bool new_scope = true);

int run_subshell(std::string sh);

std::string check_subshell(std::string str);

std::vector<std::string> check_subshell(std::vector<std::string> str);

size_t find(std::string name, bool& failed, bool& function);

void run_function(Function fun, std::vector<std::string> lex, std::map<std::string,std::string> add = {});

int run_file(std::string file, bool main = false);

#endif // ifndef HANDLERS_HPP