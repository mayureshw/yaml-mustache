PREFIX  ?=  /usr/pkg

CXXFLAGS+=	-I$(PREFIX)/include -O3 -std=c++17
LDFLAGS	+=	-Wl,-rpath=$(PREFIX)/lib -L$(PREFIX)/lib -lyaml-cpp

yaml_mustache: yaml_mustache.cpp
