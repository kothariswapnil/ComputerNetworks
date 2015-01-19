/* 
 * File:   healthcenterserver.cpp
 * Author: Swapnil Kothari
 *
 * Created on November 5, 2014, 03:33 PM
 * 
 * FUNCTIONALITY:
 *      1.  The healthcenterserver opens a static TCP port 21341 on localhost
 *      2.  Waits to receive autentication request from patients.
 *      3.  Creates a thread to receive messages from a particular patient.
 *		4.  Authenticates the patient and if succesful sends the available appointment list to the patient
 *		5.  On receiving a selection choice from patient, healthcenterserver confirms the appointment if it is available.
 *		6. 	While changing the status for a particular patient, use of locks ensures consistency. 
 *      7.  Closes TCP port after the appointment confirmation or rejection is sent by the healthcenterserver to the patient. 
 *		8.  Once both patients are done; healthcenterserver closes the port and exits
 *
 * USE OF THREAD SYSTEM CALL:
 *      I have used thread to create a child ports to receive data 
 *      This features simluates real life scenario and enables HealthCenterServer 
 * 		to accept data from multiple patients at the same time.
 *      The feature functions properly and has been tested thoroughly.
 *
 * DEVIATIONS:
 *      1. The requirement document requires that the HEALTHCENTERSERVER creates 
 *				two child  processes using fork(). However, when I tried using 
 *				fork(); if two patient are running simultaneously, patient 2 was 
 *				blocked until patient 1 enters a choice for appiontment selection.
 *				I tried to fix this problem by using pipe() system call but couldn't 
 *				resolve this issue. So I decided to use threading (i.e pthread library 
 *				in C). This server now can handle multiple requests simulataneously 
 *				and locks the list whle any patient has requested for a resrvation using 
 *				mutex lock. This avoids race condition. In fact the server can now handle 
 *				more than two patients with a small tweak in the code. 
 *
 * ASSUMPTIONS:
 * 		1. It is assumed that there are only two patients( patient 1 and patient 2) and 
 * 				the order of execution is same as mentioned in the requirements document.
 *
 * GRACEFUL DEGRADATION:
 *      1. Errors have been handled for most expected cases like
 *              a. Creation of sockets
 *              b. Binding of ports
 *              c. Receiving garbage in data sent from patients
 * 		2. In case of UDP sockets, if the healthcenterserver and doctor; and the 
 * 				client are running on different hosts, duplicate packets may be received.
 * 
 * DOCUMENTATION:
 *      1. The code is thoroughly commented and easy to understand
 * 
 * REFERENCES:
 *      1. This code contains some parts taken from BEEJ's guide
 * 
 * 
 */

#include "header.h"

//FUNCTION DECLARTION
int createTCPSocket();
void readFiles();
void *processPatient(void *);

// THIS CLASS STORES THE AVAILABILITIES (INDEX,DAY,TIME,DOC,PORT#, STATUS (RESERVED OR UNRESRVED) )
class Avl_Details{
	private:
		string index;
		string day;
		string time_;
		string doc;
		string port;
		string status;
	public:
		Avl_Details(string i,string d,string t, string doctor, string p, string s){
			index=i;
			day=d;
			time_=t;
			doc=doctor;
			port=p;
			status=s;
		}
		void setStatus(string s)	{	status=s;	}
		string getIndex() const 	{ return index; }
		string getDay() const 		{ return day; }
		string getTime() const 		{ return time_; }
		string getDoc() const 		{ return doc; }
		string getPort() const 		{ return port; }
		string getStatus() const 	{ return status; }
		
};

//GLOBAL DECLARATIONS
list<Avl_Details > availability_list;					// DATA STRUCTURE TO STORE INFORATION IN AVAILABILITIES.TXT
map<string, string > user;								// DATA STRUCTURE TO STORE PATIENT NAME AND PASSWORD FROM PATIENT#.TXT
map<string, string >::iterator it;						// ITERATOR TO ITERATE OVER THE MAP
pthread_mutex_t mutex;									// USING MUTEX TO LOCK THE LIST WHEN A PATIENT IS RESERVING AN APPOINTMENT 						

