.PHONY: all

all: ysh

ysh: src/ysh.cpp src/rang.hpp
	mkdir -p build
	$(CXX) -std=c++14 -o build/ysh.a src/ysh.cpp

clean: build
	rm -rf build
