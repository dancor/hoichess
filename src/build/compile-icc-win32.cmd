rem $Id: compile-icc-win32.cmd 1462 2007-12-18 20:49:56Z holger $
rem Build script for HoiChess using Intel C compiler for Windows

md .build-win32
md .build-win32\hoichess
md .build-win32\hoixiangqi

rem ICC options
rem ===========
rem 
rem /GX			enable exceptions (?)
rem /Ox			optimize for maximum speed
rem /QxK		generate code for Pentium III
rem /Qvec-report0	disable all messages from auto-vectorizer

icl /D HOICHESS   /D WIN32  /I win32 /I lib /I chess   /I .  /GX /Ox /QxK /Qvec-report0  /Fe.build-win32\hoichess.exe   /Fo.build-win32\hoichess\    *.cc chess\*.cc   lib\*.cc win32\*.cc
icl /D HOIXIANGQI /D WIN32  /I win32 /I lib /I xiangqi /I .  /GX /Ox /QxK /Qvec-report0  /Fe.build-win32\hoixiangqi.exe /Fo.build-win32\hoixiangqi\  *.cc xiangqi\*.cc lib\*.cc win32\*.cc

