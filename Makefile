all: build

build: main.cpp
	g++ main.cpp botsender.h botsender.cpp -o botsender -lcurl -ljsoncpp
