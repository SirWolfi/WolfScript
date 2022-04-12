#include "../inc/Handlers.hpp"
#include "../inc/Commands.hpp"

void replace_sys_vars(std::string var, std::string& ed) {
    if(var == "__MAIN__") {
        ed += (Global::current_main_file.top() ? "true" : "false");
    }
    else if(var == "__FILE__") {
        ed += Global::current_file.top();
    }
    else if(var == "__INST__") {
        ed += std::to_string(Global::instruction.top());
    }
    else if(var == "__SCOPE__") {
        ed += std::to_string(Global::scope_deepness.top());
    }
}

std::string replace_vars(std::string str) {
    LOG("replacing vars in scope: " << str)

    std::string tmp;
    std::string ed;
    bool in = false;
    for(auto i : str) {
        if(i == '$') {
            if(in) {
                if(tmp != "") {
                    if(tmp[0] == '$') {
                        tmp.erase(tmp.begin());
                    }

                    replace_sys_vars(tmp,ed);

                    if(Global::is_var(tmp)) {
                        LOG("Is a variable! (" << tmp << ")")
                        ed += Global::get_variable(tmp);
                    }
                    else {
                        LOG("Might be a class member...")
                        auto [cname,call_on,failed] = Global::clstls::extract_class(tmp);
                        Class* cls = Global::clstls::get_class_instance(cname);
                        if(failed || cls == nullptr || !Global::clstls::is_member(call_on,cls)) {
                            LOG("Is not!")
                            ed += "";
                        }
                        else {
                            LOG("Is!")
                            ed += cls->get_member(call_on);
                        }
                    }
                    tmp = "";
                }
                else {
                    ed += "$";
                }
            }
            in = true;
        }
        else if(in && IniHelper::tls::isIn(i,{'#','+','-','~','%','&','!',';',',','<','>','|',' ','\t','\r','\n','\0',')','(','}','{','\\','"'})) {
            if(tmp != "") {
                if(tmp[0] == '$') {
                    tmp.erase(tmp.begin());
                }

                replace_sys_vars(tmp,ed);
                
                if(Global::is_var(tmp)) {
                    LOG("Is a variable! (" << tmp << ")")
                    ed += Global::get_variable(tmp);
                }
                else {
                    LOG("Might be a class member...")
                    auto [cname,call_on,failed] = Global::clstls::extract_class(tmp);
                    Class* cls = Global::clstls::get_class_instance(cname);
                    if(failed || cls == nullptr || !Global::clstls::is_member(call_on,cls)) {
                        LOG("Is not!")
                        ed += "";
                    }
                    else {
                        LOG("Is!")
                        ed += cls->get_member(call_on);
                    }
                }
                tmp = "";
            }
            in = false;
        }

        if(!in) {
            ed += i;
        }
        else {
            tmp += i;
        }
    }
    if(in) {
        if(tmp[0] == '$') {
            tmp.erase(tmp.begin());
        }

        replace_sys_vars(tmp,ed);
        if(Global::is_var(tmp)) {
            LOG("Is a variable! (" << tmp << ")")
            ed += Global::get_variable(tmp);
        }
        else {
            LOG("Might be a class member... (" << tmp << ")")
            auto [cname,call_on,failed] = Global::clstls::extract_class(tmp);
            Class* cls = Global::clstls::get_class_instance(cname);
            LOG("[" << cname << ", " << call_on << ", " << failed << "]")
            LOG("Failed:" << failed << " | cls == nullptr:" << (cls == nullptr) << " | !Global::clstls::is_member(call_on,cls):" << !Global::clstls::is_member(call_on,cls))
            if(failed || cls == nullptr || !Global::clstls::is_member(call_on,cls)) {
                LOG("Is not!")
                ed += "";
            }
            else {
                LOG("Is!")
                ed += cls->get_member(call_on);
            }
        }
        tmp = "";
    }
    return ed;
}

std::vector<std::string> replace_vars(std::vector<std::string> vec) {
    for(size_t i = 0; i < vec.size(); ++i) {
        // TODO: make this better!
        while(!vec[i].empty() && Tools::is_empty(std::string(1,vec[i].front()))) {
            vec[i].erase(vec[i].begin());
        }

        while(!vec[i].empty() && Tools::is_empty(std::string(1,vec[i].back()))) {
            vec[i].pop_back();
        }

        vec[i] = check_subshell(replace_vars(vec[i]));
        LOG("replacing variables in " << vec[i] << " => " << vec[i])
    }
    return vec;
}

