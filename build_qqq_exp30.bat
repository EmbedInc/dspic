@echo off
rem
rem   BUILD_QQ2_EXP30
rem
rem   Build the QQ2 firmware from the QQ1 library.
rem
setlocal
set srcdir=qq1
set buildname=qq2
set using_xc16=false
set heapsize=0
set pictype=qq4

call treename_var (cog)src/dspic dir
cd /d "%dir%"
make_dspic_inc %pictype%

call src_get_ins_dspic dspic lcase
call src_get_ins_dspic dspic fwtype
call src_get_ins_dspic dspic std_def
call src_get_ins_dspic dspic std

call src_get_ins_dspic dspic port
call src_get_ins_dspic dspic task
call src_get_ins_dspic dspic uart
call src_get_ins_dspic dspic uart_setup

call src_ins_dspic %srcdir% %buildname%lib -set make_version
call src_get_ins_dspic %srcdir% %buildname%

call src_dspic %srcdir% %buildname%_clock
call src_dspic %srcdir% %buildname%_cmd
call src_dspic %srcdir% %buildname%_cmds
call src_dspic %srcdir% %buildname%_init
call src_dspic %srcdir% %buildname%_main
call src_dspic %srcdir% %buildname%_port
call src_dspic %srcdir% %buildname%_strt
call src_dspic %srcdir% %buildname%_task
call src_dspic %srcdir% %buildname%_trap
call src_dspic %srcdir% %buildname%_uart

call src_exp30 %srcdir% %buildname%

rem   Do SRC_GET on files just so that promotion is performed when enabled.
rem
call src_get %srcdir% doc.txt
call src_get %srcdir% build.bat
