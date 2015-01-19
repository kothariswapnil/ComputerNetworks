/* 
 * File:   doctor.cpp
 * Author: Swapnil Kothari
 *
 * Created on November 5, 2014, 03:33 PM
 * 
 *  FUNCTIONALITY:
 *			1. The doctor starts two threads doctor1 and doctor2 and creates UDP sockets on static ports 41341 and 42342 respectively.
 *			2. Waits to receive insurance type from patients.
 *			3. Finds the cost for that particular insurance type which is stored in a list.
 *			4. Sends the cost to the patient which had sent the request.
 *			5. Doctor program will not exit. It has to be explicitly killed by CTRL+c (because both patients can 
 *			   send request for cost to same doctor; depends on the appointment selection. It will exit after both doctors serve two patients each).
 *        
 *  USE OF THREAD SYSTEM CALL:
 *			Use of thread to create doctor 1 and doctor 2 concurrently. 
 *			The feature functions properly and has been tested thoroughly. 
 *        
 *  ASSUMPTIONS:
 *			1. It is assumed that there are only two patients (patient 1 and patient 2) and 
 *             the order of execution is same as mentioned in the requirements document.
 *        
 *  GRACEFUL DEGRADATION:
 *			1. Errors have been handled for most expected cases like:
 *				a. Creation of sockets
 *				b. Binding of ports
 *				c. Receiving garbage in data sent from patients
 *			2. In case of UDP sockets, if the healthcenterserver and doctor; and the 
 *			   patients are running on different hosts, duplicate packets may be received.
 *         
 *  DOCUMENTATION:
 *			1. The code is thoroughly commented and easy to understand
 *         
 *  REFERENCES:
 *			1. This code contains some parts taken from BEEJ's guide
 * 
 * 
 */


#include "header.h"

//FUNCTION DECLARATION
void readFiles();
void * processDoctor(void *);

// THIS CLASS STORES THE INSURANCE TYPE AND ITS COST FOR A PARTICULAR DOCTOR
class Insurance_Details{
	private:
		string insuranceType;
		string insuranceCost;
	public:
		Insurance_Details(string t,string c){
			insuranceType=t;
			insuranceCost=c;
		}
		string getType() const { return insuranceType; }
		string getCost() const { return insuranceCost; }		
};

//GLOBAL DECLARATIONS
list<Insurance_Details > insuranceDetailsDoc1;
list<Insurance_Details > insuranceDetailsDoc2;

// MAIN BEGINS
int main(){

	readFiles();
	char doc1Port[MAXDATASIZE],doc2Port[MAXDATASIZE];
	strcpy(doc1Port,PORT_UDP_DOC1);
	strcpy(doc2Port,PORT_UDP_DOC2);
	
	//DECLARATION AND ALLOCATION OF MEMORY FOR THREADS 
	pthread_t *doctorThread[2];
	doctorThread[0]=(pthread_t *)malloc(sizeof(pthread_t));
	doctorThread[1]=(pthread_t *)malloc(sizeof(pthread_t));

	if(pthread_create(doctorThread[0], NULL, processDoctor, doc1Port))				//CREATING UDP SOCKET FOR DOCTOR 1 AND WAITING TO RECEIVE DATA
			perror("cannot create packet thread.\n");
	sleep(1);																		// SLEEP OF 1 SECOND TO ENSURE THAT DOCTOR 1 STARTS BEFORE DOCTOR 2						
	
	if(pthread_create(doctorThread[1], NULL, processDoctor, doc2Port))				//CREATING UDP SOCKET FOR DOCTOR 1 AND WAITING TO RECEIVE DATA
		perror("cannot create packet thread.\n");
	
	// WAIT FOR ALL THREADS TO FINISH AND CLEAR UP ALL MEMORY FOR THREADS
	if(*doctorThread[0])
		pthread_join(*doctorThread[0],NULL);
	if(*doctorThread[0])
		pthread_join(*doctorThread[1],NULL);

	return 0;
}

//READS FILES DOC1.TXT AND DOC2.TXT; AND STORES IT IN APPROPPRIATE DATA STRUCTURE
void readFiles(){

	// VARIABLE DECLARATION
	string read_insuranceType,readCost,line;											
	ifstream doc1 ("doc1.txt"); 														
	ifstream doc2 ("doc2.txt");															
	if(doc1.is_open())																	// OPENING THE FILE DOC1.TXT FOR STORING INTO MEMORY
	{
		while(getline(doc1,line))														// READING FILE DOC1.TXT AND STORING IT IN A LIST
		{
			read_insuranceType=line.substr(0,10);
			readCost=line.substr(11);
			insuranceDetailsDoc1.push_back(Insurance_Details(read_insuranceType,readCost));			
		}
		doc1.close();
	}
	else
	{
		fprintf(stderr, "File doc1.txt not found\n" );
	}
	if(doc2.is_open())																	// OPENING THE FILE DOC2.TXT FOR STORING INTO MEMORY
	{
		while(getline(doc2,line))														// READING FILE DOC1.TXT AND STORING IT IN A LIST
		{
			read_insuranceType=line.substr(0,10);
			readCost=line.substr(11);
			insuranceDetailsDoc2.push_back(Insurance_Details(read_insuranceType,readCost));			
		}
		doc2.close();
	}
	else
	{
		fprintf(stderr, "File doc2.txt not found\n" );
	}
}

