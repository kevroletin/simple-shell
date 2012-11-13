#ifndef _PARSER_H
#define _PARSER_H

#include "lexer.h"
#include <iostream>
#include <fstream>
#include <vector>

#undef Log
#undef Warn
#undef Error

//#define DEBUG_PARSER
#ifdef DEBUG_PARSER
#  define Log(str) std::cerr << str << "\n";
#  define Warn(str) std::cerr << "[warn] " << str << "\n";
#  define Error(str) std::cerr << "[err] " << str << "\n";
#else
#  define Log(str)
#  define Warn(str)
#  define Error(str)
#endif


typedef unsigned uint;

enum EStatement { EPipesBatch, EStmtPipeline, EStmtExecute };
std::string statementToStr[] = { "EPipesBatch", "EStmtPipeline", "EStmtExecute" };

void PrintSpaces(std::ostream& out, uint offset) {
    while (offset--) out << ' ';
}

struct IStatement {
    virtual EStatement GetStatementType() = 0;
    virtual void Dump(std::ostream& out, uint offset) {
        PrintSpaces(out, offset);
        out << statementToStr[GetStatementType()] << "\n";
    };
};

std::ostream& operator<<(std::ostream& out, IStatement& st) {
    st.Dump(out, 0);
    return out;
}

struct CStmtExecute: public IStatement {
    std::vector<std::string> m_params;
    virtual EStatement GetStatementType() { return EStmtExecute; }
    virtual void Dump(std::ostream& out, uint offset) {
        PrintSpaces(out, offset);
        out << statementToStr[GetStatementType()] << "(";
        for (std::vector<std::string>::iterator it = m_params.begin(); it != m_params.end(); ++it) {
            out << *it << " ";
        }
        out << ")\n";
    };
    char** BuildArgv() {
        int len = m_params.size() + 1;
        char** argv = new char*[len];
        for (int i = 0; i < len - 1; ++i) {
            argv[i] = new char[m_params[i].size() + 1];
            strcpy(argv[i], m_params[i].c_str());
        }
        argv[len - 1] = NULL;
        return argv;
    }
};

struct CStmtPipeline: public IStatement {
    CStmtPipeline(): m_wait(true), m_special(false) {}
    std::vector<CStmtExecute*> m_stmts;
    bool m_wait;
    bool m_special;
    virtual EStatement GetStatementType() { return EStmtPipeline; }
    virtual void Dump(std::ostream& out, uint offset) {
        PrintSpaces(out, offset);
        out << statementToStr[GetStatementType()] << "(" << (m_wait? " ": "&") << ")" << (m_special? "*": "") << "\n";
        for (std::vector<CStmtExecute*>::iterator it = m_stmts.begin(); it != m_stmts.end(); ++it) {
            (*it)->Dump(out, offset + 2);
        }
    };
};

struct CPipesBatch: public IStatement {
    std::vector<CStmtPipeline*> m_pipes;
    virtual EStatement GetStatementType() { return EStmtPipeline; }
    virtual void Dump(std::ostream& out, uint offset) {
        PrintSpaces(out, offset);
        out << statementToStr[GetStatementType()] << "\n";
        for (std::vector<CStmtPipeline*>::iterator it = m_pipes.begin(); it != m_pipes.end(); ++it) {
            (*it)->Dump(out, offset + 2);
        }
    };
};

class CParser {
public:
    CPipesBatch* ParseLine(CLexer& lex) {
    bool b;
        CPipesBatch* batch = new CPipesBatch;
        CStmtPipeline* pipe = NULL;
        CStmtExecute* exec = NULL;
        IToken* tok;
        m_state = EClear;
        while (NULL != (tok = lex.GetToken(b))) {
            Log("got " << *tok);
            EToken ttype = tok->GetTokenType();
            switch (m_state) {
            case EWasSpecialCmd: {
                throw(std::string("unexpected input after special command"));
            } break;
            case EClear:
                Log("Create pipe " << pipe);
                pipe = new CStmtPipeline;
            case EWasPipe: {
                CheckTok(tok, EString);
                Log("Create from " << *tok);
                exec = new CStmtExecute;
                std::string data = static_cast<CTokString*>(tok)->m_data;
                exec->m_params.push_back( data );
                Log(data);
                if ("export" == data) {
                    if (NULL != (tok = lex.GetToken(b))) {
                        CheckTok(tok, EString);
                        exec->m_params.push_back( static_cast<CTokString*>(tok)->m_data );
                        m_state = EWasSpecialCmd;
                    }
                    pipe->m_special = true;
                } else if ("set" == data) {
                    if (NULL != (tok = lex.GetToken(b))) {
                        CheckTok(tok, EString);
                        exec->m_params.push_back( static_cast<CTokString*>(tok)->m_data );
                    } else {
                        throw std::string("expected variable name");
                    }
                    if (NULL != (tok = lex.GetToken(b))) {
                        CheckTok(tok, EString);
                        exec->m_params.push_back( static_cast<CTokString*>(tok)->m_data );
                    } else {
                        exec->m_params.push_back("");
                    }
                    m_state = EWasSpecialCmd;
                    pipe->m_special = true;
                } else if ("dump" == data) {
                    m_state = EWasSpecialCmd;
                    pipe->m_special = true;
                } else {
                    m_state = EWasStr;
                }
            } break;
            case EWasNoWait: {
                throw std::string("unexpected input after &");
            } break;
            case EWasStr: {
                if (EString == ttype) {
                    exec->m_params.push_back( static_cast<CTokString*>(tok)->m_data );
                } else if (EPipe == ttype) {
                    Log("Push " << *exec);
                    pipe->m_stmts.push_back(exec);
                    exec = NULL;
                    m_state = EWasPipe;
                } else if (ENoWait == ttype) {
                    Log("Push " << *exec);
                    pipe->m_stmts.push_back(exec);
                    exec = NULL;
                    m_state = EClear;
                    pipe->m_wait = false;
                    batch->m_pipes.push_back(pipe);
                    pipe = NULL;
                }
            } break;
            default: assert(0);
            }
        }
        if (NULL != exec) {
            if (0 == exec->m_params.size()) {
                delete exec;
            } else if (pipe != NULL) {
                pipe->m_stmts.push_back(exec);
                exec = NULL;
            }
        }
        if (NULL != pipe ) {
            batch->m_pipes.push_back(pipe);
            pipe = NULL;
        }
        Log("Result=" << batch);
        return batch;
    }
    void CheckTok(IToken* tok, EToken requestedType) {
        if (tok->GetTokenType() != requestedType) {
            std::stringstream ss;
            ss << "error: extected " << tokToStr[requestedType] << " but got " << *tok;
            throw ss.str();
        }
    }
protected:
    enum EState { EClear, EWasPipe, EWasNoWait, EWasStr, EWasSpecialCmd };
    EState m_state;
};

#endif
