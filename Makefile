all: libcomm.so client1 client2 server

libcomm.so: libcomm.cpp libcomm.h
	g++ -g -fPIC -Wall libcomm.cpp -shared -o libcomm.so -lrt
	
client1: client1.cpp libcomm.so libcomm.h
	g++ -g -Wall -o client1 client1.cpp libcomm.so -lrt -lpthread
	
client2: client2.cpp libcomm.so libcomm.h
	g++ -g -Wall -rdynamic -o client2 client2.cpp -ldl
	
server: server.cpp libcomm.so libcomm.h
	g++ -g -Wall -o server server.cpp libcomm.so -lrt
	
clean:
	rm libcomm.so client1 server