#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <stack>
#include <string>
#include <filesystem>

#include "Structs.hpp"

namespace fs = std::filesystem;

namespace Global {
    inline std::string last_line = "";
    inline size_t instruction = 0;

    inline size_t scope_deepness = 0;
    inline int error_code = 0;
    inline bool exit_request = false;
    inline int disable_global_setting = 0;
    inline int pop_run_request = 0;
    inline std::string err_msg = "";
    inline AccessStack<std::map<std::string,std::string>> variables;
    inline AccessStack<std::vector<Class>> class_instances;
    inline AccessStack<std::map<std::string,List>> lists;
    inline std::map<std::string,List> global_lists;
    inline std::map<std::string,std::string> global_vars;
    inline fs::path current;
    inline AccessStack<std::vector<Function>> functions;
    inline AccessStack<std::vector<Class>> classes;
    inline AccessStack<Class*> current_class;
    inline AccessStack<std::string> call_stack;
    inline AccessStack<int> last_if_result;
    inline bool loop_end_request = false;
    inline bool loop_continue_request = false;
    inline int in_loop = 0;
    inline int in_class_i = 0;
    inline int in_subshell = 0;

    namespace cache {
        inline std::vector<Function> function_cache;
        inline std::map<std::string,std::string> variable_cache;
        inline std::vector<Function> new_defined_methods;
        inline std::map<std::string,std::string> new_defined_members;
    }

    bool in_class();

    std::string get_variable(std::string var);

    void set_variable(std::string name, std::string val, bool no_new = false);

    void set_global_variable(std::string name, std::string val, bool hard = true);

    bool is_list(std::string list);

    bool is_var(std::string var);

    List get_list(std::string list);

    Function get_function(std::string name);

    void push_scope(std::map<std::string,std::string> add_vars);

    void pop_scope();

    void push_call_stack(std::string line);

    void pop_call_stack();

    namespace clstls {
        std::tuple<std::string,std::string,bool> extract_class(std::string name);

        Class* get_class(std::string name);

        Class* get_class_instance(std::string name);

        void set_class_member(Class* cls, std::string name, std::string value);
    
        bool is_class(std::string name);
    
        bool is_member(std::string name, Class* cls);

        bool is_method(std::string name, Class* cls);
    } // namespace clstls
} // namespace Global

#endif // ifndef GLOBAL_HPP