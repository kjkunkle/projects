//
//  main.cpp
//  Comp438 proj2
//
//  Created by Kevin Kunkle on 10/6/15.
//  Copyright © 2015 Kevin Kunkle. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>


#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <net/if_arp.h>

#include <netinet/if_ether.h>

#include <pcap.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include <map>

using namespace std;

#define ETHER_TYPE_IP (0x0800)
#define ETHER_TYPE_8021Q (0x8100)
#define IP_TYPE_TCP (0x06)
#define IP_TYPE_UDP (0x11)



struct tcp_connection{
    string source_ip;
    u_short source_port;
    string dest_ip;
    u_short dest_port;
};

struct con_map{
     std::map<int, char> m;

};

typedef unsigned char BYTE;


int main(int argc, const char * argv[])
{
    int pkt_counter = 0;   // packet counter
    int ip_counter = 0;    // counts # of IP packets
    int tcp_counter = 0;   // counts # of tcp packets
    int udp_counter = 0;   // counts # of udp packets
    int con_counter = 0;  // counts every tcp connection
    tcp_connection connectionlist[10000];

    unsigned long byte_counter = 0; //total bytes seen in entire trace

    //temporary packet buffers
    struct pcap_pkthdr header; // The header that pcap gives us
    const u_char *packet; // The actual packet


    //FILE * ifs = stdin;
    //FILE * ifs = fopen("Untitled", "rb");

    char errbuf[PCAP_ERRBUF_SIZE]; //error buffer for error reports

    pcap_t * handle = pcap_fopen_offline(stdin, errbuf);   //call pcap library function

    if (handle == NULL) {
        cout << "Couldn't open pcap file %s: %s\n";
        return(2);
    }
    while (packet = pcap_next(handle,&header)) { 
    	// header contains information about the packet (e.g. timestamp) 
    	u_char *pkt_ptr = (u_char *)packet; //cast a pointer to the packet data 
      
    	//parse the first (ethernet) header, grabbing the type field 
    	int ether_type = ((int)(pkt_ptr[12]) << 8) | (int)pkt_ptr[13]; 
    	int ether_offset = 0; 
 
    	if (ether_type == ETHER_TYPE_IP){ 
    		ether_offset = 14;
		    ip_counter++;
		    pkt_ptr += ether_offset;  //skip past the Ethernet II header 

		    struct ip *ip_hdr = (struct ip *)pkt_ptr; //IP header struct 

		    int packet_length = ntohs(ip_hdr->ip_len); 
        	pkt_ptr += (ip_hdr->ip_hl)*4;  //skip past IP header

		    byte_counter += packet_length; //byte counter update

		    int ip_type = ip_hdr->ip_p;

    		if (ip_type == IP_TYPE_TCP){
    		    
    		    struct tcphdr *tcp_hdr = (struct tcphdr *)pkt_ptr; //TCP struct
    		    pkt_ptr += tcp_hdr->th_off;   //skip past TCP header

    		    if (con_counter == 0){
    		        connectionlist[0].source_port = ntohs(tcp_hdr->th_sport);
                    connectionlist[0].dest_port = ntohs(tcp_hdr->dest);
                    char source_ip[INET_ADDRSTRLEN];
                    char dest_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(ip_hdr->ip_src), source_ip, INET_ADDRSTRLEN);
                    inet_ntop(AF_INET, &(ip_hdr->ip_dst), dest_ip, INET_ADDRSTRLEN);
                    string transfer_s(source_ip);
                    string transfer_d(dest_ip);
                    connectionlist[0].source_ip = transfer_s;
                    connectionlist[0].dest_ip = transfer_d;

                    con_counter++;
    		    }
                else{
                    tcp_connection test;
                    test.source_port = ntohs(tcp_hdr->th_sport);
                    test.dest_port = ntohs(tcp_hdr->th_dport);
                    char source_ip[INET_ADDRSTRLEN];
                    char dest_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(ip_hdr->ip_src), source_ip, INET_ADDRSTRLEN);
                    inet_ntop(AF_INET, &(ip_hdr->ip_dst), dest_ip, INET_ADDRSTRLEN);
                    string transfer_s(source_ip);
                    string transfer_d(dest_ip);
                    test.source_ip = transfer_s;
                    test.dest_ip = transfer_d;

                    bool new_connect = true;

                    int i;

                    for(i = 0; i < con_counter; i++){
                        if(connectionlist[i].source_port == test.source_port && connectionlist[i].dest_port == test.dest_port && connectionlist[i].source_ip == test.source_ip && connectionlist[i].dest_ip == test.dest_ip || connectionlist[i].dest_port == test.source_port && connectionlist[i].source_port == test.dest_port && connectionlist[i].dest_ip == test.source_ip && connectionlist[i].source_ip == test.dest_ip){
                            new_connect = false;
                        }
                    }
                    if(new_connect == true){
                        connectionlist[con_counter].source_port = test.source_port;
                        connectionlist[con_counter].dest_port = test.dest_port;
                        connectionlist[con_counter].source_ip = test.source_ip;
                        connectionlist[con_counter].dest_ip = test.dest_ip;

                        con_counter++;
                    }
                    
                }
    		   
    		    tcp_counter++;
    		}
    		else if (ip_type == IP_TYPE_UDP){
    		    udp_counter++;
    		} 
	   }

    	pkt_counter++; //increment number of packets seen

    } //end internal loop for reading packets (all in one file)

    pcap_close(handle);  //close the pcap file

    cout << pkt_counter << ' ' << ip_counter << ' ' << tcp_counter << ' ' << udp_counter << ' ' << con_counter << endl;
    return 0;
}
