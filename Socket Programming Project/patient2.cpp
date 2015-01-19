/* 
 * File:   patient2.cpp
 * Author: Swapnil Kothari
 *
 * Created on November 5, 2014, 03:33 PM
 * 
 *  FUNCTIONALITY:
 *         1. The patient creates a TCP socket and connects to the healthcenterserver.
 *         2. Sends authentication request to the healthcenterserver.
 *         3. On succesful authentication, patient sends request to healthcenterserver for available appointments. 
 *			  If authentication fails, patient exits. 
 *         4. On receiving the availability list from the healthcenterserver, patient enter the appropriate 
 *			  selection choice from the list.
 *         5. On receiving confirmation from the healthcenterserver, the patient closes the TCP socket. 
 *			  On rejection from the healthcenterserver, patient exits.
 *         6. The patient will create a UDP socket and connect to the doctor using the port number recieved from 
 *			  the healthcenterserver.
 *         7. Sends request for cost for insurance type.
 *         8. On recieving the cost, patient program closes the UDP socket and exits 
 *              
 *  ASSUMPTIONS:
 *         1. It is assumed that there are only two patients (patient 1 and patient 2) and 
 *            the order of execution is same as mentioned in the requirements document.
 *        
 *  GRACEFUL DEGRADATION:
 *         1. Errors have been handled for most expected cases like:
 *             a. Creation of sockets
 *             b. Binding of ports
 *             c. Receiving garbage in data sent from patients
 *         2.  In case of UDP sockets, if the healthcenterserver and doctor; and the 
 *             patients are running on different hosts, duplicate packets may be received.
 *         
 *  DOCUMENTATION:
 *         1. The code is thoroughly commented and easy to understand
 *         
 *  REFERENCES:
 *         1. This code contains some parts taken from BEEJ's guide.
 *         2. Use of functions like reverse() and itoa() have been referred from Wikipedia.
 * 
 * 
 */


#include "header.h"
void create_Process_UDPSocket(char *);
string readInsuranceFile();

// IMPLEMENTATION OF STRING REVERSE
void reverse(string s,int length)
 {
	int i,j;
	for (i = 0, j = length-1; i<j; i++, j--) {
 		// SWAP VARIBALES
 		s[i]^=s[j];
 		s[j]^=s[i];
 		s[i]^=s[j];
	}
 }

// IMPLEMENTATION OF ITOA()
void itoa(int n, string s, int base)
 {
 	int i, sign;
	if ((sign = n) < 0)						// RECORD SIGN  							 
		n = -n;          					// MAKE N POSITIVE 
		i = 0;
	do {       								// GENERATE DIGITS IN REVERSE ORDER 
		s[i++] = n % base + '0';   			// GET NEXT DIGIT 
	} while ((n /= base) > 0);     			// DELETE IT 
	if (sign < 0)
	s[i++] = '-';
	s[i] = '\0';
	reverse(s,i);
}


