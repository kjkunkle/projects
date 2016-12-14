#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>

#include <string>
#include <sstream>
#include <iostream>

typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

#define MAX_REQUEST_SIZE 10000000
#define MAX_CONCURRENCY_LIMIT 400

struct CONN_STAT {
	unsigned char data[4096];
	int size;		//0 if unknown yet
	int nRecv;
	int nSent;
	int seq_num;
};

int nConns;
struct pollfd peers[MAX_CONCURRENCY_LIMIT+1];
struct CONN_STAT connStat[MAX_CONCURRENCY_LIMIT+1];
struct CONN_STAT ooo_buffer[MAX_CONCURRENCY_LIMIT+1];
int global_seq_num;

void Error(const char * format, ...) {
	char msg[4096];
	va_list argptr;
	va_start(argptr, format);
	vsprintf(msg, format, argptr);
	va_end(argptr);
	fprintf(stderr, "Error: %s\n", msg);
	exit(-1);
}

void Log(const char * format, ...) {
	char msg[2048];
	va_list argptr;
	va_start(argptr, format);
	vsprintf(msg, format, argptr);
	va_end(argptr);
	fprintf(stderr, "%s\n", msg);
}

void CheckData(BYTE * buf, int size) {
	for (int i=0; i<size; i++) if (buf[i] != 'A' + i % 26) {
		Error("Received wrong data.");
	}
}

//pass in header buffer and all data
//send in increments of 4096

int Send_NonBlocking(int sockFD, const BYTE * data, int len, struct CONN_STAT * pStat, struct pollfd * pPeer) {
	
	int nSent = 0;
	while (nSent < len) {
		int n = send(sockFD, data + nSent, len - nSent, 0);
		if (n >= 0) {
			nSent += n;
			pStat->nSent += n;
			printf("Sent %d bytes to LP.\n", n);
		} else if (n < 0 && (errno == ECONNRESET || errno == EPIPE)) {
			Log("Connection closed.");
			close(sockFD);
			return -1;
		} else if (n < 0 && (errno == EWOULDBLOCK)) {
			pPeer->events |= POLLWRNORM;
			return 0;
		} else {
			Error("Unexpected send error %d: %s", errno, strerror(errno));
		}
	}
	pPeer->events &= ~POLLWRNORM;
	return 0;
}

int Recv_NonBlocking(int sockFD, BYTE * data, int len, struct CONN_STAT * pStat, struct pollfd * pPeer) {
	while (pStat->nRecv < len) {
		int n = recv(sockFD, data + pStat->nRecv, len - pStat->nRecv, 0);
		if (n > 0) {
			pStat->nRecv += n;
		} else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
			Log("Connection closed.");
			close(sockFD);
			return -1;
		} else if (n < 0 && (errno == EWOULDBLOCK)) {
			return 0;
		} else {
			Error("Unexpected recv error %d: %s.", errno, strerror(errno));
		}
	}
	
	return 0;
}

void SetNonBlockIO(int fd) {
	int val = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0) {
		Error("Cannot set nonblocking I/O.");
	}
}

void RemoveConnection(int i) {
	close(peers[i].fd);	
	if (i < nConns) {	
		memmove(peers + i, peers + i + 1, (nConns-i) * sizeof(struct pollfd));
		memmove(connStat + i, connStat + i + 1, (nConns-i) * sizeof(struct CONN_STAT));
	}
	nConns--;
}

int Send_Blocking(int sockFD, const BYTE * data, int len) {
	int nSent = 0;
	while (nSent < len) {
		int n = send(sockFD, data + nSent, len - nSent, 0);
		if (n >= 0) {
			Log("Sent message to server.");
			nSent += n;
		} else if (n < 0 && (errno == ECONNRESET || errno == EPIPE)) {
			Log("Connection closed.");
			close(sockFD);
			return -1;
		} else {
			Error("Unexpected error %d: %s.", errno, strerror(errno));
		}
	}
	return nSent;
}

