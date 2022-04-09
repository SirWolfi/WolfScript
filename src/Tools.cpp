#include "../inc/Tools.hpp"

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
        return "";
    }
    if (dummy == nullptr || strlen(dummy) == 0) {
        ifile.close();
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