all:
	g++ -o server -g -Wall ./src/server.cpp
	g++ -o client -g -Wall ./src/client.cpp

server:
	./server

client:
	./client