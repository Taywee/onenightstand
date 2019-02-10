DEBUG ?=
LIBS = protobuf json-c openssl
FLAGS += -std=c++14 -MMD -MP -Wall -Wextra
CFLAGS += $(shell pkg-config --cflags $(LIBS)) -Icppcodec
LDFLAGS += $(shell pkg-config --libs $(LIBS))

ifdef DEBUG
FLAGS += -O0 -ggdb
CFLAGS += -DDEBUG
else
FLAGS += -O2
CFLAGS += -DNDEBUG
LDFLAGS += -s
endif

# if OpenGL
#UNAME := $(shell uname -s)
#ifeq ($(UNAME),Darwin)
#LDFLAGS += -framework OpenGL
#else
#LDFLAGS += -lGL
#endif

CXX ?= c++

COMPILE = $(CXX) $(CFLAGS) $(FLAGS) -c
LINK = $(CXX) $(LDFLAGS) $(FLAGS)

SOURCES = main.cxx
PBSOURCES = account.proto
OBJECTS =  $(SOURCES:.cxx=.o) $(PBSOURCES:.proto=.pb.o)
DEPENDENCIES = $(OBJECTS:.o=.d)

EXECUTABLE = onenightstand

.PHONY: all clean

all: $(EXECUTABLE)

test:
	echo $(DEPENDENCIES)

-include $(DEPENDENCIES)

clean:
	-rm -v $(OBJECTS) $(DEPENDENCIES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINK) -o$@ $^

%.o : %.cxx
	$(COMPILE) -o $@ $<

%.pb.o: %.pb.cc
	$(COMPILE) -o $@ $<

%.pb.cc %.pb.h: %.proto
	protoc --cpp_out=. $<

install : onenightstand
	install -m 0755 -d $(PREFIX)/bin
	install -m 0755 -t $(PREFIX)/bin onenightstand

uninstall :
	-rm $(PREFIX)/bin/onenightstand
