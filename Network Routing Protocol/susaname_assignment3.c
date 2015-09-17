/**
 * @susaname_assignment3
 * @author  Susana Dsa <susaname@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/stat.h>
#include <ctype.h>

#include "../include/global.h"
#include "../include/logger.h"

//#define STDIN 0;
/*Data Structures definitions Start*/

/**
 * updateFields struct
 *
 *@datafield otherServerIP IP for the other server in the network
 *@datafield otherServerPort Listening Port for the other server in the network
 *@datafield otherServerID ID assigned to this server whose IP and Port is provided above
 *@datafield otherServerCost min cost identified at a given instant from current server to this other server in the network
 *
 * Holds data for the all nodes in the network
 */
struct updateFields {
	uint32_t otherServerIP;
	uint16_t otherServerPort;
	char otherServerCharIP[16];
	uint16_t otherServerID;
	uint16_t otherServerCost;
};

/**
 * routingUpdatePacket struct
 *
 *@datafield numberOfUpdateFields indicates number of nodes apart from the current whose cost is to be shared to other routers in the network
 *@datafield myServerPort Current Server listening Port
 *@datafield myServerIP Current Server IP
 *@datafield struct updateFields contains data for other nodes in the network
 *
 * Holds data for the various fields of the update packet to be sent to neighbors in the network
 */
struct routingUpdatePacket {
	uint16_t numberOfUpdateFields;
	uint16_t myServerPort;
	uint32_t myServerIP;
	struct updateFields otherServerUpdateFields[6];
};

/**
 * routingTable struct
 *
 *@datafield numberOfUpdateFields indicates number of nodes apart from the current whose cost is to be shared to other routers in the network
 *@datafield myServerPort Current Server listening Port
 *@datafield myServerIP Current Server IP
 *@datafield struct updateFields contains data for other nodes in the network
 *
 * Holds data for the various fields of the update packet to be sent to neighbors in the network
 */
struct routingTable {
	uint16_t toServerID;
	int16_t fromServerID;
	uint16_t totalCost;
	int8_t counter;
};

//uint32_t
//uint16_t

/* Data Structures definitions End*/

/*
 * Global variables definition Start
 */

int timeOut;
uint16_t numberOfServers;
uint16_t numberOfNeighbors;
//uint16_t myPortNumber;
uint16_t myServerID;
//uint32_t myIP;
struct updateFields nodeDetails[6];
struct updateFields nodeDetailsToBeSentToNeighbors[6];
struct updateFields nodeDetailsReceivedFromNeighbor[6];
struct routingTable routingDetails[6];
struct routingUpdatePacket initialRoutingPacketFromTopologyFile;
struct routingUpdatePacket routingPacketToBeSentToNeighbors;
struct routingUpdatePacket routingPacketReceivedFromNeighbor;
char myPort[5];
fd_set master;
uint16_t costMatrix[6][6];
uint16_t neighbor[5];
int udpSocket = 0;
int packetCounter = 0;
/*
 * Global variables definition End
 */

/*
 * Global constant definition Start
 */
uint16_t INF = 65535;
uint16_t padding = 0;
/*
 * Global constant definition End
 */

/**
 * fnUpdateCostMatrixFromTopologyFile function
 *
 * Puts the cost values in costMatrix based on packet stored in global struct routingPacketReceivedFromNeighbor
 */
void fnUpdateCostMatrixFromTopologyFile() {
	/*
	 * Declaration of variables Start
	 */
	int i, j, index;
	/*
	 * Declaration of variables End
	 */

	for (i = 1; i <= numberOfServers; i++) {
		if (initialRoutingPacketFromTopologyFile.myServerIP
				== nodeDetails[i].otherServerIP) {
			for (j = 1; j <= numberOfServers; j++) {
				index =
						initialRoutingPacketFromTopologyFile.otherServerUpdateFields[j].otherServerID;
				costMatrix[i][index] =
						initialRoutingPacketFromTopologyFile.otherServerUpdateFields[j].otherServerCost;
			}
			break;
		}
	}
}

/**
 * fnCreatePacket function
 *
 * Creates the packet to be sent to neighbor
 */
