#include "../inc/Handlers.hpp"
#include "../inc/Commands.hpp"

void replace_sys_vars(std::string var, std::string& ed) {
    if(var == "__MAIN__") {
        ed += (Global::current_main_file.top() ? "true" : "false");
    }
    else if(var == "__FILE__") {
        ed += Global::current_scope()->current_file;
    }
    else if(var == "__INST__") {
        ed += std::to_string(Global::current_scope()->instruction);
    }
    else if(var == "__SCOPE__") {
        ed += std::to_string(Global::current_scope()->deepness);
    }
    else if(var == "__SCOPE_IDX__") {
        ed += std::to_string(Global::current_scope()->index);
    }
    else if(var == "__NAMESPACE__") {
        if(!Global::in_namespace.empty() && !Global::in_namespace.top().empty()) {
            for(int i = Global::in_namespace.top().size()-1; i != -1; --i) {
                ed += Global::in_namespace.top().at(i) + ".";
            }
            ed.pop_back();
        }
    }
}

static std::string getvrs(std::string tmp) {
    LOG("getvrs with:" << tmp)
    std::string ed;
    if(tmp[0] == '$') {
        tmp.erase(tmp.begin());
    }

    std::string old_ed = ed;
    replace_sys_vars(tmp,ed);

    if(ed != old_ed) {
        return ed;
    }

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
            LOG("Might be a fellow namespace variable...")

            if(!Global::in_namespace.empty() && !Global::in_namespace.top().empty()) {
                std::string name;
                std::string total_name;
                for(int i = Global::in_namespace.top().size()-1; i != -1; --i) {
                    LOG("Looping number:" << i)
                    name = Global::in_namespace.top()[i] + "." + tmp;
                    if(Global::is_var(name)) {
                        LOG("Is a variable! (" << name << ")")
                        ed += Global::get_variable(name);
                        break;
                    }
                    else {
                        LOG("Might be a class member...")
                        auto [cname,call_on,failed] = Global::clstls::extract_class(name);
                        Class* cls = Global::clstls::get_class_instance(cname);
                        if(failed || cls == nullptr || !Global::clstls::is_member(call_on,cls)) {
                            LOG("Is not!")
                            ed += "";
                        }
                        else {
                            LOG("Is!")
                            ed += cls->get_member(call_on);
                            break;
                        }
                    }
                    name = "";

                    if(total_name != "") {
                        total_name += "." + Global::in_namespace.top()[i];
                    }
                    else {
                        total_name += Global::in_namespace.top()[i];
                    }
                    name = total_name  + "." + tmp;
                    LOG("Checking for:" << name)
                    if(Global::is_var(name)) {
                        LOG("Is a variable! (" << name << ")")
                        ed += Global::get_variable(name);
                        break;
                    }
                    else {
                        LOG("Might be a class member...")
                        auto [cname,call_on,failed] = Global::clstls::extract_class(name);
                        Class* cls = Global::clstls::get_class_instance(cname);
                        if(failed || cls == nullptr || !Global::clstls::is_member(call_on,cls)) {
                            LOG("Is not!")
                            Global::err_msg = "Undefined variable! (" + tmp + ")";
                            Global::error_code = 7;
                            throw Global::ErrorException{};
                            ed += "";
                        }
                        else {
                            LOG("Is!")
                            ed += cls->get_member(call_on);
                            break;
                        }
                    }
                }
            }
            else {
                LOG("Is not!")
                ed += "";
                Global::err_msg = "Undefined variable! (" + tmp + ")";
                Global::error_code = 7;
                throw Global::ErrorException{};
            }
        }
        else {
            LOG("Is!")
            ed += cls->get_member(call_on);
        }
    }
    return ed;
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
                    ed += getvrs(tmp);
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
                ed += getvrs(tmp);
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
        ed += getvrs(tmp);
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

