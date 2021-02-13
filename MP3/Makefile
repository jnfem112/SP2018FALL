all:
	g++ -std=c++11 -c csie_box.cpp -o csie_box.o
	g++ -std=c++11 -c csie_box_server.cpp -o csie_box_server.o
	g++ -std=c++11 -c csie_box_client.cpp -o csie_box_client.o
	g++ -std=c++11 csie_box_server.o csie_box.o -o csie_box_server
	g++ -std=c++11 csie_box_client.o csie_box.o -o csie_box_client

clean:
	rm -f csie_box_server.o csie_box_client.o csie_box.o csie_box_server csie_box_client
