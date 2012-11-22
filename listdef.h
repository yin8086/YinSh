#ifndef LISTDEF_H
#define LISTDEF_H
#include<iostream>
#include<vector>
#include<string>
#include"classheader.h"
class CmdList
{
    vector<PipeLine*> vcmdl;
    bool isnew;
    vector<ReDir*> rcgc;
public:
    CmdList():isnew(true)
    {
        vcmdl.reserve(VEC_RESER);
        rcgc.reserve(VEC_RESER);
    }
    PipeLine* currentPL()//访问当前构建的管道线
    {
       if(isnew) 
       {
           vcmdl.push_back(new PipeLine);
           isnew=false;
       }
       return vcmdl.back();
    }
    void SetNew(bool ops=true)
    {
        isnew=ops;
    }
    void AddRubish(ReDir* ops)
    {
        if(ops)
            rcgc.push_back(ops);
    }
    int Execute()
    {
        for(unsigned int i=0;i<vcmdl.size();i++)
        {
            if(!vcmdl[i]->IsError())
            {
                if(vcmdl[i]->Execute()==YIN_QUIT) return YIN_QUIT;
                //cout<<endl;
                //if((i+1)<vcmdl.size())
                 //   cout<<"============================"<<endl;
            }
            else
            {
                cout<<"Syntax Error"<<endl;//<<"============================"<<endl;
            }
        }
        return 0;
    }
    ~CmdList()
    {
        for(unsigned int i=0;i<vcmdl.size();i++)
        {
            if(vcmdl[i]!=NULL)
            {
                delete vcmdl[i];
                vcmdl[i]=NULL;
            }
        }
        for(unsigned int i=0;i<rcgc.size();i++)
        {
            if(rcgc[i]!=NULL)
            {
                delete rcgc[i];
                rcgc[i]=NULL;
            }
        }
    }
};
#endif
