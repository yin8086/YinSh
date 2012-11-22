#pragma once
#include<string>
#define YIN_QUIT 9
#define VEC_RESER 100
#define MAX_CWD_SIZE 256
struct ReDir
{
    enum {IN,OUT,APPEND}type;
    std::string fname;
};
class BaseCmd;
class PipeLine;
class CmdList;
class ExitCmd;
class CdCmd;
class JobCmd;
class KillCmd;
class EchoCmd;
class ExecCmd;
class ExternCmd;
