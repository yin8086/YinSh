%{
#include<iostream>
#include<string>
#include<vector>
#include<readline/readline.h>
#include<readline/history.h>
using namespace std;
#include"classheader.h"
#include"cmdparser.tab.h"
static int get_input(char *buf, int size);
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result = get_input(buf, max_size);
%}
%%

">>" {
    //几种符号直接按照默认int搞
    //追加符号
    return APPEND;
}
[|<>&;\n] {
    return *yytext;
}
[^|<>&; \t\n]+ {
	yylval.sWord=new string(yytext);
        return WORD;
}
[ \t]  ;
%%
void GetPrompt(string& opd)
{
    opd.assign("YinSh@:");
    char currwd[MAX_CWD_SIZE];
    getcwd(currwd,MAX_CWD_SIZE);
    opd.append(currwd).append("$ ");
}
static int get_input(char *buf, int size)
{
        //自己定义input函数，采用支持编辑的readline库
        char *line;
        int len=0;
        if (feof(yyin))
                return YY_NULL;
        //指定提示字符串
        string prompt;
        GetPrompt(prompt);
        line = readline(prompt.c_str());
        if (!line)
                return YY_NULL;
        //strcpy(buf, line);
        //至少留下两个字符，一个存放\n，一个为NULL
        strncpy(buf,line,size-2);
        len=strlen(line);
        len=(len)>(size-2)?(size-2):len;
        buf[len]='\0';
        add_history(buf);
        //加上换行之前加入历史
        //strcat(buf, "\n");
        buf[len]='\n';
        buf[len+1]='\0';
        free(line);
        return strlen(buf);

}
int yywrap()
{
  return 1;
}