std::string handle_sexpr(std::string expr, bool& failed) {
    if(!Tools::br_check(expr,'(',')')) {
        Global::err_msg = "Brace missmatch!";
        failed = true;
        return "";
    }

    expr.pop_back();
    expr.erase(expr.begin());

    auto exprv = IniHelper::tls::split_by(expr, {' ','\t'}, {}, opersnames ,true,true,true);

    if(exprv.size() == 1) {
        failed = false;
        return (expr == "true" || expr == "TRUE" || expr == "1" || expr == "True") ? "1" : "0";
    }

    if(exprv.size() != 3) {
        Global::err_msg = "Invalid expression!";
        failed = true;
        return "";
    }

    std::string left = check_subshell(replace_vars(exprv[0]));
    std::string oper = check_subshell(replace_vars(exprv[1]));
    std::string right = check_subshell(replace_vars(exprv[2]));

    LOG("handling normal expression: " << left << oper << right << " <= " << expr)

    for(auto i : operators) {
        if(i.name == oper) {
            LOG("returned: " << i.fun(left,right,failed))
            return i.fun(left,right,failed);
        }
    }
    LOG("No such operator!")
    failed = true;
    return "";
}

bool handle_bexpr(std::string expr, bool& failed) {
    if(!Tools::br_check(expr,'(',')')) {
        Global::err_msg = "Brace missmatch!";
        failed = true;
        return false;
    }

    expr.pop_back();
    expr.erase(expr.begin());

    auto exprv = IniHelper::tls::split_by(expr, {' ','\t'}, {}, {'=','!','>','<'},true,true,true);

    if(exprv.size() == 1) {
        auto check = replace_vars(expr);
        failed = false;
        return check == "true" || check == "TRUE" || check == "1" || check == "True";
    }

    if(exprv.size() != 3) {
        Global::err_msg = "Invalid expression!";
        failed = true;
        return false;
    }

    std::string left = exprv[0];
    std::string oper = exprv[1];
    std::string right = exprv[2];

    left = replace_vars(left);
    oper = replace_vars(oper);
    right = replace_vars(right);

    LOG("handling expression: " << left << oper << right << " <= " << expr)

    if(oper == "=") {
        return left == right;
    }
    if(oper == "!") {
        return left != right;
    }
    if(oper == "<") {
        bool d1 = false, d2 = false;
        if(Tools::all_numbers(left,d1) && Tools::all_numbers(right,d1)) {
            if(d1 && d2) {
                return std::stod(left) < std::stod(right);
            }
            else if(d1 && !d2) {
                return std::stod(left) < std::stoi(right);
            }
            else if(!d1 && d2) {
                return std::stoi(left) < std::stod(right);
            }
            else if(!d1 && !d2) {
                return std::stoi(left) < std::stoi(right);
            }
        }
        else {
            Global::err_msg = "Invalid expression!";
            failed = true;
            return false;
        }
    }
    if(oper == ">") {
        bool d1 = false, d2 = false;
        if(Tools::all_numbers(left,d1) && Tools::all_numbers(right,d1)) {
            if(d1 && d2) {
                return std::stod(left) > std::stod(right);
            }
            else if(d1 && !d2) {
                return std::stod(left) > std::stoi(right);
            }
            else if(!d1 && d2) {
                return std::stoi(left) > std::stod(right);
            }
            else if(!d1 && !d2) {
                return std::stoi(left) > std::stoi(right);
            }
        }
        else {
            Global::err_msg = "Invalid expression!";
            failed = true;
            return false;
        }
        return false;
    }
    failed = true;
    return false;
}

int run_subshell(std::string sh) {
    ++Global::in_subshell;
    fs::path cr_path = Global::current.top();
    auto mp = Global::variables;
    auto funs = Global::functions;

    sh.erase(sh.begin());
    sh.erase(sh.begin());
    sh.pop_back();

    auto parse = IniHelper::tls::split_by(sh,{'\n','\0'},{},{},true,true,false);
    int err = run(parse);

    Global::current.top() = cr_path;
    Global::variables = mp;
    Global::functions = funs;
    --Global::in_subshell;

    return err;
}

std::string check_subshell(std::string str) {
    if(str.size() < 3 || str[0] != '!') {
        return str;
    }

    str.erase(str.begin());

    if(Tools::br_check(str,'{','}')) {
        LOG("valid subshell!")
        return CATCH_OUTPUT(Global::error_code = run_subshell('!' + str););
    }
    return '!' + str;
}

std::vector<std::string> check_subshell(std::vector<std::string> str) {
    auto cp = str;
    for(auto& i : cp) {
        LOG("checking if " << i << "is a subshell")
        i = check_subshell(i);
    }
    return cp;
}

size_t find(std::string name, bool& failed, bool& function);

