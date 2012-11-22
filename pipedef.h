#ifndef PIPEDEF_H
#define PIPEDEF_H
#include<iostream>
#include<string>
#include<vector>
#include<cstdlib>
#include"classheader.h"
using namespace std;
class PipeLine
{
    bool isback;
    bool iserror;
    vector<BaseCmd*> vpipel;
public:
    PipeLine():isback(false),iserror(false)
    {
        vpipel.reserve(VEC_RESER);
    }
    void SetBack()
    {
        isback=true;
    }
    BaseCmd* currentBC()//访问当前构建的指令
    {
        if(!vpipel.empty())
            return vpipel.back();
        else 
        {
            cout<<"Something Wrong"<<endl;
            _exit(3);
        }
    }
    void AddBC(const string* ops) //当前指令构构建成功，再加入一个
    {
        //分析传入命令符号，建立不同的BashCmd
        //进行运行时绑定执行
        if(ops)
        {
            BaseCmd* tmp=NULL;
            if(ops->compare("exit")==0)
                tmp=new ExitCmd;
            else if(ops->compare("cd")==0)
                tmp=new CdCmd;
            else if(ops->compare("job")==0)
                tmp=new JobCmd;
            else if(ops->compare("kill")==0)
                tmp=new KillCmd;
            else if(ops->compare("echo")==0)
                tmp=new EchoCmd;
            else if(ops->compare("exec")==0)
                tmp=new ExecCmd;
            else
                tmp=new ExternCmd; 
            tmp->SetName(ops);
            vpipel.push_back(tmp);
        }
    }
    int Execute()
    {
        // 后台指令屏蔽标准输入
        if(1==vpipel.size())
        {
            //单个指令和管道指令分开
            if(!vpipel[0]->IsExtern())
            {
                //单指令时候的内部指令才总是进行正确的操作
                // 支持重定向但是忽略后台标志
                int oldin=dup(STDIN_FILENO);
                int oldout=dup(STDOUT_FILENO);
                if(!vpipel[0]->RedirectIn())
                    cerr<<"yinsh: Redirect In error"<<endl;
                if(!vpipel[0]->RedirectOut())
                    cerr<<"yinsh: Redirect Out error"<<endl;
                // exit只有在单指令时才启作用
                if(vpipel[0]->Execute()==YIN_QUIT) return YIN_QUIT;
                dup2(oldin,STDIN_FILENO);//及时对标准输入恢复
                dup2(oldout,STDOUT_FILENO);//及时对标准输出恢复
                //cout<<endl;
            }
            else
            {
                int child_pid;
                // 外部指令都会fork出新的进程
                if((child_pid=fork())==0)
                {
                    if(!isback) execm.UnBlockIntQuit();
                    if(!vpipel[0]->RedirectIn(isback))
                        cerr<<"yinsh: Redirect In error"<<endl;
                    if(!vpipel[0]->RedirectOut())
                        cerr<<"yinsh: Redirect Out error"<<endl;
                    exit(vpipel[0]->Execute());
                }
                else if(child_pid>0)
                {
                    if(isback)
                    {
                        execm.AddBGTask(child_pid);
                        cerr<<child_pid<<endl;
                        return 0;
                    }
                    else
                    {
                        int status;
                        waitpid(child_pid,&status,0);
                        if(WIFEXITED(status))
                            return WEXITSTATUS(status);
                        else
                            return WTERMSIG(status);
                    }
                }
                else
                {
                    cerr<<"yinsh: fork error"<<endl;
                    return -1;
                }
            }

        }
        else if(vpipel.size()>1)
        {
            // 按照正常语义处理管道中的内部指令
            // 只要存在管道，则管道线所有指令将会创建相应子进程
            // 因此其中部分内部指令可能不发挥作用
            int pm_pid;// 管道管理进程pid,也是管道最后个指令的进程
            if((pm_pid=fork())<0)
            {
                cerr<<"yinsh: fork error"<<endl;
                return -1;
            }
            else if(pm_pid==0)
            {
                if(!isback) execm.UnBlockIntQuit();
                int fds[2];
                int chpipe;
                unsigned int i;
                for(i=0;i<vpipel.size()-1;i++)
                {
                    //针对前n-1条指令指令进行处理
                    pipe(fds);//每次都会打开新的文件表对原来的无影响
                    if((chpipe=fork())==0)
                    {
                        if(0==i)
                            if(!vpipel[i]->RedirectIn(isback))
                                cerr<<"yinsh: Redirect In Error"<<endl;
                        //输出重定向
                        dup2(fds[1],STDOUT_FILENO);
                        close(fds[1]);
                        close(fds[0]);
                        exit(vpipel[i]->Execute());
                    }
                    else if(chpipe>0)
                    {
                        //为下一个子进程做准备
                        //为了实现一定并行性，不进行wait
                        dup2(fds[0],STDIN_FILENO);
                        close(fds[0]);
                        close(fds[1]);
                    }
                }
                // 实际上同时pm_pid也是管道最后一个进程的实体
                if(!vpipel[i]->RedirectOut())
                    cerr<<"yinsh: Redirect In Error"<<endl;
                exit(vpipel[i]->Execute());
            }
            else
            {
                //这是真正的父进程，管道管理进程是其子进程
                if(isback)
                {
                    // 相当于将管道最后个进程加入后台列表
                    execm.AddBGTask(pm_pid);
                    cerr<<pm_pid<<endl;
                    return 0;
                }
                else
                {
                    int status;
                    // 等待最后一个进程结束
                    waitpid(pm_pid,&status,0);
                    if(WIFEXITED(status))
                        return WEXITSTATUS(status);
                    else
                        return WTERMSIG(status);
                }
            }
        }
        return 0;
    }
    bool IsError() const
    {
        return iserror;
    }
    void SetError()
    {
        iserror=true;
    }
    ~PipeLine()
    {
        for(unsigned int i=0;i<vpipel.size();i++)
        {
            if(vpipel[i]!=NULL)
            {
                delete vpipel[i];
                vpipel[i]=NULL;
            }
        }
    }
};
#endif