//MAIN BEGINS
int main ()
{
	//VARIABLE DECLAREATION
	int childFD[2],sockfd; 								// LISTEN ON SOCK_FD, NEW CONNECTION ON CLIENT_FD
	pthread_t *patientThread[2];						// THREADS FOR PATIENTS (CHILD SCOKETS)
	struct sockaddr_storage their_addr;					// CONNECTOR'S ADDRESS INFORMATION
	int patientCount=0;									// KEEP A COUNT OF PATIENTS SO THAT IT DOES NOT EXCEED 2.
	socklen_t sin_size;
	
	//INITIALIZING THE THREADS
	patientThread[0]=NULL;
	patientThread[1]=NULL;

	//READ FILES AVAILABILITIES.TXT AND USERS.TXT; AND STORE IT IN APPROPPRIATE DATA STRUCTURE
	readFiles();

	//INITIALIZE MUTEX
	if(pthread_mutex_init(&mutex, NULL))
	{
		printf("\nCannot initialize mutex.");
		exit(1);
	}

	//CREATE TCP SOCKET AND GET SOCKET FILE DESCRIPTOR
    sockfd = createTCPSocket();

	// LISTEN TO THE TCP SOCKET JUST CREATED
	if (listen(sockfd, BACKLOG) == -1) {
		perror("Phase 1: could not listen to SOCKET. Exiting");
		exit(1);
	}

	
	while(patientCount<NUMBER_OF_PATIENTS)								// IF NUMBER_OF_PATIENTS EXCEEDS PATIENT_COUNT - EXIT THE LOOP
	{
		// ACCPET TWO INCOMING CONNECTIONS FROM PATIENTS ON THE TCP PORT
		sin_size = sizeof their_addr;
		childFD[patientCount] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (childFD[patientCount]== -1) {
			perror("Phase 1: failed to accept incoming connection. Exiting");
			continue;
		}
		
		// CREATE THREAD FOR EACH PATIENT AND PROCESS REQUESTS IN processPatient()FUNCTION
        patientThread[patientCount]= (pthread_t *)malloc(sizeof(pthread_t));
        if(pthread_create(patientThread[patientCount], NULL, processPatient, &childFD[patientCount]))
			perror("cannot create patient thread.\n");
		
		//INCREASE PATIENT COUNT
		patientCount++;
	}

	// WAIT FOR ALL THREADS TO FINISH AND CLEAR UP ALL MEMORY FOR THREADS
	if(*patientThread[0])
		pthread_join(*patientThread[0],NULL);
	if(*patientThread[1])
		pthread_join(*patientThread[1],NULL);

	// CLOSE TCP SOCKET - PHASE 2 DONE.
    close(sockfd);
	return 0;
}

//READS FILES AVAILABILITIES.TXT AND USERS.TXT; AND STORES IT IN APPROPPRIATE DATA STRUCTURE
void readFiles(){
	
	// VARIABLE DECLARATION
	string read_index,read_port,read_day,read_time,read_doc,line,user_name,password;		
	ifstream availabilities ("availabilities.txt"); 										//OPENING THE FILE AVAILAILITIES.TXT FOR STORING INTO MEMORY
	ifstream users("users.txt");															//OPENING THE FILE USERS.TXT FOR STORING INTO MEMORY
	if(availabilities.is_open())															//READING FILE AVAILABILITIES.TXT AND STORING IT IN A LIST
	{
		while(getline(availabilities,line))
		{
			read_index=line.substr(0,1);
			read_day=line.substr(2,3);
			read_time=line.substr(6,4);
			read_doc=line.substr(11,4);
			read_port=line.substr(16);

			// STORE THE INFORMATION IN AVAILABILITESLIST 
			availability_list.push_back(Avl_Details(read_index,read_day,read_time,read_doc,read_port,"unresrved"));			
		}
		availabilities.close();
	}
	else
	{
		fprintf(stderr, "File availabilities.txt not found\n" );
	}

	if(users.is_open())
	{
		line.clear();
		while(getline(users,line))
		{
			user_name=line.substr(0,8);
			password=line.substr(9);
			user[user_name]=password;						// STORING USERNAME AND PASSWORD IN MAP DATA STRUCTURE
		}
		users.close();
	}
	else
	{
		fprintf(stderr, "File users.txt not found\n" );
	}
}

