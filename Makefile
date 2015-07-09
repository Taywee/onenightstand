build = Release
objects = main.o account.o otp.o
libs = cryptpp/libcryptpp.a clipp/libclipp.a
CC = clang
CXX = clang++

commonOptsAll = -Wall -Wextra -std=c++11
commonDebugOpts = -ggdb -O0 -DDEBUG
commonReleaseOpts = -O3 -march=native
commonOpts = $(commonOptsAll) $(common$(build)Opts)

compileOptsAll = -c
compileOptsRelease =
compileOptsDebug =
compileOpts = $(commonOpts) $(compileOptsAll) $(compileOpts$(build))

linkerOptsAll =
linkerOptsRelease = -s
linkerOptsDebug =
linkerOpts = $(commonOpts) $(linkerOptsAll) $(linkerOpts$(build))

compile = $(CXX) $(compileOpts)
link = $(CXX) $(linkerOpts)


.PHONY : all clean

all : onenightstand 

clean :
	-rm -v onenightstand $(objects)

onenightstand : $(objects) $(libs)
	$(link) -o onenightstand $(objects) $(libs)

cryptpp/libcryptpp.a :
	cd cryptpp && git pull
	make -C cryptpp libcryptpp.a

clipp/libclipp.a :
	cd clipp && git pull
	make -C clipp libclipp.a

main.o : main.cxx account.hxx otp.hxx
	$(compile) -o main.o main.cxx 

account.o : account.cxx account.hxx
	$(compile) -o account.o account.cxx

otp.o : otp.cxx otp.hxx
	$(compile) -o otp.o otp.cxx
