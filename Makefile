all:
	g++ -o client client.cpp -lsocket -lnsl -lresolv 
	g++ -o serverA serverA.cpp -lsocket -lnsl -lresolv
	g++ -o serverB serverB.cpp -lsocket -lnsl -lresolv
	g++ -o serverC serverC.cpp -lsocket -lnsl -lresolv
	g++ -o serverD serverD.cpp -lsocket -lnsl -lresolv
	./client &
	./serverA &
	./serverB &
	./serverC &
	./serverD &

