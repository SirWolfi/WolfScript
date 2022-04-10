#include "../inc/Handlers.hpp"
#include "../inc/Commands.hpp"

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
        failed = false;
        return expr == "true" || expr == "TRUE" || expr == "1" || expr == "True";
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
}

int run_subshell(std::string sh) {
    ++Global::in_subshell;
    fs::path cr_path = Global::current;
    auto mp = Global::variables;
    auto funs = Global::functions;

    sh.erase(sh.begin());
    sh.erase(sh.begin());
    sh.pop_back();

    auto parse = IniHelper::tls::split_by(sh,{'\n','\0'},{},{},true,true,false);
    int err = run(parse);

    Global::current = cr_path;
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
    if(fun.name.back() != '~') {
        lex = replace_vars(lex);
        lex = check_subshell(lex);
    }
            
    if(lex.size() != fun.params.size()) {
        Global::err_msg = "To many/few params for function call!";
        Global::pop_scope();
        Global::error_code = 3;
    }

    std::map<std::string,std::string> mp = add;
    for(size_t i = 0; i < fun.params.size(); ++i) {
        mp[fun.params[i]] = replace_vars(lex[i]);
    }

    std::vector<std::string> r;
    if(fun.name.back() != '\'') {
        r = IniHelper::tls::split_by(fun.body,{'\n','\0'},{},{},true,true,false);
    }
    else {
        r = IniHelper::tls::split_by(replace_vars(fun.body),{'\n','\0'},{},{},true,true,false);
    }
    LOG("run process...")
    Global::push_call_stack(fun.name);
    Global::error_code = run(r,false,mp);
    Global::set_global_variable("?",std::to_string(Global::error_code));
    if(Global::error_code == 0)
        Global::pop_call_stack();
}

int run(std::vector<std::string> lines, bool main, std::map<std::string,std::string> add) {
    Global::push_scope(add);
    for(size_t i = 0; i < lines.size(); ++i) {
        if(main) {
            Global::instruction = i+1;
        }
        Global::last_line = lines[i];

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
                    Global::pop_scope();
                    Global::err_msg = "Not a known command/function/class: \"" + name + "\"";
                    Global::error_code = 3;
                    return 3;
                }
                else {
                    Function fun = cls->get_method(call_on);

                    run_function(fun,lex,cls->members);
                    LOG("cls->members.size() = " << cls->members.size())

                    auto vars = Global::cache::variable_cache;
                    Tools::merge_maps(Tools::get_vals(Tools::keys(cls->members),vars),cls->members);
                }
            }
            else {
                if(lex.size() != 1) {
                    Global::pop_scope();
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
        }
        if(Global::exit_request) {
            std::exit(0);
        }
        if(Global::pop_run_request) {
            --Global::pop_run_request;
            Global::pop_scope();
            return Global::error_code;
        }
        if(Global::loop_continue_request || Global::loop_end_request) {
            Global::pop_scope();
            return Global::error_code;
        }
    }
    Global::pop_scope();
    return Global::error_code;
}
