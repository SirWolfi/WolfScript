#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Tools.hpp"
#include "Global.hpp"
#include "Operators.hpp"
#include "Structs.hpp"
#include "Handlers.hpp"

#include <thread>

inline const std::vector<Command> commands = {
    {"help",{}, ArgParser()
         .addArg("topic",ARG_GET,{"h"})
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        if(pargs.has("topic")) {
            if(pargs("topic") == "help") {
                std::cout << "Prints help\n";
            }
            else if(pargs("topic") == "inimod") {
                std::cout << "Modify/get values in InI++ files!\n"
                << "Usage:\n\n"
                << " inimod set <file> <key> <section> <value>\n"
                << " inimod get <file> <key> <section>\n";
            }
            else if(pargs("topic") == "exit") {
                std::cout << "exits the shell\n";
            }
            else if(pargs("topic") == "echo") {
                std::cout << "prints text!\n";
            }
        }
        else {
            std::cout << "List of commands:\n";
            for(auto i : commands) {
                std::cout << " - " << i.name << "\n";
            }
            std::cout << "\nTry \"help <command>\" for more informations!\n";
        }
        return 0;
    }},
    {"exit",{"q"}, ArgParser()
         .addArg("code",ARG_GET)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        int cd = 0;
        if(pargs.has("code")) {
            try {
                cd = std::stoi(pargs("code"));
            }
            catch(...) {
                Global::err_msg = "Not an valid exit code!";
                return 2;
            }
        }

        Global::exit_request = true;
        return cd;
    }},
    {"inimod",{"ini"}, ArgParser()
        .addArg("get", ARG_TAG, {},0)
        .addArg("set", ARG_TAG, {},0)
        .addArg("set1", ARG_GET, {},1)
        .addArg("set2", ARG_GET, {},2)
        .addArg("set3", ARG_GET, {},3)
        .addArg("set4", ARG_GET, {},4)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            return 2;
        }
        if(pargs.has("set")) {
            LOG("has(set) is true in command inimod!")
            if(pargs.has("set1") && pargs.has("set2") && pargs.has("set3") && pargs.has("set4")) {
                std::string file = pargs("set1");
                std::string key = pargs("set2");
                std::string section = pargs("set3");
                std::string value = pargs("set4");

                if(!fs::exists(file)) {
                    std::ofstream of;
                    of.open(file, std::ios::trunc);
                    of.close();
                }
                IniFile ifile = IniFile::from_file(file);
                if(!ifile) {
                    if(ifile.error_msg() != "Empty file\n") {
                        return 1;
                    }
                }
                ifile.set(key,IniHelper::to_element(value),section);
                ifile.to_file(file);
                return 0;
            }
            else {
                return 1;
            }
        }
        if(pargs.has("get")) {
            if(pargs.has("set1") && pargs.has("set2")) {
                std::string file = pargs("set1");
                std::string key = pargs("set2");
                std::string section;
                if(pargs.has("set3"))
                   section = pargs("set3");
                else
                    section = "Main";

                if(!fs::exists(file)) {
                    std::ofstream of;
                    of.open(file, std::ios::trunc);
                    of.close();
                }
                IniFile ifile = IniFile::from_file(file);
                if(!ifile) {
                    return 1;
                }

                std::cout << ifile.get(key,section) << "\n";
                return 0;
            }
            else {
                return 1;
            }
        }
        else {
            return 1;
        }
        return 0;
    }},
    {"echo",{}, ArgParser()
         .addArg("-nn",ARG_TAG,{})
         .setbin()
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        auto b = pargs.get_bin();
        if(!b.empty()) {
            std::string o;
            for(auto i : b){
                o += i + " ";
            }
            o.pop_back();
            std::cout << o;
        }
        if(!pargs["-nn"])
            std::cout << "\n";
        return 0;
    }},
    {"set",{}, ArgParser()
         .addArg("name",ARG_GET,{},-1,Arg::Priority::FORCE)
         .addArg("value",ARG_GET,{},-1,Arg::Priority::OPTIONAL)
         .addArg("-global",ARG_TAG,{"-g"})
         .addArg("-no-new",ARG_TAG,{"-nn"})
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::string val;
        if(pargs.has("value")) {
            val = pargs("value");        
        }
        if(pargs["-global"] && Global::disable_global_setting == 0) {
            Global::set_global_variable(pargs("name"),val);
        }
        else {
            bool nw = false;
            if(pargs["-no-new"]) {
                nw = true && Global::disable_global_setting == 0;
            }
            Global::set_variable(pargs("name"),val,nw);
        }
        return 0;
    }},
    {"varc",{}, ArgParser()
         
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        LOG("running varc")
        for(auto i : Global::variables.top()) {
            std::cout << i.first << " : " << i.second << "\n";
        }
        std::cout << "Scope: " << Global::scope_deepness << "\n";
        return 0;
    }},
    {"dir",{}, ArgParser()
         .addArg("name",ARG_GET,{},0,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        fs::path ph = Global::current;
        ph.append(pargs("name"));
        fs::create_directory(ph);
        return 0;
    }},
    {"rm",{}, ArgParser()
         .addArg("item",ARG_GET,{},0,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }

        std::cout << "Really remove \"" << pargs("item") << "\" for EVER? [y\\N]:";
        std::string inp;
        std::getline(std::cin,inp);

        if(inp == "y" || inp == "Y" || inp == "Yes" || inp == "yes") {
            try {
                fs::path ph = Global::current;
                ph.append(pargs("item"));
                fs::remove(ph);
            }
            catch(...) {
                return 4;
            }
        }
        
        return 0;
    }},
    {"errmsg",{}, ArgParser()
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::cout << Global::err_msg << "\n";
        return 0;
    }},
    {"ls",{}, ArgParser()

    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        for(auto& i : fs::directory_iterator(Global::current)) {
            std::cout << i.path().filename();
            if(fs::is_directory(i)) {
                std::cout << " \t \t(DIR)";
            }
            std::cout << "\n";
        }
        return 0;
    }},
    {"while",{}, ArgParser()
        .addArg("expr",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("command",ARG_GET,{},1,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        std::string expr = pargs("expr");
        std::string com = pargs("command");

        // std::cout << expr << " | " << com << "\n";
        
        if(!Tools::br_check(com,'{','}')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }

        com.pop_back();
        com.erase(com.begin());

        bool failed = false;
        bool check = handle_bexpr(expr,failed);
        ++Global::in_loop;
        while(check) {
            if(failed) {
                return 2; // error message already set!
            }

            auto lines = IniHelper::tls::split_by(com,{'\n','\0'},{},{},true,true,false);

            int err = run(lines);
            if(err != 0) {
                LOG("run() failed with exit code: " << err)
                return err;
            }
            if(Global::loop_end_request) {
                Global::loop_end_request = false;
                break;
            }
            if(Global::loop_continue_request) {
                Global::loop_continue_request = false;
                continue;
            }
            check = handle_bexpr(replace_vars(expr),failed);

        }
        --Global::in_loop;
        return 0;
    },false},
    {"for",{}, ArgParser()
        .addArg("var",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("in",ARG_TAG,{},1,Arg::Priority::FORCE)
        .addArg("what",ARG_GET,{},2,Arg::Priority::FORCE)
        .addArg("command",ARG_GET,{},3,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        std::string var = pargs("var");
        std::string com = pargs("command");
        std::string in = replace_vars(check_subshell(pargs("what")));
        
        if(!Tools::br_check(com,'{','}')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }

        com.pop_back();
        com.erase(com.begin());

        List l;
        if(List::is(in)) {
            l.from_string(in);
        }
        else {
            if(Global::is_list(in)) {
                l = Global::get_list(in);
            }
            else {
                for(auto i : in) {
                    l.elements.push_back(std::string(1,i));
                }
            }
        }
        l.elements = replace_vars(l.elements);

        ++Global::in_loop;
        for(auto i : l.elements) {
            auto lines = IniHelper::tls::split_by(com,{'\n','\0'},{},{},true,true,false);

            int err = run(lines,false,{{var,i}});

            if(err != 0) {
                LOG("run() failed with exit code: " << err)
                return err;
            }
            if(Global::loop_end_request) {
                Global::loop_end_request = false;
                break;
            }
            if(Global::loop_continue_request) {
                Global::loop_continue_request = false;
                continue;
            }
        }
        --Global::in_loop;
        return 0;
    },false},
    {"range",{}, ArgParser()
         .addArg("count",ARG_GET,{},0,Arg::Priority::FORCE)
         .addArg("max",ARG_GET,{},1)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        List l;

        int cnt = 0, mcnt = 0;
        try {
            cnt = std::stoi(pargs("count"));
            if(pargs.has("max")) {
                mcnt = std::stoi(pargs("max"));
            }
        }
        catch(...) {
            Global::err_msg = "Not a valid range!";
            return 2;
        }

        if(pargs.has("max")) {
            if(mcnt < cnt) {
                Global::err_msg = "Not a valid range!";
                return 2;
            }

            for(size_t i = cnt; i < mcnt; ++i) {
                l.elements.push_back(std::to_string(i));
            }
        }
        else {
            for(size_t i = 0; i < cnt; ++i) {
                l.elements.push_back(std::to_string(i));
            }
        }

        std::cout << l.to_string();

        if(Global::in_subshell == 0) {
            std::cout << "\n";
        }
        return 0;
    }},
    {"getline",{"inp"}, ArgParser()
        .addArg("var",ARG_GET,{},0,Arg::Priority::FORCE)
        .setbin()
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        auto bin = pargs.get_bin();
        std::string o;
        for(auto i : bin) {
            o += i + " ";
        }
        o.pop_back();
        std::cout << o;
        std::string inp = "";
        std::getline(std::cin,inp);
        LOG("got input, write it to " << pargs("var") << " (\"" << inp << "\")")
        Global::set_variable(pargs("var"),inp);

        return 0;
    }},
    {"pwd",{}, ArgParser()

    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::cout << Global::current;
        if(Global::in_subshell == 0) {
            std::cout << "\n";
        }
        return 0;
    }},
    {"cd",{}, ArgParser()
        .addArg("dir",ARG_GET,{},0,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        if(pargs("dir") == "..") {
            Global::current = Global::current.parent_path();
            return 0;
        }
        if(pargs("dir") == ".") {
            return 0;
        }

        if(!fs::is_directory(pargs("dir"))) {
            Global::current.append(pargs("dir"));
        }
        else {
            return 2;
        }

        return 0;
    }},
    {"substr",{"spr"}, ArgParser()
        .addArg("from",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("to",ARG_GET,{},1,Arg::Priority::FORCE)
        .addArg("var",ARG_GET,{},2,Arg::Priority::FORCE)
        .setbin()
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs || !pargs.has_bin()) {
            Global::err_msg = pargs.error();
            return 2;
        }

        auto bin = pargs.get_bin();
        std::string str;
        for(auto i : bin) {
            str += i + " ";
        }
        str.pop_back();

        int from = 0;
        int to = 0;
        try { from = std::stoi(pargs("from")); 
              to = std::stoi(pargs("to"));     }
        catch(...) { return 2; }

        Global::set_variable(pargs("var"),str.substr(from,to));

        return 0;
    }},
    {"if",{}, ArgParser()
        .addArg("expr",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("command",ARG_GET,{},1,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::string expr = pargs("expr");
        std::string com = pargs("command");
        
        if(!Tools::br_check(com,'{','}')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }

        com.pop_back();
        com.erase(com.begin());
        auto coms = IniHelper::tls::split_by(com, {'\n','\0'}, {}, {},true,true,false);

        bool failed = false;
        bool check = handle_bexpr(replace_vars(expr),failed);
        if(failed) {
            return 2; // error code already set!
        }
        if(check) {
            int err = run(coms);
            if(err != 0) {
                return err;
            }
        }
        Global::last_if_result.top() = check; // fix!
        return 0;
    },false},
    {"elif",{}, ArgParser()
        .addArg("expr",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("command",ARG_GET,{},1,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::string expr = pargs("expr");
        std::string com = pargs("command");
        
        if(!Tools::br_check(com,'{','}')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }

        com.pop_back();
        com.erase(com.begin());
        auto coms = IniHelper::tls::split_by(replace_vars(com), {'\n','\0'}, {}, {},true,true,false);

        bool failed = false;
        bool check = handle_bexpr(replace_vars(expr),failed);

        if(failed) {
            return 2; // error code already set!
        }
        if(check && !Global::last_if_result.top()) {
            Global::last_if_result.top() = true;
            int err = run(coms);
            if(err != 0) {
                return err;
            }
        }
        return 0;
    },false},
    {"else",{}, ArgParser()
        .addArg("command",ARG_GET,{},0,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::string com = pargs("command");
        
        if(!Tools::br_check(com,'{','}')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }

        com.pop_back();
        com.erase(com.begin());
        auto coms = IniHelper::tls::split_by(replace_vars(com), {'\n','\0'}, {}, {},true,true,false);

        if(!Global::last_if_result.top()) {
            Global::last_if_result.top() = true;
            int err = run(coms);
            if(err != 0) {
                return err;
            }
        }
        return 0;
    },false},
    {"sleep",{"slp"}, ArgParser()
        .addArg("time",ARG_GET,{},0,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        int t = 0;
        try { t = std::stoi(pargs("time")); }
        catch(...) { return 2; }

        std::this_thread::sleep_for(std::chrono::milliseconds(t));

        return 0;
    }},
    {"clear",{"cls"}, ArgParser()
         
    ,[](ParsedArgs pargs)->int { // return error code!
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
#ifdef _WIN32
        system("cls");
#elif __linux__
        system("clear");
#endif
        return 0;
    }},
    {"cat",{}, ArgParser()
        .addArg("file",ARG_GET,{},0,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int {
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        fs::path ph = Global::current;
        ph.append(pargs("file"));
        if(fs::exists(ph)) {
            std::cout << Tools::read(ph) << "\n";
        }
        else {
            Global::err_msg = "No such file or directory!";
            return 2;
        }

        return 0;
    }},
    {"#",{}, ArgParser()
        .setbin()
    ,[](ParsedArgs pargs)->int { // return error code!
         // comment
        return Global::error_code;
    }},
    {"func",{}, ArgParser()
        .addArg("name",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("params",ARG_GET,{},1,Arg::Priority::FORCE)
        .addArg("commands",ARG_GET,{},2,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        //
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        std::string params = pargs("params");
        std::string name = pargs("name");
        std::string com = pargs("commands");

        if(!Tools::br_check(params,'[',']')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }
        if(!Tools::br_check(com,'{','}')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }

        com.pop_back();
        com.erase(com.begin());

        Function nfun;
        nfun.name = name;
        nfun.body = com;
        
        params.pop_back();
        params.erase(params.begin());

        std::vector<std::string> pms;

        if(!Tools::is_empty(params)) {
            pms = IniHelper::tls::split_by(params,{' ', '\t','\n'},{},{','},false,true);

            std::vector<std::string> fpms;
            for(auto i : pms) {
                if(i == ",") {
                    if(fpms.empty() || fpms.back() == ",") {
                        Global::err_msg = "Invalid params format!";
                        return 2;
                    }
                }
                else {
                    if(!fpms.empty() && fpms.back() == ",") {
                        Global::err_msg = "Invalid params format!";
                        return 2;
                    }
                    fpms.push_back(i);
                }
            }
            nfun.params = fpms;
        }

        bool failed = false, fun = false;
        int idx = find(nfun.name,failed,fun);
        if(!failed) {
            auto func = Global::get_function(nfun.name);
            if(!fun) {
                Global::err_msg = "Trying to re define native command " + nfun.name + "";
                return 1;
            }
            else if(fun && func.body != nfun.body && func.params != nfun.params) {
                Global::err_msg = "Double definition of function " + nfun.name + "";
                return 1;
            }
        }

        Global::functions.top().push_back(nfun);

        return Global::error_code;
    },false},
    {"break",{}, ArgParser()

    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        if(Global::in_loop) {
            Global::loop_end_request = true;
            ++Global::pop_run_request; // pops out of loop-command list
        }
        else {
            Global::err_msg = "Can't use \"break\" outside of a loop!";
            return 1;
        }
        return 0;
    }},
    {"continue",{}, ArgParser()

    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
            Global::err_msg = pargs.error();
            return 2;
        }
        if(Global::in_loop) {
            Global::loop_continue_request = true;
            ++Global::pop_run_request; // pops out of loop-command list
        }
        else {
            Global::err_msg = "Can't use \"continue\" outside of a loop!";
            return 1;
        }
        return 0;
    }},
    {"global",{"glb"}, ArgParser()
        .addArg("var",ARG_GET,{},-1,Arg::Priority::FORCE)
        .addArg("val",ARG_GET,{},-1,Arg::Priority::FORCE)
        .addArg("-hard",ARG_TAG,{"-h"})
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        int req = false;
        if(pargs["-h"]) {
            req = true;
        }
        if(Global::disable_global_setting == 0) {
            Global::set_global_variable(pargs("var"),pargs("val"),req);
        }
        return 0;
    }},
    {"expr",{}, ArgParser()
        .addArg("expr",ARG_GET,{},0,Arg::Priority::FORCE)
        //.addArg("var",ARG_GET,{},1,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        
        bool failed = false;
        std::string erg = handle_sexpr(pargs("expr"),failed);

        if(failed) {
            return 1;
        }

        std::cout << erg;
        if(Global::in_subshell == 0)
            std::cout << "\n";

        return 0;
    },false},
    {"class",{}, ArgParser()
        .addArg("name",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("body",ARG_GET,{},1,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        Class new_class;

        new_class.name = pargs("name");
        std::string body = pargs("body");

        if(!Tools::br_check(body,'{','}')) {
            return 2;
        }

        body.erase(body.begin());
        body.pop_back();

        auto r = IniHelper::tls::split_by(body,{'\n','\0'},{},{},true,true,false);

        int err = 0;
        ++Global::disable_global_setting;
        ++Global::in_class_i;
        Global::current_class.push(&new_class);
        CATCH_OUTPUT({ // no output please ;-;
            err = run(r);
        });
        Global::current_class.pop();
        --Global::in_class_i;
        --Global::disable_global_setting;

        if(err != 0) {
            return err;
        }

        new_class.methods = Global::cache::new_defined_methods;
        new_class.members = Global::cache::new_defined_members;

        Global::cache::new_defined_methods.clear();
        Global::cache::new_defined_members.clear();
        Global::classes.top().push_back(new_class);

        return 0;
    },false},
    {"list",{}, ArgParser()
        .addArg("name",ARG_GET,{},-1,Arg::Priority::FORCE)
        .addArg("list",ARG_GET,{},-1,Arg::Priority::FORCE)
        .addArg("-global",ARG_TAG,{"-g"})
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::string list = replace_vars(check_subshell(pargs("list")));
        if(!List::is(list)) {
            Global::err_msg = "\"" + list + "\" is not a list!\n";
            return 2;
        }

        List nlist;
        nlist.from_string(list);
        LOG(">:::" << nlist.elements.size())
        if(pargs["-global"]) {
            Global::global_lists[pargs("name")] = nlist;
        }
        else {
            Global::lists.top()[pargs("name")] = nlist;
        }

        return 0;
    }},
    {"take",{}, ArgParser()
        .addArg("index",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("list",ARG_GET,{},1,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        List nlist;
        if(!List::is(pargs("list"))) {
            nlist = Global::get_list(pargs("list"));
        }
        else {
            nlist.from_string(pargs("list"));
        }

        int at = 0;
        try {
            at = std::stoi(pargs("index"));
        }
        catch(...) {
            Global::err_msg = "\"" + pargs("index") + "\" is not a valid index!\n";
            return 2;
        }

        if(at >= nlist.elements.size()) {
            Global::err_msg = "\"" + pargs("list") + "\" has no index " + pargs("index") + "\n";
            return 2;
        }

        std::cout << nlist.elements[at];

        if(Global::in_subshell == 0) {
            std::cout << "\n";
        }

        return 0;
    }},
    {"length",{}, ArgParser()
        .addArg("list",ARG_GET,{},0,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        List list;
        if(!List::is(pargs("list"))) {
            if(Global::is_list(pargs("list"))) {
                list = Global::get_list(pargs("list"));
            }
            else {
                for(auto i : pargs("list")) {
                    list.elements.push_back(std::string(1,i));
                }
            }
        }
        else {
            list.from_string(pargs("list"));
        }
        std::cout << list.elements.size();

        if(Global::in_subshell == 0) {
            std::cout << "\n";
        }
        return 0;
    }},
    {"insert",{}, ArgParser()
        .addArg("list",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("index",ARG_GET,{},1,Arg::Priority::FORCE)
        .addArg("what",ARG_GET,{},2,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        List list;

        list = Global::get_list(pargs("list"));

        int at = 0;
        try {
            at = std::stoi(pargs("index"));
        }
        catch(...) {
            Global::err_msg = "\"" + pargs("index") + "\" is not a valid index!\n";
            return 2;
        }
        if(at >= list.elements.size()) {
            Global::err_msg = "\"" + pargs("index") + "\" is not a valid index!\n";
            return 2;
        }

        list.elements.insert(list.elements.begin() + at,pargs("what"));

        std::cout << list.to_string();

        if(Global::in_subshell == 0) {
            std::cout << "\n";
        }

        return 0;
    }},
    {"put",{}, ArgParser()
        .addArg("list",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("index",ARG_GET,{},1,Arg::Priority::FORCE)
        .addArg("what",ARG_GET,{},2,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        List list;

        if(List::is(pargs("list"))) {
            list.from_string(pargs("list"));
        }
        else {
            list = Global::get_list(pargs("list"));
        }

        int at = 0;
        try {
            at = std::stoi(pargs("index"));
        }
        catch(...) {
            Global::err_msg = "\"" + pargs("index") + "\" is not a valid index!\n";
            return 2;
        }
        if(at >= list.elements.size()) {
            Global::err_msg = "\"" + pargs("index") + "\" is not a valid index!\n";
            return 2;
        }

        list.elements[at] = pargs("what");

        std::cout << list.to_string();

        if(Global::in_subshell == 0) {
            std::cout << "\n";
        }
        return 0;
    }},
    {"push",{}, ArgParser()
        .addArg("list",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("what",ARG_GET,{},1,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        List list;

        if(List::is(pargs("list"))) {
            list.from_string(pargs("list"));
        }
        else {
            list = Global::get_list(pargs("list"));
        }


        list.elements.push_back(pargs("what"));

        std::cout << list.to_string();

        if(Global::in_subshell == 0) {
            std::cout << "\n";
        }
        return 0;
    }},
    {"method",{}, ArgParser()
        .addArg("name",ARG_GET,{},0,Arg::Priority::FORCE)
        .addArg("params",ARG_GET,{},1,Arg::Priority::FORCE)
        .addArg("commands",ARG_GET,{},2,Arg::Priority::FORCE)
    ,[](ParsedArgs pargs)->int { // return error code!
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }

        if(!Global::in_class()) {
            Global::err_msg = "Can only define member inside a class!";
            return 4;
        }

        std::string params = pargs("params");
        std::string name = pargs("name");
        std::string com = pargs("commands");

        if(!Tools::br_check(params,'[',']')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }
        if(!Tools::br_check(com,'{','}')) {
            Global::err_msg = "Brace missmatch!";
            return 2;
        }

        com.pop_back();
        com.erase(com.begin());

        Function nfun;
        nfun.name = name;
        nfun.body = com;
        
        params.pop_back();
        params.erase(params.begin());

        std::vector<std::string> pms;

        if(!Tools::is_empty(params)) {
            pms = IniHelper::tls::split_by(params,{' ', '\t','\n'},{},{','},false,true);

            std::vector<std::string> fpms;
            for(auto i : pms) {
                if(i == ",") {
                    if(fpms.empty() || fpms.back() == ",") {
                        Global::err_msg = "Invalid params format!";
                        return 2;
                    }
                }
                else {
                    if(!fpms.empty() && fpms.back() == ",") {
                        Global::err_msg = "Invalid params format!";
                        return 2;
                    }
                    fpms.push_back(i);
                }
            }
            nfun.params = fpms;
        }
        Global::cache::new_defined_methods.push_back(nfun);

        return Global::error_code;
    },false},
    {"member",{}, ArgParser()
         .addArg("name",ARG_GET,{},-1,Arg::Priority::FORCE)
         .addArg("value",ARG_GET,{},-1,Arg::Priority::OPTIONAL)
    ,[](ParsedArgs pargs)->int { // return error code!
        if(!pargs) {
            Global::err_msg = pargs.error();
            return 2;
        }
        std::string val;
        if(pargs.has("value")) {
            val = pargs("value");        
        }
        LOG("set " << pargs("name") << " to " << val << " in command: member")
        Global::cache::new_defined_members[pargs("name")] = val;

        return 0;
    }},
};

#endif // ifndef COMMANDS_HPP