std::string handle_sexpr(std::string expr, bool& failed, bool already_called) {
    if(!Tools::br_check(expr,'(',')')) {
        Global::err_msg = "Brace missmatch!";
        failed = true;
        return "";
    }
    expr.pop_back();
    expr.erase(expr.begin());

    auto exprv = IniHelper::tls::split_by(expr, {' ','\t'}, {}, opersnames ,true,true,true);

    for(size_t i = 0; i < exprv.size(); ++i) {
        if(exprv[i] == "!") {
            if(i+2 != exprv.size() && Tools::br_check(exprv[i+1],'{','}')) {
                exprv[i+1] = "!" + exprv[i+1];
                exprv.erase(exprv.begin()+i);
                --i;
                continue;
            }
        }
    }

    if(exprv.size() == 1) {
        if(already_called) {
            failed = true;
            return "";
        }
        failed = false;
        expr = check_subshell(replace_vars(expr));
        return handle_sexpr("(" + expr + ")",failed,true);
    }

    if(exprv.size() != 3) {
        Global::err_msg = "Invalid expression!";
        failed = true;
        return "";
    }

    std::string left = exprv[0];
    std::string right = exprv[2];
    if(Tools::br_check(left,'(',')')) {
        left = handle_sexpr(left,failed);
    }
    if(Tools::br_check(left,'(',')')) {
        right = handle_sexpr(right,failed);
    }

    left = check_subshell(replace_vars(exprv[0]));
    std::string oper = check_subshell(replace_vars(exprv[1]));
    right = check_subshell(replace_vars(exprv[2]));

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

    for(size_t i = 0; i < exprv.size(); ++i) {
        if(exprv[i] == "!") {
            LOG("Found a \"!\"" << " (" << i << "+1 != " << exprv.size() << "-1 )")
            if(i+1 != exprv.size() && Tools::br_check(exprv[i+1],'{','}')) {
                exprv[i+1] = "!" + exprv[i+1];
                exprv.erase(exprv.begin()+i);
                --i;
                continue;
            }
            LOG("Not for a subshell...")
        }
    }

    if(exprv.size() == 1) {
        auto check = replace_vars(expr);
        failed = false;
        return check == "true" || check == "TRUE" || check == "1" || check == "True";
    }

    if(exprv.size() != 3) {
        LOG("To big/not long enough expression. Size:" << exprv.size())
        Global::err_msg = "Invalid expression!";
        failed = true;
        return false;
    }

    std::string left = exprv[0];
    std::string right = exprv[2];
    if(Tools::br_check(left,'(',')')) {
        left = handle_sexpr(left,failed);
    }
    if(Tools::br_check(left,'(',')')) {
        right = handle_sexpr(right,failed);
    }

    left = check_subshell(replace_vars(exprv[0]));
    std::string oper = check_subshell(replace_vars(exprv[1]));
    right = check_subshell(replace_vars(exprv[2]));

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
    auto mp = Global::current_scope()->variables;
    auto funs = Global::current_scope()->functions;

    sh.erase(sh.begin());
    sh.erase(sh.begin());
    sh.pop_back();

    auto parse = IniHelper::tls::split_by(sh,{'\n','\0'},{},{},true,true,false);
    int err = run(parse);

    Global::current.top() = cr_path;
    Global::current_scope()->variables = mp;
    Global::current_scope()->functions = funs;
    --Global::in_subshell;

    if(err != 0) {
        Global::err_msg = "Subshell failed!";
        Global::error_code = err;
        throw Global::ErrorException{};
    }
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
    if(!fun.failed) {
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
        int old_size = lex.size();
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

    Class* owner_save = Global::current_scope()->current_owner;
    Global::current_scope()->current_owner = fun.owner;

    LOG("lexing body...")
    std::vector<std::string> r;

    r = IniHelper::tls::split_by(fun.body,{'\n','\0'},{},{},true,true,false);

    LOG("run process...")
    Global::push_call_stack(fun.name);
    ++Global::in_function;
    std::string file_save = Global::current_scope()->current_file;
    Global::current_scope()->current_file = fun.from_file;
    Global::in_namespace.push(fun.in_namespace);
    int idx_save = Global::scopes.top();
    Global::scopes.top() = fun.scope.index;

    Global::error_code = run(r,false,mp,true,fun.scope.index);
    
    Global::scopes.top() = idx_save;
    Global::in_namespace.pop();
    Global::current_scope()->current_file = file_save;
    --Global::in_function;
    Global::set_global_variable("?",std::to_string(Global::error_code));
    Global::current_scope()->current_owner = owner_save;
    if(Global::error_code == 0)
        Global::pop_call_stack();
}

int run(std::vector<std::string> lines, bool main, std::map<std::string,std::string> add,bool new_scope, bool pop_kind, int load_idx) {
    if(new_scope) {
        Global::push_scope(add,load_idx);
        if(main) {
            Global::current_scope()->current_file = Global::last_file;
        }
    }

    for(size_t i = 0; i < lines.size(); ++i) {
        Global::current_scope()->instruction = i+1;

        Global::last_line = lines[i];
        LOG("Running line: " << lines[i] << "\n")

        if(Tools::is_empty(lines[i])) {
            continue;
        }

        auto lex = IniHelper::tls::split_by(lines[i],{' ','\t'},{},{},true,true,true);
        std::string name = lex[0];
        lex.erase(lex.begin()); // erase name

        bool changed = true;
        while (changed) {
            std::string old_name = name;
            if(name[0] == '$') {
                name = replace_vars(name);
            }
            if(name[0] == '!') {
                name = check_subshell(name);
            }
            changed = old_name != name;
        }

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
                    if(Global::current_scope()->current_owner != nullptr && Global::clstls::is_method(name,Global::current_scope()->current_owner)) {
                        Function fun = Global::current_scope()->current_owner->get_method(name);

                        run_function(fun,lex,Global::current_scope()->current_owner->members);

                        if(Global::error_code != 0) {
                            return Global::error_code;
                        }
                        auto vars = Global::cache::variable_cache;
                        Tools::merge_maps(Tools::get_vals(Tools::keys(Global::current_scope()->current_owner->members),vars),Global::current_scope()->current_owner->members);
                    }
                    else {
                        if(new_scope) Global::pop_scope(pop_kind);
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
                    if(new_scope) Global::pop_scope(pop_kind);
                    Global::err_msg = "Not a valid instance definition!";
                    Global::error_code = 2;
                    return 2;
                }

                std::string instance_name = lex[0];

                Class cls = *Global::clstls::get_class(name);
                cls.name = instance_name;

                
                Global::current_scope()->class_instances.push_back(cls);
            }
        }
        else if(!function) {
            Command com = commands[idx];
            if(com.replace_v) {
                LOG("replace_v is true!")
                lex = replace_vars(lex);
                lex = check_subshell(lex);
            }
            Global::push_call_stack(Tools::until_newline(lines[i]));
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
            if(new_scope) Global::pop_scope(pop_kind);
            return Global::error_code;
        }
        if(Global::loop_continue_request || Global::loop_end_request) {
            if(new_scope) Global::pop_scope(pop_kind);
            return Global::error_code;
        }
    }
    if(new_scope) Global::pop_scope(pop_kind);
    return Global::error_code;
}