char * fnCreatePacket() {
	/*
	 * Declaration of variables Start
	 */
	int fieldSize, bufferPointerPosition;
	char * bufferForPacketToBeSent;
	uint16_t numberOfUpdateFieldsInNetworkByteOrder;
	uint16_t portNumberInNetworkByteOrder;
	uint16_t serverIDInNetworkByteOrder;
	uint16_t linkCostInNetworkByteOrder;
	uint32_t ipInNetworkByteOrder;
	int i;
	/*
	 * Declaration of variables End
	 */

//	numberOfUpdateFieldsInNetworkByteOrder = malloc(2);
//	portNumberInNetworkByteOrder = malloc(2);
//	serverIDInNetworkByteOrder = malloc(2);
//	linkCostInNetworkByteOrder = malloc(2);
//	ipInNetworkByteOrder = malloc(4);
	size_t sizeOfPacket = sizeof(uint32_t) * 2
			+ (3 * sizeof(uint32_t) * numberOfServers);
	bufferForPacketToBeSent = malloc(sizeOfPacket);

	/*Copy the struct into the packet to be sent*/
	bufferPointerPosition = 0;
	fieldSize = 2;
//	printf("routingPacketToBeSentToNeighbors.numberOfUpdateFields = %d\n",
//			routingPacketToBeSentToNeighbors.numberOfUpdateFields);
	numberOfUpdateFieldsInNetworkByteOrder = htons(
			routingPacketToBeSentToNeighbors.numberOfUpdateFields);
//	printf("numberOfUpdateFieldsInNetworkByteOrder = %d\n",
//			ntohs(numberOfUpdateFieldsInNetworkByteOrder));
//	printf("bufferForPacketToBeSent + bufferPointerPosition = %d\n",
//			(*bufferForPacketToBeSent + bufferPointerPosition));
//	memcpy(bufferForPacketToBeSent,
//				"n", fieldSize);
	memcpy((bufferForPacketToBeSent + bufferPointerPosition),
			&numberOfUpdateFieldsInNetworkByteOrder, fieldSize);
//	memcpy(bufferForPacketToBeSent, &numberOfUpdateFieldsInNetworkByteOrder,
//			fieldSize);
//	memcpy(bufferForPacketToBeSent,
//			"n", fieldSize);
//	printf("Buffer packet to be sent = %s\n", (char *) bufferForPacketToBeSent);

	printf("Port number before = %d\n",
			routingPacketToBeSentToNeighbors.myServerPort);
	bufferPointerPosition += fieldSize;
	portNumberInNetworkByteOrder = htons(
			routingPacketToBeSentToNeighbors.myServerPort);
//	printf("bufferForPacketToBeSent + bufferPointerPosition = %d\n",
//			(*bufferForPacketToBeSent + bufferPointerPosition));
	memcpy((bufferForPacketToBeSent + bufferPointerPosition),
			&portNumberInNetworkByteOrder, fieldSize);

	printf("Port number = %d\n", portNumberInNetworkByteOrder);
	printf("Port number = %d\n", ntohs(portNumberInNetworkByteOrder));

//	printf("Buffer packet to be sent = %s", (char *) bufferForPacketToBeSent);

	bufferPointerPosition += fieldSize;
	fieldSize = 4;
//	ipInNetworkByteOrder = htonl(routingPacketToBeSentToNeighbors.myServerIP);
	ipInNetworkByteOrder = (routingPacketToBeSentToNeighbors.myServerIP);
	memcpy((bufferForPacketToBeSent + bufferPointerPosition),
			&ipInNetworkByteOrder, fieldSize);
//	printf("Buffer packet to be sent = %s", (char *) bufferForPacketToBeSent);

	for (i = 1; i <= numberOfServers; i++) {
		nodeDetailsToBeSentToNeighbors[i] =
				routingPacketToBeSentToNeighbors.otherServerUpdateFields[i];
	}

	for (i = 1; i <= routingPacketToBeSentToNeighbors.numberOfUpdateFields;
			i++) {
		bufferPointerPosition += fieldSize;
		fieldSize = 4;
		ipInNetworkByteOrder =
				(nodeDetailsToBeSentToNeighbors[i].otherServerIP);
		memcpy((bufferForPacketToBeSent + bufferPointerPosition),
				&ipInNetworkByteOrder, fieldSize);

//		printf("Buffer packet to be sent = %s",
//				(char *) bufferForPacketToBeSent);

		bufferPointerPosition += fieldSize;
		fieldSize = 2;
		portNumberInNetworkByteOrder = htons(
				nodeDetailsToBeSentToNeighbors[i].otherServerPort);
		memcpy((bufferForPacketToBeSent + bufferPointerPosition),
				&portNumberInNetworkByteOrder, fieldSize);

//		printf("Buffer packet to be sent = %s",
//				(char *) bufferForPacketToBeSent);

		/*Add padding fields*/
		bufferPointerPosition += fieldSize;
		fieldSize = 2;
		memcpy((bufferForPacketToBeSent + bufferPointerPosition), &padding,
				fieldSize);

//		printf("Buffer packet to be sent = %s",
//				(char *) bufferForPacketToBeSent);

		bufferPointerPosition += fieldSize;
		fieldSize = 2;
		serverIDInNetworkByteOrder = htons(
				nodeDetailsToBeSentToNeighbors[i].otherServerID);
		memcpy((bufferForPacketToBeSent + bufferPointerPosition),
				&serverIDInNetworkByteOrder, fieldSize);

//		printf("Buffer packet to be sent = %s",
//				(char *) bufferForPacketToBeSent);

		bufferPointerPosition += fieldSize;
		fieldSize = 2;
		linkCostInNetworkByteOrder = htons(
				nodeDetailsToBeSentToNeighbors[i].otherServerCost);
		memcpy((bufferForPacketToBeSent + bufferPointerPosition),
				&linkCostInNetworkByteOrder, fieldSize);

//		printf("Buffer packet to be sent = %s",
//				(char *) bufferForPacketToBeSent);
	}

	return bufferForPacketToBeSent;
}
/**
 * fnFillPacketStruct function
 *
 * Fills the struct of the packet that is to be sent to all neighbors
 */
void fnFillPacketStruct() {
	/*
	 *
	 * Declaration of variables Start
	 */
	int i;
	/*
	 *
	 * Declaration of variables End
	 */

	routingPacketToBeSentToNeighbors.myServerIP =
			initialRoutingPacketFromTopologyFile.myServerIP;
	printf("%d\n", routingPacketToBeSentToNeighbors.myServerIP);
	printf("%d\n", initialRoutingPacketFromTopologyFile.myServerIP);
	routingPacketToBeSentToNeighbors.myServerPort =
			initialRoutingPacketFromTopologyFile.myServerPort;
	printf("%d\n", routingPacketToBeSentToNeighbors.myServerPort);

//	char s;
//
//	scanf("%s",&s);

	routingPacketToBeSentToNeighbors.numberOfUpdateFields =
			initialRoutingPacketFromTopologyFile.numberOfUpdateFields;
	printf("%d\n", routingPacketToBeSentToNeighbors.numberOfUpdateFields);
	for (i = 1; i <= numberOfServers; i++) {
		routingPacketToBeSentToNeighbors.otherServerUpdateFields[i] =
				initialRoutingPacketFromTopologyFile.otherServerUpdateFields[i];
		printf("\n%d\n", i);
		printf("%d\t%d\t%d\t%d\n",
				routingPacketToBeSentToNeighbors.otherServerUpdateFields[i].otherServerIP,
				routingPacketToBeSentToNeighbors.otherServerUpdateFields[i].otherServerID,
				routingPacketToBeSentToNeighbors.otherServerUpdateFields[i].otherServerCost,
				routingPacketToBeSentToNeighbors.otherServerUpdateFields[i].otherServerPort);
	}

	for (i = 1; i <= numberOfServers; i++) {
		routingPacketToBeSentToNeighbors.otherServerUpdateFields[i].otherServerCost =
				routingDetails[i].totalCost;
	}
}

/**
 * fnUpdateCostMatrixForNeighbor function
 *
 * Puts the cost values in costMatrix based on packet stored in global struct routingPacketReceivedFromNeighbor
 */
void fnUpdateCostMatrixForNeighbor() {
	/*
	 * Declaration of variables Start
	 */
	int i, j, index;
	/*
	 * Declaration of variables End
	 */

	for (i = 1; i <= numberOfServers; i++) {
		if (routingPacketReceivedFromNeighbor.myServerIP
				== nodeDetails[i].otherServerIP) {
			for (j = 1; j <= numberOfServers; j++) {
				index =
						routingPacketReceivedFromNeighbor.otherServerUpdateFields[j].otherServerID;
				costMatrix[i][index] =
						routingPacketReceivedFromNeighbor.otherServerUpdateFields[j].otherServerCost;
			}
			break;
		}
	}
}

/**
 * fnMinCostForThisNode function
 *
 * Calculates min cost and updates routing Table and routingPacketToBeSentToNeighbors struct
 */
uint16_t fnMinCostForThisNode(int neighborIndex) {
	/*
	 * Declaration of variables Start
	 */
	int i, j, index;
	/*
	 * Declaration of variables End
	 */
	return 0;
}

/**
 * fnIsNeighbor function
 *
 *@param
 * Calculates min cost and updates routing Table and routingPacketToBeSentToNeighbors struct
 */
int fnIsNeighbor(uint16_t serverID) {
	/*
	 * Declaration of variables Start
	 */
	int i, j, index;
	/*
	 * Declaration of variables End
	 */
	for (i = 0; i < numberOfNeighbors; i++) {
		if (neighbor[i] == serverID) {
			return 1;
		}
	}

	return 0;
}

/**
 * fnVerifyIfNeighbor function
 *
 *@param serverIP
 * Checks if the given IP corresponds to a neighbor
 */
