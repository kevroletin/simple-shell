#ifndef _VAR_TABLE_H
#define _VAR_TABLE_H

#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <assert.h>
#include <cstring>

#undef Log
#undef Warn
#undef Error

#define DEBUG_VARTABLE
#ifdef DEBUG_VARTABLE
#  define Log(str) std::cerr << str << "\n";
#  define Warn(str) std::cerr << "[warn] " << str << "\n";
#  define Error(str) std::cerr << "[err] " << str << "\n";
#else
#  define Log(str)
#  define Warn(str)
#  define Error(str)
#endif

struct CVar {
    CVar() {}
    CVar(const std::string value, bool exported = false): m_value(value), m_exported(exported) {}
    std::string m_value;
    bool m_exported;
};

class CVarTable {
public:
    std::string GetValue(std::string variableName) { 
        return m_data[variableName].m_value;
    }
    char** BuildExportTable() {
        int len = 0;
        for (std::map<std::string, CVar>::iterator it = m_data.begin(); it != m_data.end(); ++it) {
            if (it->second.m_exported) ++len;
        }        
        char** res = new char*[len];
        int i = 0;
        for (std::map<std::string, CVar>::iterator it = m_data.begin(); it != m_data.end(); ++it) {
            if (it->second.m_exported) {
                std::string str = it->first + "=" + it->second.m_value;
                res[i] = new char[str.size() + 1];
                strcpy(res[i], str.c_str());
                ++i;
            }
        }
        res[len - 1] = NULL;
        return res;
    }
    void DumpExport(std::ostream& out) {
        for (std::map<std::string, CVar>::iterator it = m_data.begin(); it != m_data.end(); ++it) {
            if (it->second.m_exported) {
                out << "*" << it->first << " " << it->second.m_value << "\n";
            }
        }
    }
    void Dump(std::ostream& out) {
        for (std::map<std::string, CVar>::iterator it = m_data.begin(); it != m_data.end(); ++it) {
            out << (it->second.m_exported ? "*" : " ") << it->first << " " << it->second.m_value << "\n";
        }
    }    
    bool Export(std::string name) {
        std::map<std::string, CVar>::iterator it = m_data.find(name);
        if (it == m_data.end()) return false;
        it->second.m_exported = true;
        return true;
    }
    void Set(std::string name, std::string value, bool exported) {
        m_data[name] = CVar(value, exported);
    }
    bool Set(std::string data, bool exported) {
        size_t pos = data.find('=');
        if (pos == -1) {
            //Set(data, "", exported);
            return false;
        } else {
            Set(data.substr(0, pos), data.substr(pos + 1, data.length() - (pos + 1)), exported);
            return true;
        }
    }
private:
    std::map<std::string, CVar> m_data;
};

#endif
