loser_peer: MD5.o loser.o loserFS.o main.o
	g++ -std=c++11 main.o loserFS.o loser.o MD5.o -o loser_peer

MD5.o:
	g++ -std=c++11 -c MD5.cpp -o MD5.o

loser.o:
	g++ -std=c++11 -c loser.cpp -o loser.o

loserFS.o:
	g++ -std=c++11 -c loserFS.cpp -o loserFS.o

main.o:
	g++ -std=c++11 -c main.cpp -o main.o

clean:
	rm -f main.o loserFS.o loser.o MD5.o loser_peer