int fnVerifyIfNeighbor(uint32_t serverIP) {
	/*
	 * Declaration of variables Start
	 */
	int i, isNeighbor;
	uint16_t checkServerID;
	/*
	 * Declaration of variables End
	 */

//	nodeDetails = malloc(sizeof(initialRoutingPacketFromTopologyFile.otherServerUpdateFields));
	for (i = 1; i <= numberOfServers; i++) {
		nodeDetails[i] =
				initialRoutingPacketFromTopologyFile.otherServerUpdateFields[i];
	}
	for (i = 1; i <= numberOfServers; i++) {
		if (nodeDetails[i].otherServerIP == serverIP) {
			checkServerID = nodeDetails[i].otherServerID;
			isNeighbor = fnIsNeighbor(checkServerID);

			if (isNeighbor == 1) {
				return checkServerID;
			}
		}
	}
	return 0;
}

//int fnVerifyIfNeighbor(uint32_t serverIP, uint16_t portNumber) {
//	/*
//	 * Declaration of variables Start
//	 */
//	int i, isNeighbor;
//	uint16_t checkServerID;
//	/*
//	 * Declaration of variables End
//	 */
//
////	nodeDetails = malloc(sizeof(initialRoutingPacketFromTopologyFile.otherServerUpdateFields));
//	for (i = 1; i <= numberOfServers; i++) {
//		nodeDetails[i] =
//				initialRoutingPacketFromTopologyFile.otherServerUpdateFields[i];
//	}
//	for (i = 1; i <= numberOfServers; i++) {
//		if (nodeDetails[i].otherServerIP == serverIP
//				&& nodeDetails[i].otherServerPort == portNumber) {
//			checkServerID = nodeDetails[i].otherServerID;
//			isNeighbor = fnIsNeighbor(checkServerID);
//
//			if (isNeighbor == 1) {
//				return checkServerID;
//			}
//		}
//	}
//	return 0;
//}

/**
 * fnBellmanFordAlgorithm function
 *
 * Calculates min cost and updates routing Table and routingPacketToBeSentToNeighbors struct
 */
void fnBellmanFordAlgorithm() {
	/*
	 * Declaration of variables Start
	 */
	uint16_t row, col, index;
	uint16_t minCost;
	uint16_t currentMinCost;
	int16_t nextHopServerID;
	int i;
	/*
	 * Declaration of variables End
	 */

	for (col = 1; col <= numberOfServers; col++) {
		minCost = INF;
		currentMinCost = INF;
		nextHopServerID = -1;
		if (col != myServerID) {
			for (row = 1; row <= numberOfServers; row++) {
				if (fnIsNeighbor(row) == 1) {

					if ( //costMatrix[myServerID][col] != INF
//							||
					costMatrix[row][col] != INF) {
						currentMinCost = costMatrix[myServerID][col]
								+ costMatrix[row][col];
						if (minCost > currentMinCost) {
							minCost = currentMinCost;
							nextHopServerID = row;
						}
					}
				}
			}
			routingDetails[col].fromServerID = nextHopServerID;
			routingDetails[col].totalCost = minCost;
		}

//		costMatrix[myServerID][col] = minCost;

	}

	printf("Printing Routing Table\n");
	for (i = 1; i <= numberOfServers; i++) {

		printf("%-15d%-15d%-15d\n", routingDetails[i].toServerID,
				routingDetails[i].fromServerID, routingDetails[i].totalCost);

	}
}
/**
 * fnStoreReceivedPacket function
 *
 * @param  inputCmd Command line input
 * @param  portNumber Process port number
 * @return int Server ID of the node from whom packet was received
 *
 * Puts the fields in the appropriate structure
 */
int fnStoreReceivedPacket(char bufferForReceivedPacket[1024]) {
	/*
	 * Declaration of variables Start
	 */
	int fieldSize, bufferPointerPosition;
//	char * bufferForReceivedPacket;
	uint16_t numberOfUpdateFieldsInNetworkByteOrder;
	uint16_t portNumberInNetworkByteOrder;
	uint16_t serverIDInNetworkByteOrder;
	uint16_t linkCostInNetworkByteOrder;
	uint32_t ipInNetworkByteOrder;
	uint16_t receivedPacketFromServerID;
	int i;
	/*
	 * Declaration of variables End
	 */

	/*Copy received packet into the struct*/

	size_t sizeOfPacket = sizeof(uint32_t) * 2
			+ (3 * sizeof(uint32_t) * numberOfServers);
//
//	bufferForReceivedPacket = malloc(sizeOfPacket);

//	printf("--------------------------Packed received of size  = %d\n",
//			sizeof(bufferForReceivedPacket));

//	cse4589_dump_packet(bufferForReceivedPacket,
//										sizeOfPacket);

	bufferPointerPosition = 0;
	fieldSize = 2;
	memcpy(&numberOfUpdateFieldsInNetworkByteOrder,
			(bufferForReceivedPacket + bufferPointerPosition), fieldSize);
//	numberOfUpdateFieldsInNetworkByteOrder =
//			routingPacketReceivedFromNeighbor.numberOfUpdateFields;
	routingPacketReceivedFromNeighbor.numberOfUpdateFields = ntohs(
			numberOfUpdateFieldsInNetworkByteOrder);

	bufferPointerPosition += fieldSize;
	memcpy(&portNumberInNetworkByteOrder,
			(bufferForReceivedPacket + bufferPointerPosition), fieldSize);
//	portNumberInNetworkByteOrder =
//			routingPacketReceivedFromNeighbor.myServerPort;
	routingPacketReceivedFromNeighbor.myServerPort = ntohs(
			portNumberInNetworkByteOrder);

	bufferPointerPosition += fieldSize;
	fieldSize = 4;
	memcpy(&routingPacketReceivedFromNeighbor.myServerIP,
			(bufferForReceivedPacket + bufferPointerPosition), fieldSize);
	ipInNetworkByteOrder = routingPacketReceivedFromNeighbor.myServerIP;
	routingPacketReceivedFromNeighbor.myServerIP = (ipInNetworkByteOrder);

	printf("Number of update fields received: %d\n",
			routingPacketReceivedFromNeighbor.numberOfUpdateFields);
	printf(
			"Server Port Number of the neighbor from whom packet is received: %d\n",
			routingPacketReceivedFromNeighbor.myServerPort);
	printf("Server IP of the neighbor from whom packet is received: %d\n",
			routingPacketReceivedFromNeighbor.myServerIP);

	receivedPacketFromServerID = fnVerifyIfNeighbor(
			routingPacketReceivedFromNeighbor.myServerIP);

//	receivedPacketFromServerID = fnVerifyIfNeighbor(
//			routingPacketReceivedFromNeighbor.myServerIP,
//			routingPacketReceivedFromNeighbor.myServerPort);

	if (receivedPacketFromServerID > 0) {

		for (i = 1; i <= routingPacketReceivedFromNeighbor.numberOfUpdateFields;
				i++) {
			bufferPointerPosition += fieldSize;
			fieldSize = 4;
			memcpy(&nodeDetailsReceivedFromNeighbor[i].otherServerIP,
					(bufferForReceivedPacket + bufferPointerPosition),
					fieldSize);
			ipInNetworkByteOrder =
					nodeDetailsReceivedFromNeighbor[i].otherServerIP;
			nodeDetailsReceivedFromNeighbor[i].otherServerIP =
					(ipInNetworkByteOrder);

			bufferPointerPosition += fieldSize;
			fieldSize = 2;
			memcpy(&nodeDetailsReceivedFromNeighbor[i].otherServerPort,
					(bufferForReceivedPacket + bufferPointerPosition),
					fieldSize);
			portNumberInNetworkByteOrder =
					nodeDetailsReceivedFromNeighbor[i].otherServerPort;
			nodeDetailsReceivedFromNeighbor[i].otherServerPort = ntohs(
					portNumberInNetworkByteOrder);

			/*Ignore padding fields*/
			bufferPointerPosition += fieldSize;
			fieldSize = 2;
			//do not copy anything

			bufferPointerPosition += fieldSize;
			fieldSize = 2;
			memcpy(&nodeDetailsReceivedFromNeighbor[i].otherServerID,
					(bufferForReceivedPacket + bufferPointerPosition),
					fieldSize);
			serverIDInNetworkByteOrder =
					nodeDetailsReceivedFromNeighbor[i].otherServerID;
			nodeDetailsReceivedFromNeighbor[i].otherServerID = ntohs(
					serverIDInNetworkByteOrder);

			bufferPointerPosition += fieldSize;
			fieldSize = 2;
			memcpy(&nodeDetailsReceivedFromNeighbor[i].otherServerCost,
					(bufferForReceivedPacket + bufferPointerPosition),
					fieldSize);
			linkCostInNetworkByteOrder =
					nodeDetailsReceivedFromNeighbor[i].otherServerCost;
			nodeDetailsReceivedFromNeighbor[i].otherServerCost = ntohs(
					linkCostInNetworkByteOrder);

			printf("Other Server[%d] IP address: %d\n", i,
					nodeDetailsReceivedFromNeighbor[i].otherServerIP);
			printf("Other Server[%d] Port Number: %d\n", i,
					nodeDetailsReceivedFromNeighbor[i].otherServerPort);
			printf("Other Server[%d] ID: %d\n", i,
					nodeDetailsReceivedFromNeighbor[i].otherServerID);
			printf("Other Server[%d] Link Cost: %d\n", i,
					nodeDetailsReceivedFromNeighbor[i].otherServerCost);

			routingPacketReceivedFromNeighbor.otherServerUpdateFields[i] =
					nodeDetailsReceivedFromNeighbor[i];
		}
		fnUpdateCostMatrixForNeighbor();
		fnBellmanFordAlgorithm();
	}
	return receivedPacketFromServerID;
}

