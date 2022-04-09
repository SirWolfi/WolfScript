
#include <filesystem>
#include <thread>
#include <string.h>


#include "../inc/Handlers.hpp"
#include "../inc/Tools.hpp"
#include "../inc/Commands.hpp"
/*
void shell() {
    Global::current = fs::current_path();
    while(true) {
        std::cout << ">> ";
        std::string inp;
        std::getline(std::cin,inp);
        if(inp == "") {
            continue;
        }
        auto lex = IniHelper::tls::split_by(inp,{' ','\t'},{},{},true,true,true);
        run(lex,true);
    }
}*/

void run_file(std::string file) {
    std::string src = Tools::read(file);
    auto lines = IniHelper::tls::split_by(src,{'\n','\0'},{},{},true,true,false);

    Global::current = fs::current_path();

    int ret = run(lines,true);

    if(ret != 0) {
        int err_size = std::to_string(Global::instruction).size() + Global::last_line.size() + 7;

        std::cout << "Exited with error code: " << ret << "\n";
        std::cout << "Error message: " << Global::err_msg << "\n";
        std::cout << "Error occured in instruction " << Global::instruction << "\n";
        std::cout << "\t  |" << std::string(err_size,'-') << "|\n";
        std::cout << "\t> | " << Global::instruction << " - \"" << Global::last_line << "\" | <\n";
        std::cout << "\t  |" << std::string(err_size,'-') << "|\n";
        std::cout << "\nCall Stack:\n";
        auto v = Global::call_stack;
        while(!v.empty()) {
            auto cur = v.top();
            std::cout << " -> in " << cur << "\n";
            v.pop();
        }
    }
}

void help_message() {
    std::cout << "WolfScript interpreter to run .ws|.wsc scripts\n"
                  << "Usage: \n"
                  << "helper [option] [file]\n\n"
                  << "Runs FILE. \n\n" // If FILE is not given, starts a WolfScript-shell.\n\n"
                  << "Options:\n"
                  << "--help, -h \t : prints this and exits\n"
                  << "--log, -l \t : Allows the programm to make a Debug.log file. Not recomended for bigger programms!\n"
                  << "\nGithub repo: https://www.github.com/SirWolfi/WolfScript \n";
}

int main(int argc, char** argv) {
    logging::file = ""; // disables logging
    CLEAR_LOG
    ArgParser parser = ArgParser()
        .enableString('"')
        .addArg("--help",ARG_TAG,{"-h"})
        .addArg("file",ARG_GET,{});

    ParsedArgs pargs = parser.parse(argv, argc);

    if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
        std::cout << pargs.error() << "\n"; 
        return 1;
    }

    if(pargs["--help"]) {
        help_message();
    }
    else if(!pargs.has("file")) {
        shell();
    }
    else if(pargs.has("file")) {
        run_file(pargs("file"));
    }
    else {
        help_message();
    }
    return 0;
}