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

void help_message() {
    std::cout << "WolfScript interpreter to run .ws|.wsc scripts. (Version:" << WolfScript::Global::version << ")\n"
                  << "Usage: \n"
                  << "helper [option] [file]\n\n"
                  << "Runs FILE. \n\n" // If FILE is not given, starts a WolfScript-shell.\n\n"
                  << "Options:\n"
                  << "--help, -h \t : prints this and exits\n"
                  << "--log, -l \t : Allows the programm to make a Debug.log file. Not recomended for bigger programms!\n"
                  << "\nGithub repo: https://www.github.com/SirWolfi/WolfScript \n";
}

int main(int argc, char** argv) {
    WolfScript::Global::version = WolfScript::Tools::read("VERSION.txt");

    logging::file = ""; // disables logging
    CLEAR_LOG
    ArgParser parser = ArgParser()
        .enableString('"')
        .addArg("--help",ARG_TAG,{"-h"})
        .addArg("--log",ARG_TAG,{"-l"})
        .addArg("file",ARG_GET,{});

    ParsedArgs pargs = parser.parse(argv, argc);

    if(!pargs && pargs != ArgParserErrors::NO_ARGS) {
        std::cout << pargs.error() << "\n"; 
        return 1;
    }

    if(pargs["--log"]) {
        logging::file = "Debug.log";
        CLEAR_LOG
    }
    else if(pargs["--help"]) {
        help_message();
    }
    //else if(!pargs.has("file")) {
        //shell();
    //}
    else if(pargs.has("file")) {
        fs::path file = std::filesystem::current_path().string() + SP + std::filesystem::path(pargs("file")).remove_filename().string();
        WolfScript::Global::start_path = file;
        WolfScript::run_file(fs::path(pargs("file")).filename().string(),true);
    }
    else {
        help_message();
    }
    return 0;
}