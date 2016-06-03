CC = clang
CXX = clang++
PREFIX = /usr/local
BUILD = Release

objects = main.o account.o
libs = -lcrypto -lssl

commonOptsAll = -Wall -Wextra -std=c++11 $(extraFlags)
commonDebugOpts = -ggdb -O0 -DDEBUG
commonReleaseOpts = -O3 -march=native
commonOpts = $(commonOptsAll) $(common$(BUILD)Opts)

compileOptsAll = -c -I cppcodec

ifeq ($(GETTEXT),1)
compileOptsAll += -DGETTEXT
endif
compileOptsRelease =
compileOptsDebug =
compileOpts = $(compileOptsAll) $(compileOpts$(BUILD)) $(commonOpts)

linkerOptsAll =
ifeq ($(GETTEXT),1)
linkerOptsAll += -lgettextlib
endif
linkerOptsRelease =
linkerOptsDebug =
linkerOpts = $(commonOpts) $(linkerOptsAll) $(linkerOpts$(BUILD))

compile = $(CXX) $(compileOpts)

.PHONY : all clean install uninstall i18n

ifeq ($(GETTEXT),1)
all : onenightstand i18n/onenightstand.pot i18n
else
all : onenightstand
endif

clean :
	-rm -v onenightstand $(objects) i18n/onenightstand.pot

install : onenightstand
	install -m 0755 -d $(PREFIX)/bin
	install -m 0755 -t $(PREFIX)/bin onenightstand

uninstall :
	-rm $(PREFIX)/bin/onenightstand

onenightstand : $(objects) $(libs)
	$(CXX) -o onenightstand $(objects) $(libs) $(linkerOpts)
ifeq ($(BUILD),Release)
	strip onenightstand
endif

cryptpp/libcryptpp.a :
	make -C cryptpp libcryptpp.a

clipp/libclipp.a :
	make -C clipp libclipp.a

main.o : main.cxx account.hxx
	$(compile) -o main.o main.cxx

account.o : account.cxx account.hxx
	$(compile) -o account.o account.cxx

i18n/onenightstand.pot : main.cxx
	mkdir -p i18n
	xgettext -d onenightstand -kgettextf -kgettext -o i18n/onenightstand.pot -s main.cxx
