A small shell like bash with pure C++,based on flex, bison and readline.[![Build Status](https://secure.travis-ci.org/yin8086/YinSh.png?branch=master)](https://travis-ci.org/yin8086/YinSh)

**Support:**
+ external cmd.
+ internal cmd: cd,exit,job,kill,echo,exec.
+ pipeline/redirection.
+ multicmd per line. separated by ';'.
+ background task.  with '&'
+ simple task management: job kill
+ ctrl+c interrupt foreground task.
+ 'exit'  exit the Yinsh.