void * processDoctor(void *arg)
{
	char port_doc[MAXDATASIZE],doc[MAXDATASIZE];
	strcpy(port_doc,(char *) arg);
	if(!strcmp(port_doc,PORT_UDP_DOC1))
		strcpy(doc,"Doctor 1");
	else if(!strcmp(port_doc,PORT_UDP_DOC2))
		strcpy(doc,"Doctor 2");
	
	int sockfdUDP, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in their_addr; 				// CONNECTOR'S ADDRESS INFORMATION
    int rv;
    struct sockaddr_in self;
    socklen_t addr_len;
    char *ipstr;
    int port = -1;
    socklen_t len = sizeof (self);

    int patientCount=0;							//EACH DOCTOR SERVERS AT THE MOST TWO PATIENTS

    // SET THE SOCKET TYPE TO UDP
    memset(&hints, 0, sizeof hints);
    memset(&servinfo, 0, sizeof servinfo);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
   
    // FILL THE DESTINATION ADDRINFO
    if ((rv = getaddrinfo(HOSTNAME, port_doc, &hints, &servinfo)) != 0) {
        fprintf(stderr, "PHASE 3 %s: Failed to getaddrinfo() for destination %s. Exiting\n",doc, gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        
        // CREATE A UDP SOCKET
        if ((sockfdUDP = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("PHASE 3: Couuld not create socket.\n");
            continue;
        }

        // BIND THE SOCKET
        if (bind(sockfdUDP, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfdUDP);
            perror("PHASE 3: Failed to bind receiving port.\n");
            continue;
        }
        break;
    }

    // CHECK FOR BINDING FAILURE
    if (p == NULL) {
        fprintf(stderr, "PHASE 3 %s: Could not bind socket. Exiting.\n",doc);
        exit(2);
    }
    freeaddrinfo(servinfo); 				// ALL DONE WITH THIS STRUCTURE
    
    // PRINT INFORMATION ABOUT HOST
    len = sizeof (self);

    if (getsockname(sockfdUDP, (struct sockaddr *) &self, &len) == -1){
        perror("PHASE 3: getsockname() failed.\n");
    }
    port = -1;
    port = ntohs(self.sin_port);
    if (port == -1)
        perror("PHASE 3: ntohs() failed.\n");
    ipstr = inet_ntoa(self.sin_addr);
    
    printf("Phase 3 %s has static UDP port number %d and IP address %s.\n", doc, port, ipstr);


    while(patientCount<NUMBER_OF_PATIENTS){
    	addr_len = sizeof their_addr;
		// RECEIVE THE INSURANCE TYPE
		if ((numbytes = recvfrom(sockfdUDP, buf, MAXDATASIZE-1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
			perror("PHASE 3: recvfrom() failed. Exiting.\n");
			exit(1);
		} 
		else {
			//GET CLIENT INFORMATION
		    int clientPort = -1;
		    clientPort = ntohs(their_addr.sin_port);
		    if (clientPort == -1)
		        perror("Phase 3: error extracting PORT using ntohs().\n");

		    // ADD NULL CHARACTER TO THE RECEIVED STRING
		    buf[numbytes] = '\0';
		    
		    printf("Phase 3: %s receives a request from the patient with port number %d and the insurance plan %s.\n",doc,clientPort,buf);
		    
		    patientCount++;		// INCREASE THE PATIENT COUNT

		    string cost;

		    //GET COST FROM LIST FOR THE INSURANCE TYPE FROM THE DOCTOR USING PORT NUMBER 
		    if(!strcmp(port_doc,PORT_UDP_DOC1) )
		    {
		    	for(list<Insurance_Details>:: iterator it=insuranceDetailsDoc1.begin();it!=insuranceDetailsDoc1.end();++it){
		    		if(!strcmp(buf, (*it).getType().c_str()  ) )
		    			cost=(*it).getCost();					
		    	}
		    }
		    else if(!strcmp(port_doc,PORT_UDP_DOC2))
		    {
		    	for(list<Insurance_Details>:: iterator it=insuranceDetailsDoc2.begin();it!=insuranceDetailsDoc2.end();++it){
		    		if(!strcmp(buf, (*it).getType().c_str()  ))
		    			cost=(*it).getCost();							
		    	}
		    }
		    
		    char costFinal[MAXDATASIZE];
		    strcpy(costFinal,cost.c_str());
		    strcpy(buf,"");
		    
		    // SEND THE COST TO THE PATIENT WHO REQUESTED IT USING THE ADDRESS FROM recvfrom(). (BIDIRECTIONAL UDP)
		    if( (numbytes=sendto(sockfdUDP, costFinal, strlen(costFinal), 0,(struct sockaddr *) & their_addr, addr_len)) == -1){
		    	perror("Phase 3: Patient 1 sendto failed. Exiting.\n");
	        	exit(1);
		    }
		    else
		    	printf("Phase 3: %s has sent estimated price %s$ to patient with port number %d.\n",doc,costFinal,clientPort);
		}
	}
    close(sockfdUDP);				// CLOSE THE UDP SOCKET
    return NULL;
}
