#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <stack>
#include <string>
#include <filesystem>
#include <exception>

#include "Structs.hpp"

namespace fs = std::filesystem;

#define SAVE_IN_NAMESPACE_GET (Global::in_namespace.empty() ? std::vector<std::string>({}) : Global::in_namespace.top())

namespace Global {

    inline std::string version;
    inline std::string last_line = "";
    inline std::string last_file = "";

    inline std::ostream uncatch(std::cout.rdbuf());

    inline std::string start_path;
    inline int error_code = 0;
    inline bool exit_request = false;
    inline int disable_global_setting = 0;
    inline int pop_run_request = 0;
    inline std::string err_msg = "";
    inline std::map<std::string,List> global_lists;
    inline std::map<std::string,std::string> global_vars;
    inline AccessStack<fs::path> current;
    inline AccessStack<Class*> current_class;
    inline AccessStack<std::string> call_stack;
    
    inline bool loop_end_request = false;
    inline bool loop_continue_request = false;
    inline int in_loop = 0;
    inline int in_class_i = 0;
    inline int in_subshell = 0;
    inline int in_function = 0;
    inline AccessStack<std::vector<std::string>> in_namespace;
    inline AccessStack<std::string> running_namespace;
    inline AccessStack<int> current_main_file;
    inline std::vector<fs::path> imported_files;

    inline AccessStack<int> scopes; // the indexes!

    namespace cache {
        inline std::vector<Function> function_cache;
        inline std::map<std::string,std::string> variable_cache;
        inline std::vector<Function> new_defined_methods;
        inline std::map<std::string,std::string> new_defined_members;
        inline std::vector<Class> new_class_extends;
        inline std::vector<Class> new_classes;
        inline std::vector<Class> class_instance_cache;

        inline std::vector<Scope> saved_scopes;
    } // namespace cache

    bool in_class();

    std::string get_variable(std::string var, std::vector<std::string> from_namespaces = SAVE_IN_NAMESPACE_GET);

    void set_variable(std::string name, std::string val, bool no_new = false, std::vector<std::string> from_namespaces = SAVE_IN_NAMESPACE_GET);

    void set_global_variable(std::string name, std::string val, bool hard = true);

    bool is_list(std::string list);

    bool is_var(std::string var);

    List get_list(std::string list);

    Function get_function(std::string name, std::vector<std::string> from_namespaces = SAVE_IN_NAMESPACE_GET);

    void push_scope(std::map<std::string,std::string> add_vars, int load_idx = -1);

    void pop_scope(bool keep_save = false);

    void push_call_stack(std::string line);

    void pop_call_stack();

    std::vector<std::string> merge_namespaces();

    namespace clstls {
        std::tuple<std::string,std::string,bool> extract_class(std::string name);

        Class* get_class(std::string name);

        Class* get_class_instance(std::string name);

        void set_class_member(Class* cls, std::string name, std::string value);
    
        bool is_class(std::string name);
    
        // 0 = No
        // 1 = Yes
        // 2 = Yes, but virtual
        int is_method(std::string name, Class* cls);

        bool is_member(std::string name, Class* cls);

    } // namespace clstls

    class ErrorException : std::exception {
    public:
        ErrorException() {};
        // dummy class
    };

    Scope* current_scope();
    Scope* get_scope(int idx);

} // namespace Global

#endif // ifndef GLOBAL_HPP