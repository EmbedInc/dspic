@echo off
rem
rem   Set up for building a Pascal module.
rem
call build_vars

call src_getbase

call src_builddate "%srcdir%"