/**
 * fnSendPacket function
 *
 *
 *Send Packet to the appropriate destination
 */

int fnSendPacket(uint16_t destinationServerID) {
	/*
	 * Declaration of variables Start
	 */
	int i;
	int flag = 0;
	char * bufferForPacketToBeSent;
	int numberOfBytesToBeSent;
	struct sockaddr_in server_addr;
//	uint16_t destinationServerID;
	uint16_t receiverPort;
	char receiverIP[16];
//	size_t sizeOfPacket;
	/*
	 * Declaration of variables End
	 //	 */
//	memset(&server_addr, 0, sizeof(server_addr));
//	server_addr.sin_family = AF_INET;
	size_t sizeOfPacket = sizeof(uint32_t) * 2
			+ (3 * sizeof(uint32_t) * numberOfServers);
	bufferForPacketToBeSent = (char *) malloc(sizeOfPacket);
//	receiverIP = (char *) malloc(16);

//	printf("bufferForPacketToBeSent = %d\n", sizeof(bufferForPacketToBeSent));

	bufferForPacketToBeSent = fnCreatePacket();

//	for (i = 0; i < numberOfNeighbors; i++) {

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

//		destinationServerID = neighbor[i];
	memcpy(receiverIP,
			routingPacketToBeSentToNeighbors.otherServerUpdateFields[destinationServerID].otherServerCharIP,
			16);
	receiverPort =
			routingPacketToBeSentToNeighbors.otherServerUpdateFields[destinationServerID].otherServerPort;

	server_addr.sin_addr.s_addr = inet_addr(receiverIP);
	printf("IP of receiver to whom data is being sent = %d\n",
			server_addr.sin_addr.s_addr);
	server_addr.sin_port = htons(receiverPort);

	if ((numberOfBytesToBeSent = sendto(udpSocket,
			(char*) bufferForPacketToBeSent, sizeOfPacket, 0,
			(struct sockaddr *) &server_addr, sizeof server_addr)) == -1) {
		perror("Packet sending error");
		return 0;
	}
	//						return 1;
	printf("Number of bytes sent = %d\n", numberOfBytesToBeSent);
	printf("Packet sent to neighbor[%d]\n", destinationServerID);
	printf("Packet sent to IP: %s\n", receiverIP);
	printf("Packet sent to port: %d\n", receiverPort);
//	}

	return 1;

}

/**
 * fnUpdate function
 *
 *
 *Displays Update command details
 */

int fnUpdate(uint16_t serverID1, uint16_t serverID2, uint16_t linkCost) {
	/*
	 * Declaration of variables Start
	 */
	int i;
	int flag = 0;
	/*
	 * Declaration of variables End
	 */
	if (serverID1 == myServerID) {
		for (i = 0; i < numberOfNeighbors; i++) {
			if (neighbor[i] == serverID2) {
				flag = 1;
				break;
			} else {
				//do nothing
			}
		}
		if (flag == 1) {
			costMatrix[myServerID][serverID2] = linkCost;
		} else {
			return 2;
		}

	} else if (serverID2 == myServerID) {
		for (i = 0; i < numberOfNeighbors; i++) {
			if (neighbor[i] == serverID1) {
				flag = 1;
				break;
			} else {
				//do nothing
			}
		}
		if (flag == 1) {
			costMatrix[myServerID][serverID1] = linkCost;
		} else {
			return 3;
		}

	} else {
		return 0;
	}

	fnBellmanFordAlgorithm();
	fnFillPacketStruct();

	return 1;
}

/**
 * fnDisplay function
 *
 *
 *Displays Display command details
 */

void fnDisplay() {
	/*
	 * Declaration of variables Start
	 */
	int i;
	/*
	 * Declaration of variables End
	 */
	for (i = 1; i <= numberOfServers; i++) {
		cse4589_print_and_log("%-15d%-15d%-15d\n", routingDetails[i].toServerID,
				routingDetails[i].fromServerID, routingDetails[i].totalCost);
	}

//	return 1;
}

/**
 * fnStep function
 *
 *
 *Send update packets to all neighbors in the network
 */