size_t find(std::string name, bool& failed, bool& function) {
    for(size_t i = 0; i < commands.size(); ++i) {
        if(commands[i].name == name) {
            failed = false;
            function = false;
            return i;
        }
        else {
            for(auto j : commands[i].aliases) {
                if(j == name) {
                    function = false;
                    failed = false;
                    return i;
                }
            }
        }
    }

    Function fun = Global::get_function(name);
    if(fun.name != "") {
        LOG("Is function!")
        function = true;
        failed = false;
        return 0;
    }
    LOG("Failed!")
    function = false;
    failed = true;
    return 0;
}

void run_function(Function fun, std::vector<std::string> lex, std::map<std::string,std::string> add) {
    LOG("calling run_function() ...")
    if(fun.name.back() != '~') {
        LOG("replacing `lex`... (Size:" << lex.size() << ")")
        lex = replace_vars(lex);
        lex = check_subshell(lex);
        LOG("New lex size:" << lex.size())
    }
            
    if(lex.size() != fun.params.size()) {
        Global::err_msg = "To many/few params for function call!";
        // Global::pop_scope();
        Global::error_code = 3;
        return;
    }
    
    LOG("adding params... (size: " << fun.params.size() << "|" << lex.size() <<")")
    std::map<std::string,std::string> mp = add;
    for(size_t i = 0; i < fun.params.size(); ++i) {
        LOG("(" << i << ") " << fun.params[i] << " is set to: " << lex[i])
        mp[fun.params[i]] = replace_vars(check_subshell(lex[i]));
    }

    Global::current_owner.push(fun.owner);

    LOG("lexing body...")
    std::vector<std::string> r;
    if(fun.name.back() != '\'') {
        r = IniHelper::tls::split_by(fun.body,{'\n','\0'},{},{},true,true,false);
    }
    else {
        r = IniHelper::tls::split_by(replace_vars(fun.body),{'\n','\0'},{},{},true,true,false);
    }
    LOG("run process...")
    Global::push_call_stack(fun.name);
    ++Global::in_function;
    Global::current_file.push(fun.from_file);
    Global::error_code = run(r,false,mp,true);
    Global::current_file.pop();
    --Global::in_function;
    Global::set_global_variable("?",std::to_string(Global::error_code));
    Global::current_owner.pop();
    if(Global::error_code == 0)
        Global::pop_call_stack();
}

int run(std::vector<std::string> lines, bool main, std::map<std::string,std::string> add,bool new_scope) {
    if(new_scope) {
        Global::push_scope(add);
    }

    for(size_t i = 0; i < lines.size(); ++i) {
        Global::instruction.top() = i+1;

        Global::last_line = lines[i];
        LOG("Running line: " << lines[i] << "\n")

        if(Tools::is_empty(lines[i])) {
            continue;
        }

        auto lex = IniHelper::tls::split_by(lines[i],{' ','\t'},{},{},true,true,true);
        std::string name = lex[0];
        lex.erase(lex.begin()); // erase name
        bool failed = false;
        bool function = false;
        auto idx = find(name,failed,function);

        if(failed) {
            Class* cls = Global::clstls::get_class(name);

            if(cls == nullptr) {
                auto [cname, call_on, failed] = Global::clstls::extract_class(name);
                cls = Global::clstls::get_class_instance(cname);
                LOG("cname:" << cname << "|call_on:" << call_on << "|failed:" << failed << "|cls_valid:" << (cls != nullptr) << "|is_method:" << Global::clstls::is_method(call_on,cls) )
                if(failed || cls == nullptr || Global::clstls::is_member(call_on,cls) || !Global::clstls::is_method(call_on,cls)) {
                    LOG("Failed");
                    if(!Global::current_owner.empty() && Global::current_owner.top() != nullptr && Global::clstls::is_method(name,Global::current_owner.top())) {
                        Function fun = Global::current_owner.top()->get_method(name);

                        run_function(fun,lex,Global::current_owner.top()->members);

                        if(Global::error_code != 0) {
                            return Global::error_code;
                        }
                        auto vars = Global::cache::variable_cache;
                        Tools::merge_maps(Tools::get_vals(Tools::keys(Global::current_owner.top()->members),vars),Global::current_owner.top()->members);
                    }
                    else {
                        if(new_scope) Global::pop_scope();
                        Global::err_msg = "Not a known command/function/class: \"" + name + "\"";
                        Global::error_code = 3;
                        return 3;
                    }
                }
                else {
                    Function fun = cls->get_method(call_on);

                    run_function(fun,lex,cls->members);
                    if(Global::error_code != 0) {
                        return Global::error_code;
                    }
                    LOG("cls->members.size() = " << cls->members.size())

                    auto vars = Global::cache::variable_cache;
                    Tools::merge_maps(Tools::get_vals(Tools::keys(cls->members),vars),cls->members);
                }
            }
            else {
                if(lex.size() != 1) {
                    if(new_scope) Global::pop_scope();
                    Global::err_msg = "Not a valid instance definition!";
                    Global::error_code = 2;
                    return 2;
                }

                std::string instance_name = lex[0];

                Class cls = *Global::clstls::get_class(name);
                cls.name = instance_name;

                
                Global::class_instances.top().push_back(cls);
            }
        }
        else if(!function) {
            Command com = commands[idx];
            if(com.replace_v) {
                LOG("replace_v is true!")
                lex = replace_vars(lex);
                lex = check_subshell(lex);
            }
            Global::push_call_stack(name);
            Global::error_code = com.fun(com.parser.parse(lex));
            Global::set_global_variable("?",std::to_string(Global::error_code));
            if(Global::error_code != 0) {
                return Global::error_code;
            }
            if(Global::error_code == 0)
                Global::pop_call_stack();
        }
        else {
            Function fun = Global::get_function(name);
            run_function(fun,lex);
            if(Global::error_code != 0) {
                return Global::error_code;
            }
        }
        if(Global::exit_request) {
            std::exit(0);
        }
        if(Global::pop_run_request) {
            --Global::pop_run_request;
            if(new_scope) Global::pop_scope();
            return Global::error_code;
        }
        if(Global::loop_continue_request || Global::loop_end_request) {
            if(new_scope) Global::pop_scope();
            return Global::error_code;
        }
    }
    if(new_scope) Global::pop_scope();
    return Global::error_code;
}

