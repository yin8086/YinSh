%{
#include<iostream>
#include<string>
#include"classheader.h"
#include"cmddef.h"
#include"pipedef.h"
#include"listdef.h"
#include"managerdef.h"
// #include"clsdef.h"
using namespace std;
void yyerror(const char*);
int yylex();
ExecManager execm;
bool errfnd=false; //加入是否恢复完成标志
%}
%debug
%union {
 	//由于union只支持POD类型（即内存分布与成员排列相同)
	//任何带有CTOR的类均非POP，采用指针解决
	string* sWord;
        ReDir* redi;
}
%token <sWord> WORD
%token APPEND
%type <sWord> execute cmd_list cmd_list1
%type <sWord> cmd_unit real_cmd easy_cmd
%destructor { delete $$;} <sWord>
%type <redi> redir 
%left ';' '\n'
%left '&' '|'
%right '<' '>' APPEND 
%%
execute: cmd_list '\n' {
         $$=NULL;
         errfnd=false;
        YYACCEPT
       }
	;
cmd_list: cmd_list1 {
         $$=NULL;
        }
	| cmd_list1 ';' {
	//此条语法专用来解决以';'的指令序列
         $$=NULL;
        }
	;
cmd_list1: cmd_list1 ';' cmd_unit {
         $$=NULL;
         }
         | cmd_list1 ';' error {
        execm.clist()->currentPL()->SetError();
            // 设置错误位，如果连pipeline没有建，顺带建一个
        execm.clist()->SetNew();
        errfnd=true;
         $$=NULL;
        yyclearin;
        YYRECOVERING();
         }
	| cmd_unit {
         $$=NULL;
        }
        | error  {
        if(!errfnd)
        {
            //专门用来解决格式错误问题
                execm.clist()->currentPL()->SetError();
            // 设置错误位，如果连pipeline没有建，顺带建一个
                execm.clist()->SetNew();
                errfnd=true;
        }
        yyclearin;
        YYRECOVERING();
        $$=NULL;
        }
        ;
cmd_unit: real_cmd {
        execm.clist()->SetNew();
         $$=NULL;
        }
        | real_cmd '&' {
        execm.clist()->currentPL()->SetBack();
        execm.clist()->SetNew();
         $$=NULL;
        }
        ;
real_cmd: real_cmd '|' real_cmd {
         $$=NULL;
        }
	| real_cmd redir {
        execm.clist()->currentPL()->currentBC()->SetReDir($2);
         $$=NULL;
	}
        | easy_cmd  {
         $$=NULL;
        }
	;
easy_cmd: WORD {
        execm.clist()->currentPL()->AddBC($1);
         delete $1;
         $$=NULL;
        }
        | easy_cmd WORD {
        execm.clist()->currentPL()->currentBC()->AddArgs($2);
         delete $2;
         $$=NULL;
        }
        ;
redir: '<' WORD {
     execm.clist()->AddRubish($$=new ReDir);
     $$->type=ReDir::IN;
     $$->fname.assign(*$2);
         delete $2;
     }
     | '>' WORD {
     execm.clist()->AddRubish($$=new ReDir);
     $$->type=ReDir::OUT;
     $$->fname.assign(*$2);
         delete $2;
      }
      | APPEND WORD {
     execm.clist()->AddRubish($$=new ReDir);
     $$->type=ReDir::APPEND;
     $$->fname.assign(*$2);
         delete $2;
      }
      ;
%%
int main()
{
    //yydebug=12;
    bool checktag=false;
    execm.BlockIntQuit();
    while(true)
    {
        execm.Detach();
        CmdList mycml;
        execm.Attach(&mycml);
        if(checktag)
            execm.CheckBGTask();
        else checktag=true;
        yyparse();
        if(mycml.Execute()==YIN_QUIT&&execm.ClearBGTask())
            break;
    }
}

void yyerror(const char* str)
{
    //采用相应全局信息输出相应位置
    //cout<<*(yylval.sWord)<<" part "<<str<<endl;
}

