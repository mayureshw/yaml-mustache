PREFIX  ?=  /usr/pkg

CXXFLAGS+=	-I$(PREFIX)/include -O3 -std=c++20
LDFLAGS	+=	-Wl,-rpath=$(PREFIX)/lib -L$(PREFIX)/lib -lyaml-cpp

yaml_mustache: yaml_mustache.cpp
