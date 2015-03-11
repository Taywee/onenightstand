build = Debug
objects = main.o crypto.o coding.o account.o otp.o
CC = clang
CXX = clang++

commonOptsAll = -Wall -Wextra -std=c++11
commonDebugOpts = -ggdb -O0 -DDEBUG
commonReleaseOpts = -s -O3 -march=native
commonOpts = $(commonOptsAll) $(common$(build)Opts)

compileOptsAll = -c
compileOptsRelease =
compileOptsDebug =
compileOpts = $(commonOpts) $(compileOptsAll) $(compileOpts$(build))

linkerOptsAll =
linkerOptsRelease =
linkerOptsDebug =
linkerOpts = $(commonOpts) $(linkerOptsAll) $(linkerOpts$(build))

compile = $(CXX) $(compileOpts)
link = $(CXX) $(linkerOpts)


.PHONY : all clean

all : onenightstand 

clean :
	-rm -v onenightstand $(objects)

onenightstand : $(objects)
	$(link) -o onenightstand $(objects)

main.o : main.cxx crypto.hxx coding.hxx account.hxx
	$(compile) -o main.o main.cxx 

crypto.o : crypto.cxx crypto.hxx
	$(compile) -o crypto.o crypto.cxx

coding.o : coding.cxx coding.hxx
	$(compile) -o coding.o coding.cxx

account.o : account.cxx account.hxx
	$(compile) -o account.o account.cxx

otp.o : otp.cxx otp.hxx crypto.hxx account.hxx
	$(compile) -o otp.o otp.cxx