int fnStep() {
	/*
	 * Declaration of variables Start
	 */
	int i;
	char * bufferForPacketToBeSent;
	int result;
	/*
	 * Declaration of variables End
	 */

	size_t sizeOfPacket = sizeof(uint32_t) * 2
			+ (3 * sizeof(uint32_t) * numberOfServers);

	bufferForPacketToBeSent = malloc(sizeOfPacket);

	fnBellmanFordAlgorithm();
	fnFillPacketStruct();
//	fnDisplay();
	for (i = 0; i < numberOfNeighbors; i++) {
		result = fnSendPacket(neighbor[i]);
	}

	return result;
}

/**
 * fnPackets function
 *
 *
 *Displays Packets command details
 */

void fnPackets() {

	cse4589_print_and_log("%d\n", packetCounter);
	packetCounter = 0;

//	return 1;
}

/**
 * fnDisable function
 *
 *
 *Displays Disable command details
 */

int fnDisable(uint16_t serverID) {
	/*
	 * Declaration of variables Start
	 */
	int i;
	int isNeighbor;
	int flag = 0;
	/*
	 * Declaration of variables End
	 */
	isNeighbor = fnIsNeighbor(serverID);

	if (isNeighbor == 1) {
		for (i = 0; i < numberOfNeighbors; i++) {
			if (neighbor[i] == serverID) {
				if (i != numberOfNeighbors - 1) {
					flag = 1;
				}
			}

			if (flag == 1) {
				if (i != numberOfNeighbors - 1) {
					neighbor[i] = neighbor[i + 1];
				}

			}
		}

		numberOfNeighbors--;
		costMatrix[myServerID][serverID] = INF;
		fnBellmanFordAlgorithm();
		fnFillPacketStruct();

	} else {
		return 0;
	}
	return 1;
}

/**
 * fnCrash function
 *
 *
 *Puts the program in an infinite loop which will not be able to accept/send any update packets from/to the neighbors
 */

void fnCrash() {

	while (1) {
		//do nothing
	}

}

/**
 * fnDump function
 *
 *
 *Displays Dump command details
 */

int fnDump() {
	/*
	 * Declaration of variables Start
	 */
	int numberOfBytesWritten;
	char * bufferPacketToPrint;
	size_t sizeOfPacket;
	/*
	 * Declaration of variables End
	 */
	fnFillPacketStruct();

	sizeOfPacket = sizeof(uint32_t) * 2
			+ (3 * sizeof(uint32_t) * numberOfServers);

	bufferPacketToPrint = (char *) malloc(sizeOfPacket);

	bufferPacketToPrint = fnCreatePacket();

//	printf("Size of packet to be dumped = %d\n", sizeof(bufferPacketToPrint));
	numberOfBytesWritten = cse4589_dump_packet(bufferPacketToPrint,
			sizeOfPacket);

	printf("Number of Bytes written = %d\n", numberOfBytesWritten);

	if (numberOfBytesWritten > 0) {
		return 1;
	} else {
		return 0;
	}
}

/**
 * fnAcademicIntegrity function
 *
 *
 *Displays Academic_Integrity command details
 */

int fnAcademicIntegrity() {
	cse4589_print_and_log(
			"I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
	return 1;
}

/**
 * fnErrorMsg function
 *
 *Gives generic error message
 */

void fnErrorMsg() {
	printf("Invalid command or arguments provided\n");
}

/**
 * read_line function
 *
 *@param c contains the input data
 *Used to read data received from console
 */

char *fnReadLine(char *c) {
	int len = strlen(c);
	if (len > 0 && c[len - 1] == '\n')
		c[len - 1] = '\0';
	return c;
}

/**
 * fnReadUserInput function
 *
 *
 *Get commands from console
 */

char *fnReadUserInput() {
	char input[1000];
	char *inputCommand;

	inputCommand = (char *) malloc(50);

//	printf("[PA1]$ ");
	inputCommand = fnReadLine(fgets(input, sizeof(input), stdin));
	return inputCommand; //fnSelectCommandAction(Selc1,4556);
}

/**
 * fnSelectCommandAction function
 *
 * @param  inputCmd Command line input
 * @param  portNumber Process port number
 * @return char* Input Typed by User
 *
 *
 */
void fnSelectCommandAction(char* inputCmd) {
	int count = 0;
	int i;
	int flag = 1;
	int functionExecuteSuccess = 0;
	char *token;
	char outputCmd1[50];
	char outputCmd2[50];
	char outputCmd3[50];
	char outputCmd4[50];
	char errorMessage[1024];

	token = (char *) malloc(50);
//	strcpy(errorMessage, (char *) malloc(255);

	token = strtok(inputCmd, " \n");

	while (token != NULL) {

		if (count < 4) {

			if (count == 0) {
				strcpy(outputCmd1, token);
			}

			else if (count == 1) {
				strcpy(outputCmd2, token);

			}

			else if (count == 2) {
				strcpy(outputCmd3, token);

			}

			else if (count == 3) {
				strcpy(outputCmd4, token);

			}
		} else
			strcpy(errorMessage, "Invalid command or invalid number of arguments");
		cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);

		token = strtok(NULL, " \n");

		count += 1;
	}

	if (count == 1) {
		if (strcasecmp(outputCmd1, "STEP") == 0) {
			functionExecuteSuccess = fnStep();
			if (functionExecuteSuccess == 1) {
				cse4589_print_and_log("%s:SUCCESS\n", outputCmd1);
			} else {
				strcpy(errorMessage, "Command execution failed to send routing updates to some or all neighbors");
				cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
			}
		} else if (strcasecmp(outputCmd1, "PACKETS") == 0) {
			cse4589_print_and_log("%s:SUCCESS\n", outputCmd1);

			fnPackets();

		} else if (strcasecmp(outputCmd1, "DISPLAY") == 0) {
			cse4589_print_and_log("%s:SUCCESS\n", outputCmd1);

			fnDisplay();

		} else if (strcasecmp(outputCmd1, "CRASH") == 0) {
			fnCrash();
		} else if (strcasecmp(outputCmd1, "DUMP") == 0) {
			functionExecuteSuccess = fnDump();
			if (functionExecuteSuccess == 1) {
				cse4589_print_and_log("%s:SUCCESS\n", outputCmd1);
			} else {
				strcpy(errorMessage, "Command execution failed to dump the packet");
				cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
			}
		} else if (strcasecmp(outputCmd1, "ACADEMIC_INTEGRITY") == 0) {
			cse4589_print_and_log("%s:SUCCESS\n", outputCmd1);

			fnAcademicIntegrity();
		} else {
			fnErrorMsg();
			strcpy(errorMessage, "Command execution failed because it is not a valid command");
			cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
		}
	}

	else if (count == 2) {
		if (strcasecmp(outputCmd1, "DISABLE") == 0) {
			for (i = 0; i < strlen(outputCmd2); i++) {
				if (isdigit(outputCmd2[i])) {
				}

				else {
					flag = 0;
					break;
				}
			}
			if (flag == 1) {

				functionExecuteSuccess = fnDisable(atoi(outputCmd2));
				if (functionExecuteSuccess == 1) {
					cse4589_print_and_log("%s:SUCCESS\n", outputCmd1);
				} else {
					strcpy(errorMessage, "Command execution failed to disable the link in the network");
					cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
				}
			} else {
				strcpy(errorMessage, "Command execution failed Input command 2 can only be numeric");
				cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
			}
		} else {
			fnErrorMsg();
			strcpy(errorMessage, "Command execution failed because it is not a valid command");
			cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
		}
	}

	else if (count == 4) {
		if (strcasecmp(outputCmd1, "UPDATE") == 0) {
			for (i = 0; i < strlen(outputCmd2); i++) {
				if (isdigit(outputCmd2[i])) {
				}

				else {
					flag = 0;
					break;
				}
			}
			if (flag == 1) {
				for (i = 0; i < strlen(outputCmd3); i++) {
					if (isdigit(outputCmd3[i])) {
					}

					else {
						flag = 0;
						break;
					}
				}

				if (flag == 1) {
					for (i = 0; i < strlen(outputCmd4); i++) {
						if (isdigit(outputCmd4[i])) {
						}

						else {
							flag = 0;
							break;
						}
					}
					if (flag == 1) {
						functionExecuteSuccess = fnUpdate(atoi(outputCmd2),
								atoi(outputCmd3), atoi(outputCmd4));
						if (functionExecuteSuccess == 1) {
							cse4589_print_and_log("%s:SUCCESS\n", outputCmd1);
						} else if (functionExecuteSuccess == 2) {
							strcpy(errorMessage, "Command execution failed ServerID2 is not a neighbor for the current node");
							cse4589_print_and_log("%s:%s\n", outputCmd1,
									errorMessage);
						} else if (functionExecuteSuccess == 3) {
							strcpy(errorMessage, "Command execution failed ServerID1 is not a neighbor for the current node");
							cse4589_print_and_log("%s:%s\n", outputCmd1,
									errorMessage);
						} else {
							strcpy(errorMessage, "Command execution failed to update the link cost");
							cse4589_print_and_log("%s:%s\n", outputCmd1,
									errorMessage);
						}
					} else {
						strcpy(errorMessage, "Command execution failed Input command 4 can only be numeric");
						cse4589_print_and_log("%s:%s\n", outputCmd1,
								errorMessage);
					}
				} else {
					strcpy(errorMessage, "Command execution failed Input command 3 can only be numeric");
					cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
				}
			} else {
				strcpy(errorMessage, "Command execution failed Input command 2 can only be numeric");
				cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
			}
		} else {
			fnErrorMsg();
			strcpy(errorMessage, "Command execution failed because it is not a valid command");
			cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
		}
	} else {
		fnErrorMsg();
		strcpy(errorMessage, "Command execution failed because it is not a valid command");
		cse4589_print_and_log("%s:%s\n", outputCmd1, errorMessage);
	}

	fflush(stdout);
}

