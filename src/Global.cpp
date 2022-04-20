#include "../inc/Global.hpp"

WOLF_SCRIPT_SOURCE_FILE

bool Global::in_class() { return in_class_i != 0; }

std::string Global::get_variable(std::string var, std::vector<std::string> from_namespaces) {
    if(var == "$" || var == "") {
        return var;
    }

    if(!from_namespaces.empty()) {
        std::string total_name = "";
        for(int i = from_namespaces.size()-1; i != -1; --i) {
            std::string nname = from_namespaces[i] + "." + var;
            int idx = Global::current_scope()->index;
            while(idx != -1 && !Global::get_scope(idx)->freed) {
                for(auto& i : Global::cache::saved_scopes[idx].variables) {
                    if(i.first == nname) {
                        return Global::cache::saved_scopes[idx].variables[nname];
                    }
                }
                idx = Global::cache::saved_scopes[idx].parent;
            }

            if(total_name != "") {
                total_name += "." + from_namespaces[i];
            }
            else {
                total_name += from_namespaces[i];
            }
            nname = total_name + "." + var;
            LOG("Total name:" << total_name)
            LOG("checking for variable:" << nname)
            idx = Global::current_scope()->index;
            while(idx != -1 && !Global::get_scope(idx)->freed) {
                for(auto& i : Global::cache::saved_scopes[idx].variables) {
                    if(i.first == nname) {
                        return Global::cache::saved_scopes[idx].variables[nname];
                    }
                }
                idx = Global::cache::saved_scopes[idx].parent;
            }
        }
    }

    int idx = Global::current_scope()->index;
    while(idx != -1 && !Global::get_scope(idx)->freed) {
        for(auto& i : Global::cache::saved_scopes[idx].variables) {
            if(i.first == var) {
                return i.second;
            }
        }
        idx = Global::cache::saved_scopes[idx].parent;
    }

    if(global_vars.count(var) != 0) {
        return global_vars[var];
    }

    Global::err_msg = "Undefined variable! (" + var + ")";
    Global::error_code = 7;
    throw Global::ErrorException{};
    return "";
}

void Global::set_variable(std::string name, std::string val, bool no_new, std::vector<std::string> from_namespaces) {
    LOG("Setting variable (" << name << ")")
    if(!no_new) {
        LOG("Making a new variable for scope...")
        current_scope()->variables[name] = val;
    }
    else {
        LOG("Not making a new variable...")
        if(!from_namespaces.empty()) {
            std::string total_name = "";
            for(int i = from_namespaces.size()-1; i != -1; --i) {
                std::string nname = from_namespaces[i] + "." + name;
                int idx = Global::current_scope()->index;
                while(idx != -1 && !Global::get_scope(idx)->freed) {
                    for(auto& i : Global::cache::saved_scopes[idx].variables) {
                        LOG("Checking for " << i.first << " == " << nname << " within idx:" << idx)
                        if(i.first == nname) {
                            Global::cache::saved_scopes[idx].variables[nname] = val;
                            return;
                        }
                    }
                    idx = Global::cache::saved_scopes[idx].parent;
                }

                if(total_name != "") {
                    total_name += "." + from_namespaces[i];
                }
                else {
                    total_name += from_namespaces[i];
                }
                nname = total_name + "." + name;
                LOG("Total name:" << total_name)
                LOG("checking for variable:" << nname)
                idx = Global::current_scope()->index;
                while(idx != -1 && !Global::get_scope(idx)->freed) {
                    for(auto& i : Global::cache::saved_scopes[idx].variables) {
                        if(i.first == nname) {
                            Global::cache::saved_scopes[idx].variables[nname] = val;
                            return;
                        }
                    }
                    idx = Global::cache::saved_scopes[idx].parent;
                }
            }
        }

        int idx = Global::current_scope()->index;
        while(idx != -1 && !Global::get_scope(idx)->freed) {
            for(auto& i : Global::cache::saved_scopes[idx].variables) {
                if(i.first == name) {
                    Global::cache::saved_scopes[idx].variables[name] = val;
                    return;
                }
            }
            idx = Global::cache::saved_scopes[idx].parent;
        }

        current_scope()->variables[name] = val;
    }
}

