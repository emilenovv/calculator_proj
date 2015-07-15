all: libcomm.so client1 client2 server

libcomm.so: comm_lib.cpp comm_lib.hpp ipc_defs.hpp
	g++ -g -fPIC -Wall comm_lib.cpp -shared -o libcomm.so $(OPT) $(TIME) -lrt
	
client1: client1.cpp libcomm.so comm_lib.hpp ipc_defs.hpp
	g++ -g -Wall -o client1 client1.cpp -Wl,-rpath=. libcomm.so -lrt 
	
client2: client2.cpp libcomm.so comm_lib.hpp ipc_defs.hpp
	g++ -g -Wall -rdynamic -o client2 client2.cpp -ldl
	
server: server.cpp libcomm.so comm_lib.hpp ipc_defs.hpp
	g++ -g -Wall -o server server.cpp -Wl,-rpath=. libcomm.so -lrt
	
clean:
	rm libcomm.so client1 client2 server