// CREATES A TCP SOCKET AND RETURNS THE SOCKET DESCRIPTOR
int createTCPSocket() {

    // VARIABLE DECLARATION
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int sockfd = -1;

    //  SET THE SOCK TYPE AS TCP
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // CREATE THE SERVER ADDRINFO STRUCTURE
    if ((rv = getaddrinfo(HOSTNAME, PORT_TCP, &hints, &servinfo)) != 0) {
        fprintf(stderr, "Phase 1: could not getaddrinfo %s. Exiting\n",
                gai_strerror(rv));
       	exit(1);
    }

    // LOOP THROUGH ALL THE RESULTS AND CONNECT TO THE FIRST WE CAN
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            fprintf(stderr, "Phase 1: could not create socket.\n");
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            fprintf(stderr, "Phase 1: could not bind %s", PORT_TCP);
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "Phase 1: failed to connect. Exiting.\n");
        exit(2);
    }

    freeaddrinfo(servinfo); 				// ALL DONE WITH THIS STRUCTURE

    //PRINT HOST INFORMATION
    struct sockaddr_in self;
    socklen_t len = sizeof (self);

    if (getsockname(sockfd, (struct sockaddr *) &self, &len) == -1)
        perror("Phase 1: error fetching info using getsockname().\n");

    char *ipstr = NULL;
    int port = -1;
    port = ntohs(self.sin_port);
    if (port == -1)
        perror("Phase 1: error extracting PORT using ntohs().\n");
    ipstr = inet_ntoa(self.sin_addr);

    printf("Phase 1: The Health Center Server has port number %d and IP address %s.\n", port, ipstr);
    return sockfd;

}