void Global::set_global_variable(std::string name, std::string val, bool hard) {
    if(!hard && global_vars.count(name) == 0) {
        global_vars[name] = val;
    }
    else if(hard) {
        global_vars[name] = val;
    }
}

bool Global::is_list(std::string list) {
    int idx = Global::current_scope()->index;
    while(idx != -1 && !Global::get_scope(idx)->freed) {
        for(auto& i : Global::cache::saved_scopes[idx].lists) {
            if(i.first == list) {
                return true;
            }
        }
        idx = Global::cache::saved_scopes[idx].parent;
    }

    if(global_lists.count(list) != 0) {
        return true;
    }
    return false;
}

bool Global::is_var(std::string var) {
    LOG("checking if " << var << " is a variable...")
    if(var != "" && var.front() == '$' && var != "$") {
        var.erase(var.begin());
    }
    if(var == "$") {
        return true;
    }
    if(var == "") {
        return false;
    }

    int idx = Global::current_scope()->index;
    while(idx != -1 && !Global::get_scope(idx)->freed) {
        for(auto& i : Global::cache::saved_scopes[idx].variables) {
            if(i.first == var) {
                return true;
            }
        }
        idx = Global::cache::saved_scopes[idx].parent;
    }

    if(global_vars.count(var) != 0) {
        LOG("Is in global scope...")
        return true;
    }
    LOG("Is not!")
    return false;
}

List Global::get_list(std::string list) {
    int idx = Global::current_scope()->index;
    while(idx != -1 && !Global::get_scope(idx)->freed) {
        for(auto& i : Global::cache::saved_scopes[idx].lists) {
            if(i.first == list) {
                return i.second;
            }
        }
        idx = Global::cache::saved_scopes[idx].parent;
    }

    if(global_lists.count(list) != 0) {
        return global_lists[list];
    }
    return List();
}

Function Global::get_function(std::string name, std::vector<std::string> from_namespaces, bool also_blocked) {
    Function* r = get_functionptr(name,from_namespaces,also_blocked);
    if(r == nullptr) {
        Function ret;
        ret.failed = true;
        return ret;
    }
    return *r;
}

Function* Global::get_functionptr(std::string name, std::vector<std::string> from_namespaces, bool also_blocked) {
    if(!from_namespaces.empty()) {
        std::string total_name = "";
        for(int i = from_namespaces.size()-1; i != -1; --i) {
            std::string nname = from_namespaces[i] + "." + name;
            int idx = Global::current_scope()->index;
            while(idx != -1 && !Global::get_scope(idx)->freed) {
                LOG("Searching function with index: " << idx)
                for(auto& i : Global::cache::saved_scopes[idx].functions) {
                    if(i.name == nname && (!i.blocked || also_blocked)) {
                        return &i;
                    }
                }
                idx = Global::cache::saved_scopes[idx].parent;
            }

            if(total_name != "") {
                total_name += "." + from_namespaces[i];
            }
            else {
                total_name += from_namespaces[i];
            }
            nname = total_name + "." + name;
            LOG("Total name:" << total_name)
            LOG("checking for function:" << nname)
            idx = Global::current_scope()->index;
            while(idx != -1 && !Global::get_scope(idx)->freed) {
                LOG("Searching function with index: " << idx)
                for(auto& i : Global::cache::saved_scopes[idx].functions) {
                    if(i.name == nname && (!i.blocked || also_blocked)) {
                        return &i;
                    }
                }
                idx = Global::cache::saved_scopes[idx].parent;
            }
        }
    }

    int idx = Global::scopes.top();
    while(idx != -1 && !Global::get_scope(idx)->freed) {
        LOG("Searching function with index: " << idx)
        for(auto& i : Global::cache::saved_scopes[idx].functions) {
            if(i.name == name && (!i.blocked || also_blocked)) {
                return &i;
            }
        }
        idx = Global::cache::saved_scopes[idx].parent;
    }

    return nullptr;
}

