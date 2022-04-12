#include "../inc/Global.hpp"

bool Global::in_class() { return in_class_i != 0; }

std::string Global::get_variable(std::string var) {
    auto tmpv = variables;
    while(!tmpv.empty()) {
        auto cur = tmpv.top();
        if(cur.count(var) != 0) {
            return cur[var];
        }
        tmpv.pop();
    }

    if(global_vars.count(var) != 0) {
        return global_vars[var];
    }
    return "";
}

void Global::set_variable(std::string name, std::string val, bool no_new) {
    if(!no_new) {
        variables.top()[name] = val;
    }
    else {
        for(auto& i : variables.data()) {
            if(i.count(name) != 0) {
                i[name] = val;
                return;
            }
        }
        variables.top()[name] = val;
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
    auto tmpv = lists;

    while(!tmpv.empty()) {
        auto cur = tmpv.top();
        if(cur.count(list) != 0) {
            return true;
        }
        tmpv.pop();
    }

    if(global_lists.count(list) != 0) {
        return true;
    }
    return false;
}

bool Global::is_var(std::string var) {
    LOG("checking if " << var << " is a variable...")
    if(var != "" && var.front() == '$') {
        var.erase(var.begin());
    }
    auto tmpv = variables;
    while(!tmpv.empty()) {
        auto cur = tmpv.top();
        if(cur.count(var) != 0) {
            LOG("Is in normal scope...")
            return true;
        }
        tmpv.pop();
    }

    if(global_vars.count(var) != 0) {
        LOG("Is in global scope...")
        return true;
    }
    LOG("Is not!")
    return false;
}

List Global::get_list(std::string list) {
    auto tmpv = lists;

    while(!tmpv.empty()) {
        auto cur = tmpv.top();
        if(cur.count(list) != 0) {
            return cur[list];
        }
        tmpv.pop();
    }

    if(global_lists.count(list) != 0) {
        return global_lists[list];
    }
    return List();
}

Function Global::get_function(std::string name) {
    auto tmpv = functions;

    while(!tmpv.empty()) {
        auto cur = tmpv.top();
        for(auto i : cur) {
            if(i.name == name) {
                return i;
            }
        }
        tmpv.pop();
    }
    return Function{};
}

void Global::push_scope(std::map<std::string,std::string> add_vars) {
    LOG("Pushing scope...(" << Global::scope_deepness.top() << ")")
    ++Global::scope_deepness.top();
    variables.push(add_vars);
    functions.push({});
    classes.push({});
    class_instances.push({});
    lists.push({});
    last_if_result.push(false);
    instruction.push(0);
}

void Global::pop_scope() {
    LOG("Poping scope... (" << Global::scope_deepness.top() << ")")
    if(Global::scope_deepness.top() != 0) {
        --Global::scope_deepness.top();
    }

    if(!variables.empty()) {
        cache::variable_cache = variables.pop();
    }

    if(!class_instances.empty()) {
        Global::cache::class_instance_cache = class_instances.pop();
    }

    if(!functions.empty()) {
        cache::function_cache = functions.pop();
    }

    if(!classes.empty()) {
        Global::cache::new_classes = classes.pop();
        for(size_t i = 0; i < Global::cache::new_classes.size(); ++i) {
            if(Global::cache::new_classes[i].is_private) {
                Global::cache::new_classes.erase(Global::cache::new_classes.begin()+i);
                --i;
            }
        }
    }

    if(!lists.empty()) {
        lists.pop();
    }

    if(!last_if_result.empty()) {
        last_if_result.pop();
    }

    if(!instruction.empty()) {
        instruction.pop();
    }
}

void Global::push_call_stack(std::string line) {
    call_stack.push("(" + Global::current_file.top() + ") " + std::to_string(Global::instruction.top()) + " | \"" + line + "\"");
}

void Global::pop_call_stack() {
    if(!call_stack.empty()) {
        call_stack.pop();
    }
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

    for(auto& i : classes.data()) {
        for(auto& j : i) {
            if(j.name == name) {
                if(!j.is_private || j.bind_to_file == Global::current.top().string() + Global::current_file.top())
                return &j;
            }
        }
    }
    return nullptr;
}

Class* Global::clstls::get_class_instance(std::string name) {
    if(name == "") {
        return nullptr;
    }

    for(auto& i : class_instances.data()) {
        LOG("i.size() = " << i.size())
        for(auto& j : i) {
            LOG("Checking for:" << j.name << " == " << name)
            if(j.name == name) {
                return &j;
            }
        }
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
