#ifndef MANADEF_H
#define MANADEF_H
#include<list>
#include<cstdio>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include"classheader.h"
using namespace std;
class ExecManager
{
    CmdList* pclist;
    list<pid_t> vbgtask;
public:
    ExecManager():pclist(NULL)
    {
    }
    bool IsReady() const
    {
        return pclist!=NULL?true:false;
    }
    void Attach(CmdList* ops)
    {
        pclist=ops;
    }
    void Detach()
    {
        pclist=NULL;
    }
    void AddBGTask(pid_t pid)
    {
        vbgtask.push_back(pid);
    }
    void CheckBGTask(bool show=false)
    {
        pid_t back_pid;
        int count=0;
        for(list<pid_t>::iterator ilp=vbgtask.begin();
                ilp!=vbgtask.end();)
        {
            int status;
            back_pid=waitpid(*ilp,&status,WNOHANG);
            count++;
            if(0==back_pid)
            {
                if(show) cout<<"["<<count<<"] Running "<<*ilp<<endl;
                ilp++;
            }
            else
            {
                switch(status)
                {
                    case SIGTERM:
                        cout<<"["<<count<<"] Terminated "<<*ilp<<endl;
                        break;
                    case 0:
                        cout<<"["<<count<<"] Done "<<*ilp<<endl;
                        break;
                    case SIGKILL:
                        cout<<"["<<count<<"] Killed "<<*ilp<<endl;
                        break;
                    default:
                        cout<<"["<<count<<"] Stopped("<<status<<") "
                            <<*ilp<<endl;
                        break;
                }
                ilp=vbgtask.erase(ilp);
            }

        }
    }
    int BlockIntQuit()
    {
        if(signal(SIGINT,SIG_IGN)==SIG_ERR)
            return -1;
        if(signal(SIGQUIT,SIG_IGN)==SIG_ERR)
            return -1;
        return 0;
    }
    int UnBlockIntQuit()
    {
        if(signal(SIGINT,SIG_DFL)==SIG_ERR)
            return -1;
        if(signal(SIGQUIT,SIG_DFL)==SIG_ERR)
            return -1;
        return 0;
    }
    bool ClearBGTask()
    {
        char sure='y';
        if(!vbgtask.empty())
        {
            cerr<<"Following jobs still running"<<endl;
            CheckBGTask(true);
            do
            {
                cerr<<"Exit will kill them.Are you sure?(y or n)";
                sure=getc(stdin);
            }while(sure!='y' && sure!='n');
            if(sure=='y')
            {
                for(list<pid_t>::iterator ilp=vbgtask.begin();
                        ilp!=vbgtask.end();)
                    kill(*ilp,SIGTERM);
                return true;
            }
            else return false;
        }
        else return true;
    }
    CmdList* clist()
    {
        return pclist;
    }
    ~ExecManager()
    {
    }
};
#endif
