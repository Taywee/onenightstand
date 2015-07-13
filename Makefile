build = Release
objects = main.o account.o otp.o
libs = cryptpp/libcryptpp.a clipp/libclipp.a
CC = clang
CXX = clang++

commonOptsAll = -Wall -Wextra -std=c++11 $(extraFlags)
commonDebugOpts = -ggdb -O0 -DDEBUG
commonReleaseOpts = -O3 -march=native
commonOpts = $(commonOptsAll) $(common$(build)Opts)

compileOptsAll = -c

ifeq ($(USE_GETTEXT),1)
compileOptsAll += -DGETTEXT
endif
compileOptsRelease =
compileOptsDebug =
compileOpts = $(compileOptsAll) $(compileOpts$(build)) $(commonOpts) 

linkerOptsAll =
ifeq ($(USE_GETTEXT),1)
linkerOptsAll += -lgettextlib
endif
linkerOptsRelease =
linkerOptsDebug =
linkerOpts = $(commonOpts) $(linkerOptsAll) $(linkerOpts$(build))

compile = $(CXX) $(compileOpts)

.PHONY : all clean

ifeq ($(USE_GETTEXT),1)
all : onenightstand i18n/onenightstand.pot
else
all : onenightstand
endif

clean :
	-rm -v onenightstand $(objects) i18n/onenightstand.pot

onenightstand : $(objects) $(libs)
	$(CXX) -o onenightstand $(objects) $(libs) $(linkerOpts)
ifeq ($(build),Release)
	strip onenightstand
endif

cryptpp/libcryptpp.a :
	make -C cryptpp libcryptpp.a

clipp/libclipp.a :
	make -C clipp libclipp.a

main.o : main.cxx account.hxx otp.hxx
	$(compile) -o main.o main.cxx 

account.o : account.cxx account.hxx
	$(compile) -o account.o account.cxx

otp.o : otp.cxx otp.hxx
	$(compile) -o otp.o otp.cxx

i18n/onenightstand.pot : main.cxx
	mkdir -p i18n
	xgettext -d onenightstand -kgettextf -kgettext -o i18n/onenightstand.pot -s main.cxx
