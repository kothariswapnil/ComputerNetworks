all: healthcenterserver doctor patient1 patient2
healthcenterserver: healthcenterserver.cpp 
	g++ -Wall -o healthcenterserver healthcenterserver.cpp -lsocket -lresolv -lnsl -lpthread
doctor: doctor.cpp
	g++ -Wall -o doctor doctor.cpp -lsocket -lresolv -lnsl -lpthread
patient1: patient1.cpp 
	g++ -Wall -o patient1 patient1.cpp -lsocket -lresolv -lnsl
patient2: patient2.cpp 
	g++ -Wall -o patient2 patient2.cpp -lsocket -lresolv -lnsl
clean:
	rm -f patient2 patient1 doctor healthcenterserver
