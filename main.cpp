#include "parser.h"
#include <unistd.h>
#include <wait.h>

#undef Log
#undef Warn
#undef Error

//#define DEBUG_EXECUTOR
#ifdef DEBUG_EXECUTOR
#  define Log(str) std::cerr << str << "\n";
#  define Warn(str) std::cerr << "[warn] " << str << "\n";
#  define Error(str) std::cerr << "[err] " << str << "\n";
#else
#  define Log(str)
#  define Warn(str)
#  define Error(str)
#endif


struct CTaskRunner {
    CTaskRunner(CStmtPipeline pipeline, CVarTable& table) {
        Log("execute " << pipeline);
        char **env =  table.BuildExportTable();
        char **argv;
        {
            int len = pipeline.m_stmts[0]->m_params.size() + 1;
            argv = new char*[len];
            for (int i = 0; i < len - 1; ++i) {
                argv[i] = new char[pipeline.m_stmts[0]->m_params[i].size() + 1];
                strcpy(argv[i], pipeline.m_stmts[0]->m_params[i].c_str());
            }
            argv[len - 1] = NULL;
        }

        int pid = fork();
        if (pid == -1) {
            throw std::string("error with fork");
        } if (0 == pid) {
            execve(pipeline.m_stmts[0]->m_params[0].c_str(), argv, env);
        } else {
            for (int i = 0; NULL != env[i]; i++) delete env[i];
            delete env;
            for (int i = 0; NULL != argv[i]; i++) delete argv[i];
            delete argv;

            if (pipeline.m_wait) waitpid(pid, NULL, 0);
        }
    }
};

class CExecutor {
public:
    CExecutor() { 
        ReadEnv();
        Run(std::cin);
    }
    CExecutor(std::string filename) {
        std::ifstream fin;
        fin.open(filename.c_str());
        ReadEnv();
        Run(fin);
        fin.close();
    }
protected:
    void ReadEnv() {
	for (int i = 0; NULL != environ[i]; i++) {
//            Log(environ[i]);
            m_table.Set(environ[i], true);
	}        
    }

    void Run(std::istream& in) {
        while (in.good()) {
            std::string str;
            getline(in, str);
            std::stringstream ss;
            ss << str;

            CLexer lex(ss, m_table);
            CParser parser;
            try {
                CStmtPipeline res = parser.ParseLine(lex);
                if (res.m_special) {
                    EvaluateSpecial(res);
                } else {
                    EvaluatePipeline(res);
                }
            }
            catch (std::string str) {
                std::cerr << str << "\n";
            }
        }
    }

    void EvaluateSpecial(CStmtPipeline& pipeline) {
        CStmtExecute* stmt = pipeline.m_stmts[0];
        std::string cmd = stmt->m_params[0];
        if ("export" == cmd) {
            if (stmt->m_params.size() == 1) {
                m_table.DumpExport(std::cout);
            } else {
                m_table.Export(stmt->m_params[1]);
            }
        } else if ("set" == cmd) {
            m_table.Set(stmt->m_params[1], stmt->m_params[2], false);
        } else if ("dump" == cmd) {
            m_table.Dump(std::cout);
        }
    }
    void EvaluatePipeline(CStmtPipeline& res) {
        if (res.m_stmts.size() > 0) {
            CTaskRunner r(res, m_table);
        }
    }
    
    CVarTable m_table;
};

int main(int argc, char* argv[], char* env[])
{
    if (1) {
        CExecutor e;
        return 0;
    }
    
    {
        std::ifstream fin;
        fin.open("input.txt");
        std::cerr << "=== Tokens ===\n";
        CVarTable table;
        table.Set("var", "value");
        CLexer lex(fin, table);
        IToken* tok = NULL;
        do {
            bool b;
            tok = lex.GetToken(b);
            if (tok != NULL ) std::cerr << *tok << "\n";
            else std::cerr << "[NULL]";
        } while (tok != NULL);
        fin.close();
        return 0;
    }
    {
        std::ifstream fin;
        fin.open("input.txt");
        std::cerr << "\n=== Statements ===\n";
        CVarTable table;
        CLexer lex(fin, table);
        CParser parser;
        try {
            CStmtPipeline res = parser.ParseLine(lex);
            std::cerr << res;
        }
        catch (std::string str) {
            std::cerr << str << "\n";
        }
        fin.close();
    }    
    return 0;
}