/**
 * fnUDPListen function
 *
 * @param  inputCmd Command line input
 * @param  portNumber Process port number
 * @return char* Input Typed by User
 *
 * Start Listening Port and receive input commands and other packets
 */
void fnUDPListen() {
	/*
	 * Declaration of variables Start
	 */
	int fdMax, sockIndex;
	struct addrinfo server_addr, *servinfo, *p;
	int getaddrinfoResult, selectResult;
	fd_set read_fds;
	FD_ZERO(&master);               // clear the master and temp sets
	FD_ZERO(&read_fds);
	int receivedPacketSize, numberOfReceivedBytes, numberOfBytesToBeSent;
	socklen_t theirAddrLength;
	char bufferForReceivedPacket[1024];
	char * bufferForPacketToBeSent;
	struct sockaddr_storage theirAddr;
	struct in_addr addr;
	struct timeval tv;
	int i;
	int packetReceived;
	/*
	 * Declaration of variables End
	 */

	servinfo = malloc(100);
	p = malloc(100);

	size_t sizeOfPacket = sizeof(uint32_t) * 2
			+ (3 * sizeof(uint32_t) * numberOfServers);

//	bufferForReceivedPacket = (char *) malloc(sizeOfPacket);
	bufferForPacketToBeSent = (char *) malloc(sizeOfPacket);

//Ref: Beej Guide
	memset(&server_addr, 0, sizeof server_addr);
	server_addr.ai_family = AF_INET;
	server_addr.ai_socktype = SOCK_DGRAM;
	server_addr.ai_flags = AI_PASSIVE;

//	strcpy(myPort,"4567");

	if ((getaddrinfoResult = getaddrinfo(NULL, myPort, &server_addr, &servinfo))
			!= 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoResult));
