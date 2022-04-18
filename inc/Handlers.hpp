#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "Global.hpp"
#include "Tools.hpp"
#include "Structs.hpp"
#include "Operators.hpp"

WOLF_SCRIPT_HEADER_BEGIN


std::string replace_vars(std::string str);

std::vector<std::string> replace_vars(std::vector<std::string> vec);

std::string handle_expr(std::string expr, bool& failed,bool already_called = false);

int run(std::vector<std::string> lines, bool main = false, std::map<std::string,std::string> add = {}, bool new_scope = true,bool pop_kind = false, int load_idx = -1);

int run_text(std::string text, bool main = false, std::map<std::string,std::string> add = {}, bool new_scope = true,bool pop_kind = false, int load_idx = -1);

int run_subshell(std::string sh);

std::string check_subshell(std::string str);

std::vector<std::string> check_subshell(std::vector<std::string> str);

// only = 0 -> all (commands first)
// only = 1 -> commands
// only = 2 -> functions
size_t find(std::string name, bool& failed, bool& function, bool also_blocked = false, int only = 0);

void run_function(Function fun, std::vector<std::string> lex, std::map<std::string,std::string> add = {});

int run_file(std::string file, bool main = false, bool allow_twice = false);


WOLF_SCRIPT_HEADER_END

#endif // ifndef HANDLERS_HPP