int Recv_Blocking(int sockFD, BYTE * data, int len) {
	int nRecv = 0;
//	while (nRecv < len) {
	int n = recv(sockFD, data + nRecv, len - nRecv, 0);
	if (n > 0) {
		Log("Received message from server.");
		nRecv += n;
	} else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
		Log("Connection closed.");
		close(sockFD);
		return -1;
	} else {
		Error("Unexpected error %d: %s.", errno, strerror(errno));
	}
	return nRecv;
}

void DoServer(int svrPort, int maxConcurrency) {
	BYTE * buf = (BYTE *)malloc(MAX_REQUEST_SIZE);	
	bzero(buf, MAX_REQUEST_SIZE);

	int listenFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFD < 0) {
		Error("Cannot create listening socket as server.");
	}
	SetNonBlockIO(listenFD);

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((unsigned short) svrPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int cli_fd[1000];
	bool client_status[1000]; //-1/ 0 = not connected, 1 = connected
	//bzero(client_status, maxConcurrency + 1);

	struct sockaddr_in saddr_in;
	memset(&saddr_in, 0, sizeof(saddr_in));

	//prepare data
	for (int i=0; i<MAX_REQUEST_SIZE; i++) {
		buf[i] = 'A' + i % 26;
	}

	int optval = 1;
	int r = setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (r != 0) {
		Error("Cannot enable SO_REUSEADDR option.");
	}
	
	if (bind(listenFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
		Error("Cannot bind to port %d.", svrPort);
	}
	
	if (listen(listenFD, 16) != 0) {
		Error("Cannot listen to port %d.", svrPort);
	}
	
	nConns = 0;	
	memset(peers, 0, sizeof(peers));	
	peers[0].fd = listenFD;
	peers[0].events = POLLRDNORM;	
	memset(connStat, 0, sizeof(connStat));
	
	int connID = 0;
	while (1) {	//the main loop
		
		int nReady = poll(peers, nConns + 1, -1);
		
		if (nReady < 0) {
			Error("Invalid poll() return value.");
		}			
			
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);	
		
		if ((peers[0].revents & POLLRDNORM) && (nConns < maxConcurrency)) {					
			int fd = accept(listenFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
			if (fd != -1) {
				SetNonBlockIO(fd);
				Log("New incoming connection.");
				nConns++;
				peers[nConns].fd = fd;
				peers[nConns].events = POLLRDNORM;
				peers[nConns].revents = 0;
				
				memset(&connStat[nConns], 0, sizeof(struct CONN_STAT));
			}
			else{
				Log("Accept LP error. :'(");
			}
			
			if (--nReady <= 0) continue;
		}
		
		for (int i=1; i<=nConns; i++) {
			printf("Receive iteration number %d\n", i);
			printf("Num connections %d\n", nConns);

			if (peers[i].revents & (POLLRDNORM | POLLERR | POLLHUP)) {
				int fd = peers[i].fd;
				int byte_count = 0;
				//read request
				if (connStat[i].nRecv < 8) {
											
					if (Recv_NonBlocking(fd, (BYTE *)&connStat[i].data, 4104, &connStat[i], &peers[i]) < 0) {
						//RemoveConnection(i);
						goto NEXT_CONNECTION;
					}
					printf("%d bytes received from local proxy\n", connStat[i].nRecv);


					if (connStat[i].nRecv >= 8) {
						while(byte_count < connStat[i].nRecv){
							bzero(buf, MAX_REQUEST_SIZE);

							printf("New packet start\n");
							int k = byte_count;
			                while(k < byte_count+8) {
			                    printf("Byte #%d:%d\n", k, connStat[i].data[k]);
			                    k++;
			                }
			                
			                u_short connection_ID = 0;
			                int seq_num = 0;
			                u_short proxy_length = 0;

			                memcpy(&connection_ID, &connStat[i].data[byte_count], 2);
			                byte_count += 2;
			                memcpy(&seq_num, &connStat[i].data[byte_count], 4);
			                byte_count += 4;
			                memcpy(&proxy_length, &connStat[i].data[byte_count], 2);
			                byte_count += 2;

			                printf("Here is i:%d\n", i);
			                printf("Here is the connection ID:%d\n", connection_ID);
			                printf("Here is the sequence number:%d\n", seq_num);
			                printf("Here is the proxy length:%d\n", proxy_length);


			                //if SYN packet, connect to server
			                if(proxy_length == 65535){
			                	printf("SYN Packet\n");
			                	//check 6 more bytes for IP and port info
			                	k = byte_count;
				                while(k < byte_count+6) {
				                    printf("Byte #%d:%d\n", k, connStat[i].data[k]);
				                    k++;
				                }

								int dst_ip = 0;
			                	u_short dst_port = 0;
								
								memcpy(&dst_ip, &connStat[i].data[byte_count], 4);
								byte_count += 4;
			                	memcpy(&dst_port, &connStat[i].data[byte_count], 2);
			                	byte_count += 2;

			                	char ip_buf[INET_ADDRSTRLEN];
				        		inet_ntop(AF_INET, &dst_ip, ip_buf, INET_ADDRSTRLEN);
			                	
			                	printf("Here is the destination IP:%s\n", ip_buf);
			                	printf("Here is the destination port:%d\n", dst_port);

			                	//fill in the address
								saddr_in.sin_family = AF_INET;
								inet_pton(AF_INET, ip_buf, &(saddr_in.sin_addr));
								saddr_in.sin_port = htons(dst_port);

								cli_fd[connection_ID] = socket(AF_INET, SOCK_STREAM, 0);
								if (cli_fd[connection_ID] < 0) 
									Error("ERROR opening socket as client");
								//connect to server
								if (connect(cli_fd[connection_ID], (const struct sockaddr *) &saddr_in, sizeof(struct sockaddr_in)) != 0){
					                Error("Error connecting to server.");
					            }
					            else{
					            	Log("Connected to server!");
					        	}
					        	client_status[connection_ID] = true;

					        	if(ooo_buffer[connection_ID].size > 0){
					    			int nSent = Send_Blocking(cli_fd[connection_ID], (const BYTE *)&ooo_buffer[connection_ID].data[0], ooo_buffer[connection_ID].size);
					                if (nSent < 0) {
							        	Log("Could not forward to server :(.");
							        	break;
							        }
							        else
							        	printf("%d bytes sent. \n", nSent);

							        int nRecv = Recv_Blocking(cli_fd[connection_ID], (BYTE *)&buf[8], 4096);
							        if (nRecv < 0){
							        	Log("Could not receive from server :(."); 
							        	break;
							        }
							        else
							        	printf("%d bytes received. \n", nRecv);

							        //update size so response code knows when to attempt to send
							        connStat[i].size = nRecv + 8; // nRecv + header length

							        u_short total_length = nRecv + 8;
							        int test_seq_num = 0; 
									//connection_ID = 1;

							        //fill in header field of send buffer
							        memcpy (&buf[0], &connection_ID, 2);
		                    		memcpy (&buf[2], &test_seq_num, 4);
		                    		memcpy (&buf[6], &total_length, 2);

		                    		int x = 0; 
		                    		while(x < 20) {
					                    printf("Byte #%d:%d\n", x, buf[x]);
					                    x++;
					                }
					                CheckData(&buf[8], nRecv);

					                ooo_buffer[connection_ID].size = 0;
					        	}

			                }
			                //if FIN
			                else if(proxy_length == 65534){
			                	printf("FIN Packet\n");
			                	//check 1 more byte for the reason flag
			                	k = byte_count;
				                while(k < byte_count + 1) {
				                    printf("Byte #%d:%d\n", k, connStat[i].data[k]);
				                    k++;
				                }

				                printf("Here is the reason flag:%d\n", connStat[i].data[byte_count]);
				                byte_count += 1;

				                close(cli_fd[connection_ID]);
				                client_status[connection_ID] = false;
				                printf("closed connection: %d\n", connection_ID);
			                }
			                //if data
							else{
								printf("Data Packet\n");
								//check length - 8 more bytes for data
								k = byte_count;
				                while(k < byte_count + proxy_length - 8) {
				                    printf("Byte #%d:%d\n", k, connStat[i].data[k]);
				                    k++;
				                }
				                if(client_status[connection_ID] == true){
				                	printf("file descripter: %d\n", cli_fd[connection_ID]);
				                	printf("client status: %d\n", client_status[connection_ID]);
					                int nSent = Send_Blocking(cli_fd[connection_ID], (const BYTE *)&connStat[i].data[byte_count], proxy_length - 8);
					                if (nSent < 0) {
							        	Log("Could not forward to server :(.");
							        	break;
							        }
							        else
							        	printf("%d bytes sent. \n", nSent);

							        byte_count += proxy_length - 8;

							        int nRecv = Recv_Blocking(cli_fd[connection_ID], (BYTE *)&buf[8], 4096);
							        if (nRecv < 0){
							        	Log("Could not receive from server :(."); 
							        	break;
							        }
							        else
							        	printf("%d bytes received. \n", nRecv);

							        //update size so response code knows when to attempt to send
							        connStat[i].size = nRecv + 8; // nRecv + header length

							        u_short total_length = nRecv + 8;
							        int test_seq_num = 0; 
									//connection_ID = 1;

							        //fill in header field of send buffer
							        memcpy (&buf[0], &connection_ID, 2);
		                    		memcpy (&buf[2], &test_seq_num, 4);
		                    		memcpy (&buf[6], &total_length, 2);

		                    		int x = 0; 
		                    		while(x < 20) {
					                    printf("Byte #%d:%d\n", x, buf[x]);
					                    x++;
					                }
					                CheckData(&buf[8], nRecv);

					                //iterate sequence_number
					                global_seq_num++;
					            }
					            else{
					            	//create buffer to send later
					            	memcpy(&ooo_buffer[connection_ID].data[0], &connStat[i].data[byte_count], proxy_length - 8);
					            	ooo_buffer[connection_ID].size = proxy_length - 8;

					            	byte_count += proxy_length - 8;
					            }
							}
						}
						//reset receive count
				        connStat[i].nRecv = 0;
					}
				}
			
				//send response
				if (connStat[i].size != 0) {
					int size = connStat[i].size;
					Log("Normal: Attempting to send %d bytes to Local Proxy", size);
					if (Send_NonBlocking(fd, buf, size, &connStat[i], &peers[i]) < 0){
						Log("Error on send to local proxy");
						//RemoveConnection(i);
						goto NEXT_CONNECTION;
					}
					if(connStat[i].nSent == size) {
						Log("Sent message to local proxy");
						connStat[i].size = 0;
						connStat[i].nSent = 0;
					}
					// may need editing
					else if(connStat[i].nSent < size){
						connStat[i].size -= connStat[i].nSent;
					}
				}
			}
			
			if (peers[i].revents & POLLWRNORM) {
				int size = connStat[i].size;
				Log("Peers: Attempting to send %d bytes to Local Proxy", size);
				if (Send_NonBlocking(peers[i].fd, buf, size, &connStat[i], &peers[i]) < 0 || connStat[i].nSent == size) {
					//RemoveConnection(i);
					goto NEXT_CONNECTION;
				}
			}
			
			NEXT_CONNECTION:
			int do_nothing;
			//if (--nReady <= 0) break;
		}
	}	
}

int main(int argc, char * * argv) {
	
	if (argc != 3) {
		Log("Usage: %s [server Port] [max concurrency]", argv[0]);
		return -1;
	}
	
	int port = atoi(argv[1]);
	int maxConcurrency = atoi(argv[2]);
	DoServer(port, maxConcurrency);
	
	return 0;
}