//		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((udpSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("listener:socket\n");
			continue;
		}

		if (bind(udpSocket, p->ai_addr, p->ai_addrlen) == -1) {
			close(udpSocket);
			perror("listener:bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
//		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener waiting to recvfrom\n");

	FD_SET(udpSocket, &master);
	FD_SET(0, &master); //Add STDIN
	fdMax = udpSocket;

	tv.tv_sec = timeOut;
	tv.tv_usec = 0;
//	optval = 1;
//	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

	while (1) {
		read_fds = master; // create a copy
		selectResult = select(fdMax + 1, &read_fds, NULL, NULL, &tv);
		if (selectResult == -1) {
//			perror("Select error");
			exit(4);
		} else if (selectResult > 0) {
			for (sockIndex = 0; sockIndex <= fdMax; sockIndex++) {
//				printf("Socket received\n");
				if (FD_ISSET(sockIndex, &read_fds)) {
//					printf("FD set\n");
					if (sockIndex == 0) {
						char * userSelection = fnReadUserInput(); //read_line(fgets(GetUserInput,sizeof(GetUserInput),stdin));
//						userSelection =
						fnSelectCommandAction(userSelection);
					}
					if (sockIndex == udpSocket) {
//						printf("Socket received from node in the network\n");
//						receivedPacketSize = sizeof(uint32_t) * 2
//								+ (3 * sizeof(uint32_t) * numberOfServers);

//						bufferForReceivedPacket = (char *) malloc(
//								sizeOfPacket);

						printf("sizeOfPacket = %d\n", sizeOfPacket);

						printf("Original size of buffer = %d\n",
								sizeof(bufferForReceivedPacket));

//						printf(
//								"All set to receive incoming packet in buffer\n");

//						numberOfReceivedBytes = recvfrom(sockIndex,
//														bufferForReceivedPacket, receivedPacketSize, 0,
//														(struct sockaddr *) &theirAddr,
//														&theirAddrLength);

						numberOfReceivedBytes = recvfrom(sockIndex,
								bufferForReceivedPacket,
								sizeof(bufferForReceivedPacket), 0,
								NULL,
								NULL);

						if (numberOfReceivedBytes == -1) {
							perror("Error on recvfrom");
							exit(1);
						}

						printf("Number of bytes received = %d\n",
								numberOfReceivedBytes);

						printf(
								"Packet received from network\nNow verifying if received from neighbor\n");

						printf("Packet size being sent = %d\n",
								sizeof(bufferForReceivedPacket));
//						if (fnVerifyIfNeighbor(bufferForReceivedPacket)) {
//						cse4589_dump_packet(bufferForReceivedPacket,
//									sizeOfPacket);
						packetReceived = fnStoreReceivedPacket(
								bufferForReceivedPacket);
						//add logic to update routingPacketToBeSentToNeighbors struct and routingDetails struct
						if (packetReceived > 0) {
							cse4589_print_and_log(
									"RECEIVED A MESSAGE FROM SERVER %d\n",
									packetReceived);
							fnFillPacketStruct();
							packetCounter++;
						}
//						}
					}
				}
			}
		} else if (selectResult == 0) {
			//when timeout occurs
			for (i = 0; i < numberOfNeighbors; i++) {
				printf("Check to send updates to all neighbors\n");
				if (routingDetails[neighbor[i]].counter != 127) {
					routingDetails[neighbor[i]].counter++;
				}

				if (routingDetails[neighbor[i]].counter == 3) {
					routingDetails[neighbor[i]].counter = 127;
					//						routingDetails[neighbor[i]].totalCost = INF;
					costMatrix[myServerID][i] = INF;
				}

				fnBellmanFordAlgorithm();
				fnFillPacketStruct();
				bufferForPacketToBeSent = fnCreatePacket();

//					if ((numberOfBytesToBeSent = sendto(udpSocket,
//							bufferForPacketToBeSent,
//							sizeof(bufferForPacketToBeSent), 0,
//							(struct sockaddr *) &server_addr,
//							sizeof server_addr)) == -1) {
//						perror("Packet sending error");
//						return 0;
//					}
//					return 1;
				fnSendPacket(neighbor[i]);
			}

			tv.tv_sec = timeOut;
			tv.tv_usec = 0;
		}
	}
//	return 0;
}

/**
 * fnInitialize function
 *
 *
 */
void fnInitialize() {
	/*
	 * Declaration of variables Start
	 */
	int i, j;
	char c;
	/*
	 * Declaration of variables End
	 */

//Initializing Cost Matrix
	printf("Number of servers = %d\n", numberOfServers);
	printf("Number of neighbors = %d\n", numberOfNeighbors);
	for (i = 1; i <= (numberOfServers); i++) {
		printf("Number of servers = %d\n", numberOfServers);
//		scanf("%c", &c);
		for (j = 1; j <= (numberOfServers); j++) {
			printf("Number of servers = %d\n", numberOfServers);
//			scanf("%c", &c);
			if (i == j) {
				costMatrix[i][j] = 0;
				printf("Cost Matrix [%d][%d] = %d\n", i, j, costMatrix[i][j]);
			} else {
				costMatrix[i][j] = INF;
				printf("Cost Matrix [%d][%d] = %d\n", i, j, costMatrix[i][j]);
			}
		}
	}

//Initialize Routing Table
//	routingDetails = malloc(8 * (numberOfServers + 1));
	printf("Initializing Routing Table\n");
	for (i = 1; i <= numberOfServers; i++) {
		if (i == myServerID) {
			routingDetails[i].toServerID = i;
//			printf("To Server ID[%d]: %d\n", i, routingDetails[i].toServerID);
			routingDetails[i].fromServerID = i;
//			printf("From Server ID[%d]: %d\n", i,
//					routingDetails[i].fromServerID);
			routingDetails[i].totalCost = 0;
//			printf("Total Cost ID[%d]: %d\n", i, routingDetails[i].totalCost);
			routingDetails[i].counter = 127;
//			printf("Counter ID[%d]: %d\n", i, routingDetails[i].counter);

		} else {
			routingDetails[i].toServerID = i;
//			printf("To Server ID[%d]: %d\n", i, routingDetails[i].toServerID);
			routingDetails[i].fromServerID = -1;
//			printf("From Server ID[%d]: %d\n", i,
//					routingDetails[i].fromServerID);
			routingDetails[i].totalCost = INF;
//			printf("Total Cost ID[%d]: %d\n", i, routingDetails[i].totalCost);
			routingDetails[i].counter = 127;
//			printf("Counter ID[%d]: %d\n", i, routingDetails[i].counter);
		}
	}

	printf("Printing Routing Table\n");
	for (i = 1; i <= numberOfServers; i++) {
		printf("%-15d%-15d%-15d\n", routingDetails[i].toServerID,
				routingDetails[i].fromServerID, routingDetails[i].totalCost);
	}

	printf("Initializing Routing Table complete\n");

}

/**
 * fnParseTopologyFile function
 *
 * @param  topologyFile File which defines the network topology
 *
 */
void fnParseTopologyFile(FILE* topologyFile) {
	/*
	 * Declaration of variables Start
	 */
	int i, j;
	char *currentLineInTopologyFile = NULL;
	char * split;
	size_t currentLineLength = 0;
	uint16_t countOfSplits = 0;
	uint16_t countOfNeighbors = 0;
	uint16_t numberOfLinesInTopologyFile = 0;
	uint16_t index;
	uint16_t neighborServerID;
	struct sockaddr_in server_ip;
	int newNeighborServerID;
	/*
	 * Declaration of variables End
	 */

//	currentLineInTopologyFile = char(*)malloc()
	while (getline(&currentLineInTopologyFile, &currentLineLength, topologyFile)
			!= -1) {
		printf("Current Line is: %s", currentLineInTopologyFile);

		++numberOfLinesInTopologyFile;

		if (numberOfLinesInTopologyFile == 1) {
			numberOfServers = atoi(currentLineInTopologyFile);
			initialRoutingPacketFromTopologyFile.numberOfUpdateFields =
					numberOfServers;

//			nodeDetails = (struct updateFields *) malloc(
//					(numberOfServers + 1) * 26);
//			nodeDetailsReceivedFromNeighbor = (struct updateFields *) malloc(
//					(numberOfServers + 1) * 26);
//			nodeDetailsToBeSentToNeighbors = (struct updateFields *) malloc(
//					(numberOfServers + 1) * 26);
//
//			initialRoutingPacketFromTopologyFile.otherServerUpdateFields =
//					(struct updateFields *) malloc((numberOfServers + 1) * 26);
//
//			routingPacketReceivedFromNeighbor.otherServerUpdateFields =
//					(struct updateFields *) malloc((numberOfServers + 1) * 26);
//
//			routingPacketToBeSentToNeighbors.otherServerUpdateFields =
//					(struct updateFields *) malloc((numberOfServers + 1) * 26);

			printf("Number of Servers = %d\n", numberOfServers);

		} else if (numberOfLinesInTopologyFile == 2) {
			numberOfNeighbors = atoi(currentLineInTopologyFile);
//			routingDetails = (struct routingTable *) malloc(
//					numberOfNeighbors + 1);
			printf("Number of Neighbors = %d\n", numberOfNeighbors);
//			neighbor = malloc(2 * numberOfNeighbors);

			printf("Begin Cost matrix Initialization\n");

			fnInitialize();
//			fnBellmanFordAlgorithm();
			printf("Cost matrix initialization complete\n");

		} else {
			if (numberOfLinesInTopologyFile <= (numberOfServers + 2)) {

				countOfSplits = 0;
				split = strtok(currentLineInTopologyFile, " ");

				while (split != NULL) {
					++countOfSplits;

					printf("Split value = %s\n", split);

					if (countOfSplits == 1) {
						index = atoi(split);
						nodeDetails[index].otherServerID = index;
					} else if (countOfSplits == 2) {
//						nodeDetails[index].otherServerCharIP = (char *) malloc(
//								16);
//						nodeDetailsReceivedFromNeighbor[index].otherServerCharIP =
//								(char *) malloc(16);
//						nodeDetailsToBeSentToNeighbors[index].otherServerCharIP =
//								(char *) malloc(16);
//
//						initialRoutingPacketFromTopologyFile.otherServerUpdateFields[index].otherServerCharIP =
//								(char *) malloc(16);
//						routingPacketReceivedFromNeighbor.otherServerUpdateFields[index].otherServerCharIP =
//								(char *) malloc(16);
//						routingPacketToBeSentToNeighbors.otherServerUpdateFields[index].otherServerCharIP =
//								(char *) malloc(16);

						memcpy(nodeDetails[index].otherServerCharIP, split, 16);
						if (inet_aton(split, &server_ip.sin_addr) != 0) {
							nodeDetails[index].otherServerIP =
									server_ip.sin_addr.s_addr;
						}
					} else if (countOfSplits == 3) {
						nodeDetails[index].otherServerPort = atoi(split);
						nodeDetails[index].otherServerCost = INF;
					}

					split = strtok(NULL, " ");
				}

				printf("Server ID = %d\n", nodeDetails[index].otherServerID);
				printf("Server IP = %d\n", nodeDetails[index].otherServerIP);
				printf("Server Port = %d\n",
						nodeDetails[index].otherServerPort);
			} else if (numberOfLinesInTopologyFile > (numberOfServers + 2)) {
				countOfSplits = 0;

				split = strtok(currentLineInTopologyFile, " ");
				while (split != NULL) {

					++countOfSplits;

					if (countOfSplits == 1) {
						myServerID = atoi(split);
						printf(
								"--------------------------------------------We are here----------------------------------\n");
						printf("My server ID = %d\n", myServerID);

						routingDetails[myServerID].fromServerID = myServerID;
						routingDetails[myServerID].totalCost = 0;

						printf("Printing Routing Table\n");
						for (i = 1; i <= numberOfServers; i++) {

							printf("%-15d%-15d%-15d\n",
									routingDetails[i].toServerID,
									routingDetails[i].fromServerID,
									routingDetails[i].totalCost);

						}

						nodeDetails[myServerID].otherServerCost = 0;

						initialRoutingPacketFromTopologyFile.myServerIP =
								nodeDetails[myServerID].otherServerIP;
						initialRoutingPacketFromTopologyFile.myServerPort =
								nodeDetails[myServerID].otherServerPort;

//						myPort = (char *) malloc(5);

						sprintf(myPort, "%d",
								nodeDetails[myServerID].otherServerPort);
						printf("Current Server ID = %d\n", myServerID);
					} else if (countOfSplits == 2) {
//						newNeighborServerID = atoi(split);
//
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].fromServerID = atoi(
//								split);
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].toServerID = atoi(
//								split);
						neighborServerID = atoi(split);
						neighbor[countOfNeighbors] = atoi(split);
						++countOfNeighbors;
					} else if (countOfSplits == 3) {
						costMatrix[myServerID][neighborServerID] = atoi(split);
						for (i = 1; i <= numberOfServers; i++) {
							for (j = 1; j <= numberOfServers; j++) {
								printf("%d\t", costMatrix[i][j]);
							}
							printf("\n");
						}
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].totalCost = atoi(
//								split);
//						routingDetails[neighborServerID].totalCost = atoi(
//								split);
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].counter = 0;
						nodeDetails[neighborServerID].otherServerCost = atoi(
								split);
					}

					split = strtok(NULL, " ");
				}

				for (i = 1; i <= numberOfServers; i++) {
					initialRoutingPacketFromTopologyFile.otherServerUpdateFields[i] =
							nodeDetails[i];
				}

//				printf("From Server ID = %d\n",
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].fromServerID);
//				printf("To Server ID = %d\n",
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].toServerID);
//				printf("Cost for the path = %d\n",
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].totalCost);
//				printf("Counter for this server = %d\n",
//						routingDetails[numberOfLinesInTopologyFile
//								- numberOfServers - 2 - 1].counter);

			}

