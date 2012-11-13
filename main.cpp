#include "parser.h"
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>


#undef Log
#undef Warn
#undef Error

//#define DEBUG_EXECUTOR
#ifdef DEBUG_EXECUTOR
#  define Log(str) std::cerr << str << "\n";
#else
#  define Log(str)
#endif

//#define DEBUG_FORK
#ifdef DEBUG_FORK
#  define ForkLog(str) std::cerr << getpid() << " " << str << "\n";
#else
#  define ForkLog(str)
#endif

struct CTaskRunner {
    CTaskRunner(CStmtPipeline pipeline, CVarTable& table) {
        Log("execute " << pipeline);

        std::vector<int> childs;
        int pipefd[2];
        int &pipeWr = pipefd[1];
        int &pipeRd = pipefd[0];
        int lastPipeRd = -1;

        for (int i = 0; i < pipeline.m_stmts.size(); ++i) {
            bool firstTime = i == 0;
            bool lastTime  = i == pipeline.m_stmts.size() - 1;
            if (!lastTime) {
                if (-1 == pipe(pipefd)) {
                    throw std::string("error with pipe");
                }
                ForkLog("pipe " << pipeWr << " -> " << pipeRd);
            }
            
            char **env =  table.BuildExportTable();
            char **argv = pipeline.m_stmts[i]->BuildArgv();
            
            int pid = fork();
            if (-1 == pid) {
                throw std::string("error with fork");
            }
            if (0 == pid) {
                if (!lastTime) {
                    ForkLog("close " << pipeRd);
                    close(pipeRd);

                    ForkLog("stout = " << pipeWr);
                    if (-1 == dup2(pipeWr, STDOUT_FILENO)) {
                        throw std::string("can't dup to stdout");
                    }

                    ForkLog("close " << pipeWr);
                    close(pipeWr);
                }
                if (!firstTime) {
                    ForkLog("stdin = " << lastPipeRd);
                    if (-1 == dup2(lastPipeRd, STDIN_FILENO)) {
                        throw std::string("can't dup to stdin");
                    }
                    ForkLog("close " << lastPipeRd);
                    close(lastPipeRd);
                }
                ForkLog("exec " << pipeline.m_stmts[i]->m_params[0]);
                execvpe(pipeline.m_stmts[i]->m_params[0].c_str(), argv, env);
                std::cerr <<  "can not execute: " + pipeline.m_stmts[0]->m_params[0] << "\n";
                exit(EXIT_FAILURE);
            }
            childs.push_back(pid);
            ForkLog("forked: " << pid);
            
            if (!firstTime) {
                ForkLog("close " << lastPipeRd);
                close(lastPipeRd);
            }
            lastPipeRd = pipeRd;
            if (!lastTime) {
                ForkLog("close " << pipeWr);
                close(pipeWr);
            }

            for (int i = 0; NULL != env[i]; i++) delete env[i];
            delete env;
            for (int i = 0; NULL != argv[i]; i++) delete argv[i];
            delete argv;
        }
        if (pipeline.m_wait) {
            for (int i = 0; i < childs.size(); ++i) {
                waitpid(childs[i], NULL, 0);
            }
        }
    }
};

class CExecutor {
public:
    CExecutor(): m_batchMode(false) { 
        ReadEnv();
        Run(std::cin);
    }
    CExecutor(std::string filename): m_batchMode(true) {
        std::ifstream fin;
        fin.open(filename.c_str());
        ReadEnv();
        Run(fin);
        fin.close();
    }
protected:
    void ReadEnv() {
	for (int i = 0; NULL != environ[i]; i++) {
            m_table.Set(environ[i], true);
	}        
    }
    void Run(std::istream& in) {
        while (in.good()) {
            if (!m_batchMode) std::cerr << "$ ";

            std::string str;
            getline(in, str);
            std::stringstream ss;
            ss << str;

            CLexer lex(ss, m_table);
            CParser parser;
            try {
                CPipesBatch* batch = parser.ParseLine(lex);
                for (std::vector<CStmtPipeline*>::iterator it = batch->m_pipes.begin(); it != batch->m_pipes.end(); ++it) {
                    try {
                        if ((*it)->m_special) {
                            EvaluateSpecial(**it);
                        } else {
                            EvaluatePipeline(**it);
                        }
                    }
                    catch (std::string str) {
                        std::cerr << str << "\n";
                    }
                }
                delete batch;
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
    bool m_batchMode;
};

int main(int argc, char* argv[], char* env[])
{
    if (argc == 1) {
        CExecutor e;
    } else {
        for (int i = 1; i < argc; ++i) {
            CExecutor e(argv[i]);
        }
    }
    return 0;
}

