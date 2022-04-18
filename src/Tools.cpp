#include "../inc/Tools.hpp"
#include "../inc/Structs.hpp"

WOLF_SCRIPT_SOURCE_FILE

bool Tools::is_empty(std::string str) {
    if(str == "") {
        return true;
    }
    for(auto i : str) {
        if(!IniHelper::tls::isIn(i,{' ','\t','\r','\n'})) {
            return false;
        }
    }
    return true;
}

bool Tools::all_numbers(std::string str, bool& dot) {
    dot = false;
    for(auto i : str) {
        if(!IniHelper::tls::isIn<char>(i,{'0','1','2','3','4','5','6','7','8','9'})) {
            if(i == '.') {
                if(dot) {
                    return false;
                }
                dot = true;
                continue;
            }
            return false;
        }
    }
    return true;
}

bool Tools::br_check(std::string str, char open, char close) {
    int br_count = 0;
    if(str.size() < 2) {
        return false;
    }
    for(size_t i = 0; i < str.size(); ++i) {
        if(str[i] == open) {
            ++br_count;
        }
        if(str[i] == close) {
        --br_count;
        }

        if(br_count == 0 && i+1 != str.size()) {
            return false;
        }
    }
    if(br_count == 0) {
        return true;
    }
    return false;
}

std::streamsize Tools::get_flength(std::ifstream& file) {
    if(!file.is_open()) {
        return 0;
    }
    std::streampos temp_1 = file.tellg();
    file.seekg(0, std::fstream::end);
    std::streampos temp_2 = file.tellg();
    file.seekg(temp_1, std::fstream::beg);

    return temp_2;
}

std::string Tools::read(std::string path) {
    std::ifstream ifile;
    ifile.open(path, std::ios::binary);
    std::streamsize len = get_flength(ifile);
    char* dummy = new char[len];

    try {
        ifile.read(dummy, len);
    }
    catch(std::exception& err) {
        ifile.close();
        delete[] dummy;
        dummy = nullptr;
        return "";
    }
    if (dummy == nullptr || strlen(dummy) == 0) {
        ifile.close();
        delete[] dummy;
        dummy = nullptr;
        return "";
    }
    ifile.close();
    //dummy += '\0';
    std::string re;
    re.assign(dummy, len);

    delete[] dummy;
    dummy = nullptr;

    return re;
}

std::map<std::string,std::string> Tools::get_vals(std::vector<std::string> vec, std::map<std::string,std::string> mp) {
    std::map<std::string,std::string> ret_map;
    for(auto i : mp) {
        if(IniHelper::tls::isIn(i.first,vec)) {
            ret_map[i.first] = i.second;
        }
    }

    return ret_map;
}

void Tools::merge_maps(std::map<std::string,std::string> prio1, std::map<std::string,std::string>& prio2) {
    for(auto i : prio1) {
        if(i.second != "") {
            prio2[i.first] = i.second;
        }
    }
}

std::string Tools::until_newline(std::string str) {
    std::string ret;
    bool found_normal_ch = false;
    for(auto i : str) {
        if(!found_normal_ch && (i == ' ' || i == '\t')) {
            continue;
        }
        else if(i == '\n') {
            break;
        }
        else {
            found_normal_ch = true;
        }
        ret += i;
    }
    return ret;
}

std::vector<Function> Tools::merge_functions(std::vector<Function> f1, std::vector<Function> f2, std::string& err_msg, bool lookfor_virtual) {
    auto ret = f1;
    for(auto i : f2) {
        for(size_t j = 0; j < ret.size(); ++j) {
            if(i.name == ret[j].name) {
                if(lookfor_virtual && ret[j].is_virtual) {
                    ret.erase(ret.begin()+j);
                    --j;
                }
                else {
                    err_msg = "Double definition of function: " + i.name;
                    return std::vector<Function>();
                }
            }
        }
        ret.push_back(i);
    }
    err_msg = "";
    return ret;
}

std::vector<Class> Tools::merge_classes(std::vector<Class> f1, std::vector<Class> f2, std::string& err_msg) {
    auto ret = f1;
    for(auto i : f2) {
        for(auto j : ret) {
            if(i.name == j.name) {
                err_msg = "Double definition of: " + i.name;
                return std::vector<Class>();
            }
        }
        ret.push_back(i);
    }
    err_msg = "";
    return ret;
}

bool Tools::is_true(std::string str) {
    return str == "true" ||
           str == "True" ||
           str == "TRUE" ||
           str == "1";
}

bool Tools::is_false(std::string str) {
    return str == "false" ||
           str == "False" ||
           str == "FALSE" ||
           str == "0";
}