//			for (i = 1; i <= numberOfServers; i++) {
//				routingDetails[i].toServerID = i;
//				printf("To Server ID[%d]: %d\n", i,
//						routingDetails[i].toServerID);
//				routingDetails[i].fromServerID = -1;
//				printf("From Server ID[%d]: %d\n", i,
//						routingDetails[i].fromServerID);
//				routingDetails[i].totalCost = 65535;
//				printf("Total Cost ID[%d]: %d\n", i,
//						routingDetails[i].totalCost);
//				routingDetails[i].counter = 127;
//				printf("Counter ID[%d]: %d\n", i, routingDetails[i].counter);

//			}

		}

	}

	printf(
			"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Start Bellman Ford Algorithm----------------------------\n");
	fnBellmanFordAlgorithm();
	//				fnUpdateCostMatrixFromTopologyFile();
	fnFillPacketStruct();
}

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv) {
	/*
	 * Declaration of variables Start
	 */
	uint8_t numberOfLinesInTopologyFile = 0;
	uint8_t currentServerID;
	struct sockaddr_in server_ip;
	FILE *topologyFile;
	/*
	 * Declaration of variables End
	 */
	/*Init. Logger*/
	cse4589_init_log();

	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));

	/*Start Here*/
	if (argc == 5) {
		if (strcmp(argv[1], "-i") == 0) {
			printf("This is timer\n");
			timeOut = atoi(argv[2]);
			printf("TimeOut value = %d\n", timeOut);

			if (strcmp(argv[3], "-t") == 0) {
				printf("This is next param topology file \n");

				topologyFile = fopen(argv[4], "r");
			} else {
				printf("Invalid input parameters\n");
			}
		} else if (strcmp(argv[1], "-t") == 0) {
			printf("This is topology file\n");

			topologyFile = fopen(argv[2], "r");

			if (strcmp(argv[3], "-i") == 0) {
				printf("This is next param timer\n");
				timeOut = atoi(argv[4]);
				printf("TimeOut value = %d\n", timeOut);
			} else {
				printf("Invalid input parameters\n");
				exit(1);
			}
		} else {
			printf("Invalid input parameters\n");
			exit(1);
		}
	} else {
		printf("Invalid number of arguments\n");
		exit(1);
	}

	if (topologyFile != NULL) {
		fnParseTopologyFile(topologyFile);
	} else {
		printf("Error: File %s not found\n", argv[4]);
		exit(1);
	}

	fnUDPListen(myPort);

//	free(nodeDetails);
	return 0;
}
