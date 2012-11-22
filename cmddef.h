#ifndef CMDDEF_H
#define CMDDEF_H
#include<iostream>
#include<string>
#include<vector>
#include<cstdlib>
#include<cstring>
#include<fcntl.h>
#include"classheader.h"
#include"managerdef.h"
using namespace std;
extern ExecManager execm;

class BaseCmd
{
protected:
   string cname;
   vector<string> vargs;
   string infd;
   string outfd;
   bool isappend;
   bool isextern;
public:
   BaseCmd():isappend(false)
    {
        vargs.reserve(VEC_RESER);
    }
   void SetName(const string *ops)
   {
       if(ops&&!ops->empty())
           cname.assign(*ops);
   }
   void AddArgs(const string *ops)
   {
       if(ops&&!ops->empty())
       {
          //tmparg.assign(*ops);
          vargs.push_back(*ops);
       }
   }
   void SetReDir(const ReDir* ops)
   {
       if(ops&&!ops->fname.empty())
       {
            switch(ops->type)
            {
                case ReDir::IN:
                    infd.assign(ops->fname);
                    break;
                case ReDir::OUT:
                    outfd.assign(ops->fname);
                    isappend=false;
                    break;
                case ReDir::APPEND:
                    outfd.assign(ops->fname);
                    isappend=true;
                    break;
            }
       }
   }
   char** transcArgs()
   {
       if(cname.empty())
           return NULL;
       else
       {
           char **tmp=new char*[vargs.size()+2];
           tmp[0]=new char[cname.length()+1];
           strcpy(tmp[0],cname.c_str());
           unsigned int i;
           for(i=0;i<vargs.size();i++)
           {
               tmp[i+1]=new char[vargs[i].length()+1];
               strcpy(tmp[i+1],vargs[i].c_str());
           }
           tmp[i+1]=NULL;
           return tmp;
       }
   }
   void freecArgs(char** arglist)
   {
       if(arglist==NULL)
           return;
       else
       {
           while(arglist)
           {
               delete[] (*arglist);
               arglist++;
           }
           delete[] arglist;
       }
   }
   bool RedirectOut()
   {
       if(!outfd.empty())
       {
           int rdfdo;
           if(!isappend)
               rdfdo=open(outfd.c_str(),O_WRONLY|O_CREAT|O_TRUNC);
           else
               rdfdo=open(outfd.c_str(),O_WRONLY|O_CREAT|O_APPEND);
           if(rdfdo==-1)
           {
               cerr<<"yinsh: "<<outfd<<": No such file or directory"<<endl;
               return false;
           }
           else
           {
               if(dup2(rdfdo,STDOUT_FILENO)==-1)
               {
                   close(rdfdo);
                   return false;
               }
               close(rdfdo);
           }
       }
       return true;
   }
   bool RedirectIn(bool back=false)
   {
       if(!infd.empty())
       {
          int rdfdi;
          if((rdfdi=open(infd.c_str(),O_RDONLY))==-1) 
          {
              cerr<<"yinsh: "<<infd<<": No such file or directory"<<endl;
              return false;
          }
          else
          {
              if(dup2(rdfdi,STDIN_FILENO)==-1)
              {
                  close(rdfdi);
                  return false;
              }
              close(rdfdi);
          }
       }
       else if(back)
       {
           // 无重定向输入时候，而且后台，则屏蔽标准输入
           int rdfdn=open("/dev/null",O_RDONLY);
           dup2(rdfdn,STDIN_FILENO);
           close(rdfdn);
       }
       return true;
   }
   bool IsExtern() const
   {
       return isextern;
   }
   virtual int Execute()=0;
   virtual ~BaseCmd()
   {
   }
};
            
class ExitCmd:public BaseCmd
{
public:
    ExitCmd():BaseCmd()
    {
        isextern=false;
    }
    virtual int Execute()
    {
        return YIN_QUIT;
    }
    virtual ~ExitCmd(){}

};

class CdCmd:public BaseCmd
{
public:
    CdCmd():BaseCmd()
    {
        isextern=false;
    }
    virtual int Execute()
    {
        if(vargs.empty())
        {
            cerr<<"cd: usage: cd [directory]"<<endl;
            return -1;
        }
        else if(chdir(vargs[0].c_str())==-1)
            cerr<<"yinsh: cd: No such file or directory"<<endl;
        return 0;
    }
    virtual ~CdCmd(){}
};

class JobCmd:public BaseCmd
{
public:
    JobCmd():BaseCmd()
    {
        isextern=false;
    }
    virtual int Execute()
    {
        execm.CheckBGTask(true);
        return 0;
    }
    virtual ~JobCmd(){}
};

class KillCmd:public BaseCmd
{
public:
    KillCmd():BaseCmd()
    {
        isextern=false;
    }
    virtual int Execute()
    {
        pid_t pid;
        if(vargs.empty())
        {
            cerr<<"yinsh: kill must have arguments"<<endl;
            return -1;
        }
        if((pid=abs(atoi(vargs[0].c_str())))==0)
        {
            cerr<<"yinsh: kill: "<<vargs[0]<<": arguments invalid"<<endl;
            return -1;
        }
        if(kill(pid,SIGTERM) == -1)
        {
            cerr<<"yinsh: kill: "<<pid<<"- No such process"<<endl;
            return -1;
        }
        return 0;
    }
    virtual ~KillCmd(){}
};

class EchoCmd:public BaseCmd
{
public:
    EchoCmd():BaseCmd()
    {
        isextern=false;
    }
    virtual int Execute()
    {
        if(vargs.empty())
            cout<<endl;
        else
            cout<<vargs[0]<<endl;
        return 0;
    }
    virtual ~EchoCmd(){}
};

class ExecCmd:public BaseCmd
{
public:
    ExecCmd():BaseCmd()
    {
        isextern=false;
    }
    virtual int Execute()
    {
        if(vargs.empty())
            return 0;
        else
        {
            char** largs=transcArgs();
            if(largs==NULL)
                return 0;
            else if(execvp(largs[1],largs+1)==-1)
            {
                cerr<<"yinsh: exec: "<<largs[1]<<" not found"<<endl;
                freecArgs(largs);
                return -1;
            }
            return 0;
        }

    }
    virtual ~ExecCmd(){}
};

class ExternCmd:public BaseCmd
{
public:
    ExternCmd():BaseCmd()
    {
        isextern=true;
    }
    virtual int Execute()
    {
        char** largs=transcArgs();
        if(largs==NULL)
            return 0;
        else if(execvp(largs[0],largs)==-1)
        {
            cerr<<"yinsh: "<<*largs<<" command not found"<<endl;
            freecArgs(largs);
        }
        return 0;
    }
    virtual ~ExternCmd(){}
};
#endif