void Global::push_scope(std::map<std::string,std::string> add_vars, int load_idx) {
    LOG("pushing scope!")
    if(load_idx >= -1) {
        Scope scp;

        scp.variables = add_vars;
        if(!scopes.empty()) {
            scp.parent = scopes.top(); // TODO: fix!
            scp.current_file = Global::current_scope()->current_file;
        }

        for(size_t i = 0; i < cache::saved_scopes.size(); ++i) {
            if(cache::saved_scopes[i].freed) {
                scp.index = i;
                cache::saved_scopes[i] = scp;
                scopes.push(scp.index);
                return;
            }
        }

        scp.index = cache::saved_scopes.size();
        cache::saved_scopes.push_back(scp);
        scopes.push(scp.index);
    }
    else {
        scopes.push(load_idx);
        if(!add_vars.empty()) {
            Tools::merge_maps(add_vars,current_scope()->variables);
        }
    }
}

void Global::pop_scope(bool keep_save) {
    LOG("Poping scope...")
    if(!scopes.empty()) {
        LOG("poping index:" << scopes.top() << "; saved_scopes.size() == " << cache::saved_scopes.size())
        if(!keep_save) {
            cache::saved_scopes[scopes.top()].freed = true;
        }

        cache::function_cache = current_scope()->functions;
        cache::variable_cache = current_scope()->variables;
        cache::new_classes = current_scope()->classes;
        cache::class_instance_cache = current_scope()->class_instances;

        scopes.pop();
        LOG("Poped!")
    }
}

void Global::push_call_stack(std::string line) {
    call_stack.push("(" + Global::current_scope()->current_file + ") " + std::to_string(Global::current_scope()->instruction) + " | \"" + line + "\"");
}

void Global::pop_call_stack() {
    if(!call_stack.empty()) {
        call_stack.pop();
    }
}

std::vector<std::string> Global::merge_namespaces() {
    std::vector<std::string> ret;
    auto tmp = Global::running_namespace;
    while(!tmp.empty()) {
        if(tmp.top() != "") {
            ret.push_back(tmp.top());
        }
        tmp.pop();
    }
    return ret;
}


std::tuple<std::string,std::string,bool> Global::clstls::extract_class(std::string name) {
    std::string cname;
    std::string call_on;
    bool sw = false;
            
    for(auto i : name) {
        if(i == ':') {
            sw = true;
        }
        else if(sw) {
            call_on += i;
        }
        else {
            cname += i;
        }
    }

    if(!sw) {
        return std::make_tuple("","",true);
    }

    return std::make_tuple(cname,call_on,false);
}

Class* Global::clstls::get_class(std::string name) {
    if(name == "") {
        return nullptr;
    }

    int idx = Global::current_scope()->index;
    while(idx != -1 && !Global::get_scope(idx)->freed) {
        for(auto& i : Global::cache::saved_scopes[idx].classes) {
            if(i.name == name) {
                return &i;
            }
        }
        idx = Global::cache::saved_scopes[idx].parent;
    }
    return nullptr;
}

Class* Global::clstls::get_class_instance(std::string name) {
    if(name == "") {
        return nullptr;
    }

    int idx = Global::current_scope()->index;
    while(idx != -1 && !Global::get_scope(idx)->freed) {
        for(auto& i : Global::cache::saved_scopes[idx].class_instances) {
            if(i.name == name) {
                return &i;
            }
        }
        idx = Global::cache::saved_scopes[idx].parent;
    }

    LOG("Failed by finding class instance:" << name)
    return nullptr;
}

void Global::clstls::set_class_member(Class* cls, std::string name, std::string value) {
    if(cls != nullptr) {
        cls->members[name] = value;
    }
}
    
bool Global::clstls::is_class(std::string name) {
    return get_class(name) != nullptr;
}
    
bool Global::clstls::is_member(std::string name, Class* cls) {
    if(cls == nullptr) { return false; }

    for(auto i : cls->members) {
        if(i.first == name) {
            return true;
        }
    }
    return false;
}

int Global::clstls::is_method(std::string name, Class* cls) {
    if(cls == nullptr) { return 0; }
    LOG("cls's method count:" << cls->methods.size())
    for(auto i : cls->methods) {
        LOG("checking if " << name << " is a method (" << i.name << ")")
        if(i.name == name) {
            if(i.is_virtual) {
                return 2;
            }
            return 1;
        }
    }
    return 0;
}


Scope* Global::current_scope() {
    return &Global::cache::saved_scopes[Global::scopes.top()];
}

Scope* Global::get_scope(int idx) {
    if(idx <= -1) {
        return nullptr;
    }
    return &Global::cache::saved_scopes[idx];
}