//THREAD FOR EACH PATIENT; THE PHASE 1 AND PHASE 2 ARE HANDLED IN THE FUNCTION BELOW
void *processPatient(void *arg)
{
	// VARIABLE DECLARATION
	char *p_uname,*p_pwd,*temp,*temp1,*temp2;
	char buf[MAXDATASIZE],buf_avl[MAXDATASIZE],buf_sel[MAXDATASIZE];
	int numbytes;
	string buf_send;
	int * new_fd;
	
	// READING THE ARGUMENTS 
	new_fd=(int *)arg;
	
	//GET CLIENT INFORMATION
    struct sockaddr_in addr_inet;
	socklen_t len = sizeof (addr_inet);
	int getpeer_check=getpeername(*new_fd, (struct sockaddr *) &addr_inet, &len);
	
	//ERROR  CHECKING
	if(getpeer_check==-1){
			perror("failed getsockname. Exiting");
			exit(1);
	}

	char *clientIPstr = NULL;
    int clientPort = -1;
    clientPort = ntohs(addr_inet.sin_port);
    if (clientPort == -1)
        perror("Phase 1: error extracting PORT using ntohs().\n");
    clientIPstr = inet_ntoa(addr_inet.sin_addr);

    // RECEIVE "AUTHENTICATE" REQUEST FROM PATIENT
	if ((numbytes = recv(*new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
		printf("NOT RECEIVED \n\n\n");
		perror("recv");
		exit(1);
	}
	// ADD NULL CHARACTER TO THE RECEIVED STRING
	buf[numbytes] = '\0';

	//TOKENIZE THE RECEIVED STRING
	temp=strtok(buf," ");
	if(!strcmp(temp,"authenticate")){
		p_uname=strtok(NULL," ");
		p_pwd=strtok(NULL," ");
		printf("Phase 1: The Health Center Server has received request from patient with username %s and password %s.\n",p_uname,p_pwd );
		if(!strcmp( p_pwd, (user.find(p_uname)->second).c_str() ) )
			buf_send="success";
		else
			buf_send="failure";
		if (send(*new_fd, buf_send.c_str(), buf_send.length()+1, 0) == -1)
			perror("send");
		else
			printf("Phase 1: The Health Center Server sends the response %s to patient with username %s.\n",buf_send.c_str(),p_uname);
	
		// RECEIVE "AVAILABLE" REQUEST FROM PATIENT
		if ((numbytes = recv(*new_fd, buf_avl, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
		}
		// ADD NULL CHARACTER TO THE RECEIVED STRING
		buf_avl[numbytes] = '\0';

		// TOKENIZE THE RECEIVED STRING
		temp1=strtok(buf_avl," ");
		if(!strcmp(temp1,"available")){
			printf("Phase 2: The Health Center Server, receives a request for available slots from patients with port number %d and IP address %s.\n",clientPort,clientIPstr);
		
			string avl_slot;
			for(list<Avl_Details>:: iterator i=availability_list.begin(); i!=availability_list.end();++i){
				if(!(*i).getStatus().compare("unresrved"))
				{
					avl_slot+=(*i).getIndex()+" "+(*i).getDay()+" "+(*i).getTime()+"\n";
				}
			}

			// SEND AVAILABLE APPOINTMENTS TO PATIENT
			if (send(*new_fd, avl_slot.c_str(), avl_slot.length()+1, 0) == -1)
					perror("send");
			else
				printf("Phase 2: The Health Center Server sends available time slots to patient with username %s.\n",p_uname);
			
			// RECEIVE "SELECTION" REQUEST FROM PATIENT
			if ((numbytes = recv(*new_fd, buf_sel, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
			}
			// ADD NULL CHARACTER TO THE RECEIVED STRING
			buf_sel[numbytes] = '\0';
			int flagReserved=0;

			// TOKENIZE THE RECEIVED STRING
			temp2=strtok(buf_sel," ");
			if(!strcmp(temp2,"selection")){
				char *recv_index=strtok(NULL," ");
				printf("Phase 2: The Health Center Server receives a request for appointment %s from patient with port number %d and username %s.\n",recv_index,clientPort,p_uname);
				
				//LOCKING THREAD USNIG MUTEX BEFORE RESERVING AN APPOINTMENT
				pthread_mutex_lock(&mutex);										
				
				// ITERATE THROUGH THE LIST AND CHECK IF APPOINTMENT IS AVAILABLE; IF APPIONTMENT IS AVAILABLE THEN CHANGE ITS STATUS TO RESRVED
				for(list<Avl_Details>:: iterator i=availability_list.begin(); i!=availability_list.end();++i){
					if(!(*i).getStatus().compare("unresrved") && !(*i).getIndex().compare(recv_index)){
						(*i).setStatus("reserved");						
						flagReserved=1;
					}
					if(flagReserved){
						string writeStatus ((*i).getIndex());
						writeStatus+=" "+(*i).getStatus();
						string msg_docDetails=(*i).getDoc()+" "+(*i).getPort();
						
						// SEND CONFIRMATION OF APPOINTMENT TO PATIENT
						if (send(*new_fd, msg_docDetails.c_str(), msg_docDetails.length()+1, 0) == -1)
							perror("send");
						else
							printf("Phase 2: The Health Center Server confirms the following appointment %s to patient with username %s.\n",recv_index,p_uname);	
						break;
					}
				}
				
				// UNLOCKING THREAD USNIG MUTEX AFTER RESERVING AN APPOINTMENT
				pthread_mutex_unlock(&mutex);
				if(!flagReserved){
					
					// SEND REJECT TO PATIENT
					string msg_reject="notavailable";
					if (send(*new_fd, msg_reject.c_str(), msg_reject.length()+1, 0) == -1)
							perror("send");
					else{
						printf("Phase 2: The Health Center Server rejects the following appointment %s to patient with username %s.\n",recv_index,p_uname);							
					}
				}
			}
		}
	}
	close(*new_fd);
	return NULL;
}