int main ()
{
	// VVARIABLE DECLARATION
	int sockfd, numbytes;
	char buf[MAXDATASIZE],buf_list[MAXDATASIZE],bufStatus[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in addr_inet;
	int rv;
	
	// OPAN AND READ FILE PATIENT1.TXT FOR STORING USERNAME AND PASSWORD INTO MEMORY
	ifstream patient2("patient2.txt");						
	string user_name,password,line;
	
	if(patient2.is_open())
	{
		line.clear();
		while(getline(patient2,line))
		{
			// STORE THE USERNAME AND PASSWORD IN LOCAL VARIABLES
			user_name=line.substr(0,8);
			password=line.substr(9);
		}
		patient2.close();
	}
	else
	{
		cout<<"File patient2.txt not found"<<endl;
	}	
	
	// INITIALIZING SOCKET VARIALBES
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(HOSTNAME, PORT_TCP, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}	
	
	// LOOP THROUGH ALL THE RESULTS AND CONNECT TO THE FIRST WE CAN
	for(p = servinfo; p != NULL; p = p->ai_next) {
		//CREATE TCP SOCKET AND GET SOCKET FILE DESCRIPTOR
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("patient2: socket");
			continue;
		}
		// CONNECT TO THE SERVER
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("patient2: connect");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "patient2: failed to connect\n");
		return 2;
	}

	socklen_t len=sizeof(struct sockaddr);	
	int getsock_check=getsockname(sockfd, (struct sockaddr *) &addr_inet, &len);
	//ERROR  CHECKING
	if(getsock_check==-1){
  		perror("getsockname");
  		exit(1);
	} 
    printf("Phase 1: Patient 2 has TCP port number %u and IP address %s.\n",ntohs(addr_inet.sin_port),inet_ntoa(addr_inet.sin_addr));
	
	freeaddrinfo(servinfo); 	// ALL DONE WITH THIS STRUCTURE
	

	string msg="authenticate "+user_name+" "+password;
	
	// SEND "AUTHTICATE" MESSAGE TO SERVER
	if (send(sockfd,msg.c_str(),msg.length()+1,0)==-1)
	{
		perror("send");
	}
	else
		printf("Phase 1: Authentication request from Patient 2 with username %s and password %s has been sent to the Health Center Server.\n",user_name.c_str(), password.c_str());
	
	// RECEIVE MESSAGE FROM HEALTHCENTERSERVER
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	else{
		// ADD NULL CHARACTER TO THE RECEIVED STRING
		buf[numbytes] = '\0';

		printf("Phase 1: Patient 2 authentication result: %s\n",buf);
		printf("Phase 1: End of Phase 1 for Patient 2.\n");
		if(strcmp(buf,"failure")==0)								// IF AUTHENCATION IS FAILED FROM SERVER, EXIT	
			exit(1);
		else if(strcmp(buf,"success")==0)							// IF AUTHENCATION IS SUCCESSFUL FROM SERVER, SEND REQUEST FOR APPOINTMENT SCHEDULE	
		{
			// SEND "AVAILABLE" MESSAGE TO SERVER
			string msg_avl ("available ");
			if (send(sockfd,msg_avl.c_str(),msg_avl.length()+1,0)==-1)	// SEND AVAILABLE TO SERVER TO GET LIST OF AVAILABLE SLOTS
			{
				perror("send");
			}

			// RECEIVE AVAILABILITY LIST FROM THE SERVER
			if ((numbytes = recv(sockfd, buf_list, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			else{
				// ADD NULL CHARACTER TO THE RECEIVED STRING
				buf_list[numbytes] = '\0';								// RECEIVED LIST FROM SERVER
				printf("Phase 2: The following appointments are available for Patient 2:\n%s",buf_list);
				
				string buf_listCopy=buf_list;
				int i=0,flagFound=0;
				int l=buf_listCopy.length()-1;
				string indexToSearch;
				while(i<l){
					indexToSearch+=buf_listCopy.substr(i,1);
					i=i+11;
				}

				// GET APPIOINTMENT CHOICE FROM PATIENT 
				string msg_select="selection ";
				do{
					printf("Please enter the preferred appointment index and press enter:");
					string indexChoice;									
					cin>>indexChoice;
					for(unsigned int k=0;k<indexToSearch.length();k++){
						if(indexChoice[0]==indexToSearch[k])  {
							msg_select+=indexChoice;
							flagFound=1;
						}
					}
				}while(flagFound==0);

				// SEND "SELECTION" MESSAGE TO SERVER TO CONFIRM AN APPOINTMENT
				if (send(sockfd,msg_select.c_str(),msg_select.length()+1,0)==-1){					
						perror("send");
				}
				// RECEIVE CONFIRMATION OR REJECTION MESSAGE FROM SERVER
				if ((numbytes = recv(sockfd, bufStatus, MAXDATASIZE-1, 0)) == -1) {
					perror("recv");
					exit(1);
				}
				else{
					bufStatus[numbytes] = '\0';												
					 
					if(!strcmp(bufStatus,"notavailable")){	// RECEIVED REJECT FROM SERVER
						printf("Phase 2: The requested appointment from Patient 2 is not available. Exiting...\n");
						close(sockfd);
						exit(1);
					}
					else{									// RECEIVED APPOINTMENT CONFIRMATION FROM SERVER
						char *temp;
						temp=strtok(bufStatus," ");
						strcpy(temp," ");
						char *doc_port=strtok(NULL," ");
						printf("Phase 2: The requested appointment is available and reserved to Patient 2. The assigned doctor port number is %s.\n",doc_port);
						close(sockfd);						// PHASE 2 DONE. CLOSE THE TCP SOCKET FOR PATIENT 1

						//CREATE UDP SOCKET AND SEND REQUEST TO DOCTOR FOR COST
						create_Process_UDPSocket(doc_port);						
					}
				}
			}		
		}
	}
	close(sockfd); 		// CLOSE THE TCP SOCKET	
	return 0;
}
void create_Process_UDPSocket(char *docPort){
	
	//3 LINES BELOW ARE USED TO REMOVE THE EXTRA CHARACTERS LIKE '\N' OR '\T' IN THE PORT# TO AVOID ERRORS IN GETADDRINFO() FUNCTION. FOUND THE SOLUTION FROM STACKOVERFLOW.COM
	char doc_port[MAXDATASIZE];
	int pp = atoi(docPort);
    sprintf( doc_port, "%d", pp );		
	
	// VARIABLE DECLARATION
	int sockfd_UDP, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in their_addr; 			// CONNECTOR'S ADDRESS INFORMATION
    int rv;
    struct sockaddr_in self;
    socklen_t addr_len;

    // SET THE SOCKET TYPE TO UDP
	memset(&hints, 0, sizeof hints);
	memset(&servinfo, 0, sizeof servinfo);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    // FILL THE DESTINATION ADDRINFO
    if ((rv = getaddrinfo(HOSTNAME, doc_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // LOOP THROUGH ALL THE RESULTS AND MAKE A SOCKET
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_UDP = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("PHASE 3: Couuld not create socket.\n");
            continue;
        }
        break;
    }

    if (p == NULL) {
        perror("PHASE 3: Couuld not bind socket.\n");
        exit(2);
    }
    
    addr_len = sizeof their_addr;
    string insurance_type1=readInsuranceFile();
    char insurance_type[MAXDATASIZE];
    strcpy(insurance_type,insurance_type1.c_str());


    //SEND THE INSURANCE TYPE
    if (( numbytes = sendto(sockfd_UDP, insurance_type, strlen(insurance_type), 0,p->ai_addr,p->ai_addrlen) ) == -1) {
        perror("Phase 3: Patient 2 sendto failed. Exiting. \n");
        exit(1);
    }
    else{
    	// PRINT INFORMATION ABOUT PATIENT
	    socklen_t len = sizeof (self);

	    if (getsockname(sockfd_UDP, (struct sockaddr *) &self, &len) == -1)
	        perror("PHASE 3: getsockname() failed.\n");

	    int port = -1;
	    port = ntohs(self.sin_port);
	    //ERROR  CHECKING
	    if (port == -1)
	        perror("PHASE 3: ntohs() failed.\n");

		struct hostent *hp;
	    char *ipstr=NULL;
	    hp = (struct hostent *)gethostbyname(HOSTNAME);
	    self.sin_addr = *((struct in_addr *)hp->h_addr);
	    ipstr = inet_ntoa(self.sin_addr);

	    printf("Phase 3: Patient 2 has dynamic UDP port number %d and IP address %s.\n",port, ipstr);
    }
    freeaddrinfo(servinfo); 				// ALL DONE WITH THIS STRUCTURE
    if ((numbytes = recvfrom(sockfd_UDP, buf, MAXDATASIZE - 1, 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("PHASE 3: recvfrom() failed. Exiting.\n");
        exit(1);
	}
	else{
		// ADD NULL CHARACTER TO THE RECEIVED STRING
		buf[numbytes]='\0';
		char *docIPstr = NULL;

		// GET THE IP ADDRESS
		docIPstr=inet_ntoa(their_addr.sin_addr);
		
		printf("Phase 3: The cost estimation request from Patient 2 with insurance plan %s has been sent to doctor with port number %s and IP address %s.\n",insurance_type,doc_port,docIPstr );
		printf("Phase 3: Patient 2 receives %s$ estimation from doctor with port number %s and name %s.\n",buf,doc_port,(!strcmp(doc_port,PORT_UDP_DOC1))?"Doctor 1":"Doctor 2" );
		printf("Phase 3: End of Phase 3 for Patient 2.\n");
	}
	close(sockfd_UDP);			//CLOSE THE UDP SOCKET
}

string readInsuranceFile(){

	// OPENING THE FILE PATIENT1INSURANCE FOR STORING INSURANCE TYPE IN A STRING VARIABLE
	ifstream patient2insurance("patient2insurance.txt");						
	string insuranceType,line;
	if(patient2insurance.is_open())
	{
		while(getline(patient2insurance,line))
		{
			insuranceType=line;	
		}
		patient2insurance.close();
	}
	else
	{
		fprintf(stderr, "File patient2.txt not found\n");
	}
	return insuranceType;
}
