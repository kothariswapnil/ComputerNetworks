	README
	======
		+-------------------------------------------------------------------------------+
		|                        NAME: SWAPNIL KOTHARI                                  |
		|           PROJECT OBJECTIVE: TO CREATE A CLIENT SERVER PROGRAM TO SIMULATE    |
		|                              HEALTHCENTER WITH PATIENTS AND DOCTOR            |
		|                                                                               |
		|                              THE PROJECT AIMS TO HELP UNDERSTAND THE          |
		|                              UNIX SOCKET PROGRAMMING API                      |
		+-------------------------------------------------------------------------------+
		
		1.  The package includes the following files:
			1. healthcenterserver.cpp
			2. doctor.cpp
			3. patient1.cpp
			4. patient2.cpp
			5. header.h
			6. Makefile
			7. README.md (this file)
			
		2.  Explanation about the files:
			1. healthcenterserver.cpp: 	Contains the code for the healthcenterserver
										The code uses thread library to create child sockets and mutex to lock the availability list.
			2. doctor.cpp: 	Contains the code for two doctor processes.
							The code uses thread library to create two doctor processes that execute concurrently.
			3. patient1.cpp:	Contains code for patient 1 which is: 
								1. TCP socket that sends request for an appointment to the healthcenterserver and 
								2. UDP socket that sends request to doctor to get the cost for insurance.
			4. patient2.cpp: 	Contains code for patient 2 which is:
								1. TCP socket that sends request for an appointment to the healthcenterserver and 
								2. UDP socket that sends request to doctor to get the cost for insurance.
			5. header.h: Contains all the header files and constant declarations.
			6. Makefile: contains the make rules for compiling the programs
			7. README.md: is this file.
			
		3.  To compile the program simply type:
				make
			on the nunki's / Linux (Remove -lsocket from Makefile to run on Linux/Ubuntu) shell prompt.
			The program should compile without errors or warnings. The output should be
			four programs:
				1. healthcenterserver
				2. doctor
				3. patient1
				4. patient2
			  
		4.  To execute the program, open four terminal windows connected to nunki's shell.
			On one of the window, type:
				./healthcenterserver
			On the other type:
				./doctor
			On the other type:
				./patient1
			On the other type:
				./patient2

			Please strictly follow this order of invocation. 
			
			There is user interaction required for both patient1 and patient2.
			
			The doctor program use sleep() to synchronize the doctor processes.
			
			The healthcenterserver program expects the input files as:
				1. availabilities.txt
				2. users.txt
			The doctor program expects the input files as:
				1. doc1.txt
				2. doc2.txt
			The patient1 program expects the input files as:
				1. patient1.txt
				2. patient1insurance.txt
			The patient2 program expects the input files as:
				1. patient2.txt
				2. patient2insurance.txt
	
		5.  Following is description of the specific features of each program:
    
		healthcenterserver:
		=========    
			FUNCTIONALITY:
			1.  The healthcenterserver opens a static TCP port 21341 on localhost
			2.  Waits to receive authentication request from patients.
			3.  Creates a thread to receive messages from a particular patient.
			4.  Authenticates the patient and if successful sends the available appointment list to the patient
			5.  On receiving a selection choice from patient, healthcenterserver confirms the appointment if it is available.
			6.  While changing the status for a particular patient, use of locks ensures consistency. 
			7.  Closes TCP port after the appointment confirmation or rejection is sent by the healthcenterserver to the patient. 
			8.  Once both patients are done; healthcenterserver closes the port and exits

			USE OF THREAD SYSTEM CALL:
				I have used thread to create a child ports to receive data 
				This features simulates real life scenario and enables HealthCenterServer 
				to accept data from multiple patients at the same time.
				The feature functions properly and has been tested thoroughly.

			DEVIATIONS:
			1.  The requirement document requires that the HEALTHCENTERSERVER creates 
				two child  processes using fork(). However, when I tried using 
				fork(); if two patient are running simultaneously, patient 2 was 
				blocked until patient 1 enters a choice for appointment selection.
				I tried to fix this problem by using pipe() system call but couldn't 
				resolve this issue. So I decided to use threading (i.e pthread library 
				in C). This server now can handle multiple requests simultaneously 
				and locks the list when any patient has requested for a reservation using 
				mutex lock. This avoids race condition. In fact the server can now handle 
				more than two patients with a small tweak in the code. 

			ASSUMPTIONS:
			1.	It is assumed that there are only two patients (patient 1 and patient 2) and 
				the order of execution is same as mentioned in the requirements document.

			GRACEFUL DEGRADATION:
			1.	Errors have been handled for most expected cases like:
				a. Creation of sockets
				b. Binding of ports
				c. Receiving garbage in data sent from patients
			2.	In case of UDP sockets, if the healthcenterserver and doctor; and the 
				patients are running on different hosts, duplicate packets may be received.

			DOCUMENTATION:
			1.  The code is thoroughly commented and easy to understand

			REFERENCES:
			1.  This code contains some parts taken from BEEJ's guide

		doctor
		=======
			FUNCTIONALITY:
			1.  The doctor starts two threads doctor1 and doctor2 and creates UDP sockets on static ports 41341 and 42342 respectively.
			2.  Waits to receive insurance type from patients.
			3.  Finds the cost for that particular insurance type which is stored in a list.
			4.  Sends the cost to the patient which had sent the request.
			5.  Doctor program will not exit. It has to be explicitly killed by CTRL+c (because both patients can send request for cost to 
				same doctor; depends on the appointment selection. It will exit after both doctors serve two patients each).

			USE OF THREAD SYSTEM CALL:
				Use of thread to create doctor 1 and doctor 2 concurrently. 
				The feature functions properly and has been tested thoroughly. 

			ASSUMPTIONS:
			1.  It is assumed that there are only two patients (patient 1 and patient 2) and 
				the order of execution is same as mentioned in the requirements document.

			GRACEFUL DEGRADATION:
			1. 	Errors have been handled for most expected cases like:
				a. Creation of sockets
				b. Binding of ports
				c. Receiving garbage in data sent from patients
			2.  In case of UDP sockets, if the healthcenterserver and doctor; and the 
				patients are running on different hosts, duplicate packets may be received.

			DOCUMENTATION:
			1. The code is thoroughly commented and easy to understand

			REFERENCES:
			1. This code contains some parts taken from BEEJ's guide

		patient# (1 and 2)
		=====
			FUNCTIONALITY:
			1.  The patient creates a TCP socket and connects to the healthcenterserver.
			2.  Sends authentication request to the healthcenterserver.
			3.  On successful authentication, patient sends request to healthcenterserver for available appointments. If authentication fails, patient exits. 
			4.  On receiving the availability list from the healthcenterserver, patient enter the appropriate selection choice from the list.
			5.  On receiving confirmation from the healthcenterserver, the patient closes the TCP socket. On rejection from the healthcenterserver, patient exits.
			6.  The patient will create a UDP socket and connect to the doctor using the port number received from the healthcenterserver.
			7.  Sends request for cost for insurance type.
			8.  On receiving the cost, patient program closes the UDP socket and exits 
					  
			ASSUMPTIONS:
			1.	It is assumed that there are only two patients (patient 1 and patient 2) and 
				the order of execution is same as mentioned in the requirements document.

			GRACEFUL DEGRADATION:
			1.	Errors have been handled for most expected cases like:
				a. Creation of sockets
				b. Binding of ports
				c. Receiving garbage in data sent from patients
			2.  In case of UDP sockets, if the healthcenterserver and doctor; and the 
				patients are running on different hosts, duplicate packets may be received.

			DOCUMENTATION:
			1.	The code is thoroughly commented and easy to understand

			REFERENCES:
			1.	This code contains some parts taken from BEEJ's guide.
			2.	Use of functions like reverse() and itoa() have been referred from Wikipedia.