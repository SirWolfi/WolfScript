#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <fstream>
#include <string.h>
#include <vector>

#include "InI++/Inipp.hpp"

#define WOLF_SCRIPT_HEADER_BEGIN namespace WolfScript {
#define WOLF_SCRIPT_HEADER_END }

#define WOLF_SCRIPT_SOURCE_FILE using namespace WolfScript;

#define WS_PRINT_VEC(vec) { WolfScript::Global::uncatch << "["; for(auto i : vec) { WolfScript::Global::uncatch << i << ","; } WolfScript::Global::uncatch << "]\n"; }
#define WS_CATCH_OUTPUT(code) \
            [&]()->std::string {                                \
                std::streambuf* oldBuffer = std::cout.rdbuf();  \
                std::ostringstream cath;                        \
                std::cout.rdbuf(cath.rdbuf());                  \
                code                                            \
                std::cout.rdbuf(oldBuffer);                     \
                return cath.str();                              \
            } ();                                               \

#define WS_IN_CLASS_CHECK(bl)                                           \
    if(WolfScript:: Global::in_class() == !bl) {                        \
        if(!bl)                                                         \
            Global::err_msg = "Can't use this command inside a class!"; \
        else                                                            \
            Global::err_msg = "Can't use this command outside a class!";\
        return 4;                                                       \
    }

#ifdef _WIN32
# define SP "\\"
#elif defined(__linux__)
# define SP "/"
#else
# define SP "/"
# define WS_UNKNOWN_OS
#endif


WOLF_SCRIPT_HEADER_BEGIN

struct Function;
struct Class;

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

    std::string until_newline(std::string);

    std::vector<Function> merge_functions(std::vector<Function> f1, std::vector<Function> f2,std::string& err_msg,bool lookfor_virtual = false);
    std::vector<Class> merge_classes(std::vector<Class> f1, std::vector<Class> f2, std::string& err_msg);

    bool is_true(std::string str);
    bool is_false(std::string str);
}

WOLF_SCRIPT_HEADER_END

#endif // ifndef TOOLS_HPP