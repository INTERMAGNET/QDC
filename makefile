# Quasi-Definite Compare "QDC" 
# QDC requires the Boost "program-options" library (https://www.boost.org/)
#
# This example makefile is setup to compile QDC on a 64-bit Windows system using  
# gcc version 9.1.0 (Rev3, Built by MSYS2 project) (https://www.msys2.org/) with 
# the Boost libraries installed into c:\msys64\mingw64\lib\ 
# as libboost_program_options-mt.a
#
# On other systems the Boost "program options" library may be installed 
# into a different location or have a different name (for example 
# libprogram_options.a), so the "-L" and "-l" options in the command below 
# will need to adjusted accordingly. 
# 
# On a Windows mingw system use "mingw32-make" to compile QDC as follows:
# <path>/mingw32-make -f makefile QDC 
# or simply
# <path>/mingw32-make


CC=g++ 
cc=gcc

.cpp.o :
	$(CC)  -std=c++11 -c  $*.cpp

.c.o :
	$(cc)  -c $*.c

default : QDC

QDC : QDCMain.o QDC.o QDCInit.o EMApplication.o EMApplicationInit.o gmday.o	
	$(CC)   -o QDC \
	-std=c++11 -static  \
	QDCMain.o QDCInit.o QDC.o \
	EMApp*.o gmday.o \
	-L /c/msys64/mingw64/lib \
	-lboost_program_options-mt


