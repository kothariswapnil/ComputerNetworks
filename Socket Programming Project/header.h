/* 
 * File:   header.h
 * Author: Swapnil Kothari
 *
 * Created on November 5, 2014, 03:33 PM
 * 
 *  FUNCTIONALITY:
 *			   Contains all the header files and constant declarations.
 */
 
#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <map>

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT_TCP 			"21341"			// THE PORT PATIENTS WILL BE CONNECTING TO HEALTHCENTERSERVER
#define PORT_UDP_DOC1 		"41341"			// THE PORT PATIENTS WILL BE CONNECTING TO DOC1
#define PORT_UDP_DOC2 		"42341"			// THE PORT PATIENTS WILL BE CONNECTING TO DOC2
#define BACKLOG 			10 				// HOW MANY PENDING CONNECTIONS QUEUE WILL HOLD
#define HOSTNAME 			"nunki.usc.edu"	// NAME OF THE HOST 
#define MAXDATASIZE 		1024			// SIZE OF THE BUFFER
#define NUMBER_OF_PATIENTS 	2 				// NUMBER OF PATIENTS. 
#define SLEEP_TIME 			1 				// 1 SECOND OF SLEEP TO START THE PROCESSES IN ORDER AS MENTIONED IN REQUIREMENTS DOCUMENT 
using namespace std;
