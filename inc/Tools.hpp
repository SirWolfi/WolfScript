#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <fstream>
#include <string.h>
#include <vector>

#include "InI++/Inipp.hpp"

class Function;

#define PRINT_VEC(vec) { std::cout << "["; for(auto i : vec) { std::cout << i << ","; } std::cout << "]\n"; }
#define CATCH_OUTPUT(code) \
            [&]()->std::string {                                \
                std::streambuf* oldBuffer = std::cout.rdbuf();  \
                std::ostringstream cath;                        \
                std::cout.rdbuf(cath.rdbuf());                  \
                code                                            \
                std::cout.rdbuf(oldBuffer);                     \
                return cath.str();                              \
            } ();                                               \

#define NOT_IN_CLASS_CHECK                                          \
    if(Global::in_class()) {                                        \
        Global::err_msg = "Can't use this command inside a class!"; \
        return 4;                                                   \
    }

#ifdef _WIN32
# define SP "\\"
#elif defined(__linux__)
# define SP "/"
#endif


namespace Tools {

    bool is_empty(std::string str);

    bool all_numbers(std::string str, bool& dot);

    bool br_check(std::string str, char open, char close);

    std::streamsize get_flength(std::ifstream& file);

    std::string read(std::string path);

    std::map<std::string,std::string> get_vals(std::vector<std::string> vec, std::map<std::string,std::string> mp);

    void merge_maps(std::map<std::string,std::string> prio1, std::map<std::string,std::string>& prio2);

    template<typename T,typename T2>
    inline std::vector<T> keys(std::map<T,T2> mp) {
        std::vector<T> ret;
        for(auto i : mp) {
            ret.push_back(i.first);
        }
        return ret;
    }

    std::vector<Function> merge_functions(std::vector<Function> f1, std::vector<Function> f2,std::string& err_msg);
}


#endif // ifndef TOOLS_HPP