void error_message(int code) {
    std::string lline = Tools::until_newline(Global::last_line);
    int err_size = std::to_string(Global::current_scope()->instruction).size() + lline.size() + 7;

    std::cout << "Exited with error code: " << code << "\n";
    std::cout << "Error message: " << Global::err_msg << "\n";
    std::cout << "Error occured in instruction " << Global::current_scope()->instruction << "\n";
    std::cout << "\t  |" << std::string(err_size,'-') << "|\n";
    std::cout << "\t> | " << Global::current_scope()->instruction << " - \"" << lline << "\" | <\n";
    std::cout << "\t  |" << std::string(err_size,'-') << "|\n";
    std::cout << "\nCall Stack: (Most recent call first)\n";
    auto v = Global::call_stack;
    while(!v.empty()) {
        auto cur = v.top();
        std::cout << " -> in " << cur << "\n";
        v.pop();
    }
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
    std::string file_save;
    int scope_save = 0;
    if(!main) {
        scope_save = Global::current_scope()->deepness;
        Global::current_scope()->deepness = 0;
        file_save = Global::current_scope()->current_file;
        Global::current_scope()->current_file = file.filename();
    }
    else {
        Global::last_file = file.filename();
    }
    Global::current_main_file.push(main);

    int ret = 0;
    try {
        ret = run(lines,main,{},main);
        LOG("Finished with running file!")
    }
    catch(Global::ErrorException& err) {
        if(!main) {
            Global::current_scope()->deepness = scope_save;
            Global::current_scope()->current_file = file_save;
            Global::current.pop();
        }
        error_message(Global::error_code);
        Global::current_main_file.pop();
        return Global::error_code;
    }
    if(!main) {
        LOG("!main is true, resseting necessary...")
        Global::current_scope()->deepness = scope_save;
        Global::current_main_file.pop();
        Global::current_scope()->current_file = file_save;
        Global::current.pop();

        Global::imported_files.push_back(file);
    }
    else {
        Global::current_main_file.pop();
    }

    if(ret != 0) {
        error_message(ret);
    }
    return ret;
}