int run_file(std::string pfile, bool main, bool allow_twice) {
    if(!Global::current.empty()) {
        Global::current.push(Global::current.top().parent_path().string() + SP + fs::path(pfile).remove_filename().string());
    }   
    else {
        Global::current.push(Global::start_path + fs::path(pfile).remove_filename().string());
    }

    fs::path file = Global::current.top().string() + pfile;

    if(!fs::exists(file) && !file.has_extension()) {
        file += ".wsc";
    }

    if(!allow_twice && IniHelper::tls::isIn(file,Global::imported_files)) {
        return 0; // to not run something twice!
    }

    if(!fs::exists(file)) {
        if(!main) {
            Global::err_msg = "Trying to import non existing file: " + pfile;
            Global::error_code = 5;
            Global::current.pop();
            return 5;
        }
        else {
            std::cout << "Unknown file: " << pfile << "\n";
        }
        Global::err_msg = "File doesn't exist: " + pfile;
        Global::error_code = 5;
        Global::current.pop();
        return 5;
    }

    std::string src = Tools::read(file);
    if(Tools::is_empty(src)) {
        Global::error_code = 0;
        return 0;
    }
    auto lines = IniHelper::tls::split_by(src,{'\n','\0'},{},{},true,true,false);

    LOG("New current path:" << Global::current.top())
    Global::scope_deepness.push(0);
    Global::current_main_file.push(main);
    Global::current_file.push(file.filename());

    int ret = run(lines,main,{},main);

    Global::scope_deepness.pop();
    Global::current_main_file.pop();
    Global::current_file.pop();
    Global::current.pop();

    if(!main) {
        /*if (!Global::cache::function_cache.empty()) {
            std::string errm = "";
            Global::functions.top() = Tools::merge_functions(Global::functions.top(),Global::cache::function_cache,errm);

            if(errm != "") {
                Global::err_msg = errm;
                return 3;
            }
        }
        if (!Global::cache::new_classes.empty()) {
            std::string errm = "";
            Global::classes.top() = Tools::merge_classes(Global::classes.top(),Global::cache::new_classes,errm);

            if(errm != "") {
                Global::err_msg = errm;
                return 3;
            }
        }
        if (!Global::cache::class_instance_cache.empty()) {
            std::string errm = "";
            Global::class_instances.top() = Tools::merge_classes(Global::class_instances.top(),Global::cache::class_instance_cache,errm);

            if(errm != "") {
                Global::err_msg = errm;
                return 3;
            }
        }*/
        Global::imported_files.push_back(file);
    }

    if(ret != 0) {
        std::string lline = Tools::until_newline(Global::last_line);
        int err_size = std::to_string(Global::instruction.top()).size() + lline.size() + 7;

        std::cout << "Exited with error code: " << ret << "\n";
        std::cout << "Error message: " << Global::err_msg << "\n";
        std::cout << "Error occured in instruction " << Global::instruction.top() << "\n";
        std::cout << "\t  |" << std::string(err_size,'-') << "|\n";
        std::cout << "\t> | " << Global::instruction.top() << " - \"" << lline << "\" | <\n";
        std::cout << "\t  |" << std::string(err_size,'-') << "|\n";
        std::cout << "\nCall Stack:\n";
        auto v = Global::call_stack;
        while(!v.empty()) {
            auto cur = v.top();
            std::cout << " -> in " << cur << "\n";
            v.pop();
        }
    }
    return ret;
}
