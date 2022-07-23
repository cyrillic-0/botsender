all: build

build: main.cpp
	g++ main.cpp -o main -lcurl -ljsoncpp
