#ifndef _LEXER_H
#define _LEXER_H

#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <assert.h>

//#define DEBUG_LEXER
#ifdef DEBUG_LEXER
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
    CVar(const std::string name, const std::string value, bool exported = false): m_name(name), m_value(value), m_exported(exported) {}
    std::string m_name;
    std::string m_value;
    bool m_exported;
};

bool operator<<(const CVar& a, const CVar& b) { return a.m_name < b.m_name; }

class CVarTable {
public:
    std::string GetValue(std::string variableName) const { return ""; /* TODO */ }
    char* BuildExportTable() const;
private:
    std::set<CVar> m_data;
};

enum EToken { EString, EPipe, ENoWait };
std::string tokToStr[] = { "EString", "EPipe", "ENoWait" };

struct IToken { 
    virtual EToken GetTokenType() = 0;
    virtual void Dump(std::ostream& out) { out << tokToStr[GetTokenType()]; }
};

std::ostream& operator<<(std::ostream& out, IToken& tok) {
    tok.Dump(out);
    return out;
}

struct CTokString: public IToken {
    CTokString(std::string data): m_data(data) {}
    virtual EToken GetTokenType() { return EString; }
    virtual void Dump(std::ostream& out) {
        out << tokToStr[GetTokenType()] << "(" << m_data << ")";
    }
    std::string m_data;
};

struct CTokPipe: public IToken {
    virtual EToken GetTokenType() { return EPipe; }
};

struct CTokNoWait: public IToken {
    virtual EToken GetTokenType() { return ENoWait; }
};

class CLexer {
public:
    CLexer(std::istream& in, CVarTable& varTable): m_in(in), m_varTable(varTable), m_state(EWasSpace), m_res(NULL) {}
    IToken* GetToken(bool& expanded) {
        if (EEndOfInput == m_state) return NULL;
        m_res = NULL;
        char c = ' ';
        while (c != '\n' && !m_in.eof() && m_res == NULL) {
            c = m_in.get();
            Log("got " << c);

            if (m_in.eof()) continue;

            if (m_state == EWasPara) {
                assert(0);
            } else {
                if (m_state == EWasBackSlash) {
                    m_str += c;
                    m_state = EWasLetter;
                } else if (c == '\\') {
                    FinishVar();
                    m_state = EWasBackSlash;
                } else if (c == '&') {
                    if (FinishStr()) {
                        m_in.putback(c);
                    } else {
                        m_res = new CTokNoWait;
                    }
                } else if (c == '|') {
                    if (FinishStr()) {
                        m_in.putback(c);
                    } else {
                        m_res = new CTokPipe;
                    }
                } else if (isspace(c)) {
                    if (FinishStr()) {
                    } else {
                        m_state = EWasSpace;
                    }
                } else if (c == '"') {
                    FinishVar();
                    "" == m_str ? EWasSpace : EWasLetter;
                } else if (c == '$') {
                    m_state = EWasVar;
                } else {
                    m_str += c;
                }
            }
        }
        if (NULL == m_res) FinishStr();
        if (NULL == m_res) {
            Log("return NULL");
        } else {
            Log("return " << *m_res);
        }
        if (m_in.eof()) {
            m_state = EEndOfInput;
        }
        return m_res;
    }
protected:
    enum EState {
        EWasSpace,
        EWasBackSlash,
        EWasVar,
        EWasLetter,
        EWasPara,
        EEndOfInput
    };
    void FinishVar() {
        if (m_var != "") {
            m_str += m_varTable.GetValue(m_var);
            m_var = "";
        }
    }
    bool FinishStr() {
        if (EWasVar == m_state) {
            FinishVar();
        }
        if ("" == m_str) return false;

        assert(NULL == m_res);
        m_res = new CTokString(m_str);
        Log("m_res Assigned");
        m_str = "";
        return false;
    }

    IToken* m_res;
    std::string m_str;
    std::string m_var;
    CVarTable& m_varTable;    
    std::istream& m_in;
    EState m_state;
};

#endif
