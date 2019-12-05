all: client server

client:
	g++ -std=c++11 client.cxx -o client -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks -framework SDL2

server:
	g++ -std=c++11 server.cxx -o server -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks -framework SDL2