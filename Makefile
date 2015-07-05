build = Debug
objects = main.o account.o otp.o
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

onenightstand : $(objects) cryptpp/libcryptpp.a
	$(link) -o onenightstand $(objects) cryptpp/libcryptpp.a

cryptpp/libcryptpp.a :
	cd cryptpp && git pull
	make -C cryptpp libcryptpp.a

main.o : main.cxx account.hxx otp.hxx
	$(compile) -o main.o main.cxx 

account.o : account.cxx account.hxx
	$(compile) -o account.o account.cxx

otp.o : otp.cxx otp.hxx
	$(compile) -o otp.o otp.cxx
