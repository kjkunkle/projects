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
#include <algorithm>
#include <vector>
#include <map>


#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <net/if_arp.h>

#include <netinet/if_ether.h>

#include <pcap.h>
#include <net/ethernet.h>
#include <arpa/inet.h>


using namespace std;

#define ETHER_TYPE_IP (0x0800)
#define ETHER_TYPE_8021Q (0x8100)
#define IP_TYPE_TCP (0x06)
#define IP_TYPE_UDP (0x11)
#define TCP_ACK (0x010)
#define TCP_SYN (0x002)
#define TCP_SYN_ACK (0x012)
#define TCP_FIN_ACK (0x011)

/*
template<size_t N>

string convert(char const(&data)[N])
{
   return string(data, std::find(data, data + N, '\0'));
}
*/

struct tcp_connection{
    string source_ip;
    u_short source_port;
    string dest_ip;
    u_short dest_port;
    u_short flag;
    
    int client_pktcounter;
    int server_pktcounter;

    int uplink;
    int downlink;

    map<int, string> cm;
    map<int, string> sm;
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
    		    pkt_ptr += (tcp_hdr->th_off)*4;   //skip past TCP header to payload
                
                int payload_length = packet_length - ((ip_hdr->ip_hl)*4 + (tcp_hdr->th_off)*4);
                
                
                //if http packet
                if(ntohs(tcp_hdr->th_dport) == 80 || ntohs(tcp_hdr->th_sport) == 80){
                    //printf("Payload size: %d bytes\n", payload_length);
                    char pay_buffer[payload_length];
                    const u_char *temp_pointer = pkt_ptr;
                    int i = 0;
                    while(i < payload_length) {
                        pay_buffer[i] = *temp_pointer;
                        temp_pointer++;
                        i++;
                    }
                    string payper(pay_buffer, payload_length);


                    // Print payload in ASCII
                    /*if (payload_length > 0) {
                        cout << ntohl(tcp_hdr->th_seq) << endl;
                        const u_char *temp_pointer = pkt_ptr;
                        int byte_count = 0;
                        while (byte_count++ < payload_length) {
                            printf("%c", *temp_pointer);
                            temp_pointer++;
                        }
                    } 
                    */
            

                    if (con_counter == 0){
                        //if client, make new connection struct
                        if(ntohs(tcp_hdr->th_dport) == 80){
                            connectionlist[0].source_port = ntohs(tcp_hdr->th_sport);
                            connectionlist[0].dest_port = ntohs(tcp_hdr->th_dport);
                            char source_ip[INET_ADDRSTRLEN];
                            char dest_ip[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(ip_hdr->ip_src), source_ip, INET_ADDRSTRLEN);
                            inet_ntop(AF_INET, &(ip_hdr->ip_dst), dest_ip, INET_ADDRSTRLEN);
                            string transfer_s(source_ip);
                            string transfer_d(dest_ip);
                            connectionlist[0].source_ip = transfer_s;
                            connectionlist[0].dest_ip = transfer_d;
                            if (payload_length > 0) {
                                connectionlist[0].cm.insert(pair<int, string>(ntohl(tcp_hdr->th_seq), payper));
                                connectionlist[0].uplink += payload_length;
                            }
                            connectionlist[0].client_pktcounter++;

                            con_counter++;
                        }
                        //if server, make new connection struct
                        else if(ntohs(tcp_hdr->th_sport) == 80){
                            connectionlist[0].source_port = ntohs(tcp_hdr->th_dport);
                            connectionlist[0].dest_port = ntohs(tcp_hdr->th_sport);
                            char source_ip[INET_ADDRSTRLEN];
                            char dest_ip[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(ip_hdr->ip_src), source_ip, INET_ADDRSTRLEN);
                            inet_ntop(AF_INET, &(ip_hdr->ip_dst), dest_ip, INET_ADDRSTRLEN);
                            string transfer_s(source_ip);
                            string transfer_d(dest_ip);
                            connectionlist[0].source_ip = transfer_d;
                            connectionlist[0].dest_ip = transfer_s;
                            if (payload_length > 0) {
                                connectionlist[0].sm.insert(pair<int, string>(ntohl(tcp_hdr->th_seq), payper));
                                connectionlist[0].downlink += payload_length;
                            }

                            connectionlist[0].server_pktcounter++;
                            con_counter++;
                        } 
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
                            //if uplink, add payload to map cm
                            if(connectionlist[i].source_port == test.source_port && connectionlist[i].dest_port == test.dest_port && connectionlist[i].source_ip == test.source_ip && connectionlist[i].dest_ip == test.dest_ip){
                                if (payload_length > 0) {
                                    connectionlist[i].cm.insert(pair<int, string>(ntohl(tcp_hdr->th_seq), payper));
                                    connectionlist[i].uplink += payload_length;
                                }
                                connectionlist[i].client_pktcounter++;
                                new_connect = false;
                            }
                            //if downlink, add payload to map sm
                            else if(connectionlist[i].dest_port == test.source_port && connectionlist[i].source_port == test.dest_port && connectionlist[i].dest_ip == test.source_ip && connectionlist[i].source_ip == test.dest_ip){
                                if (payload_length > 0) {
                                    connectionlist[i].sm.insert(pair<int, string>(ntohl(tcp_hdr->th_seq), payper));
                                    connectionlist[i].downlink += payload_length;
                                }
                                connectionlist[i].server_pktcounter++;
                                new_connect = false;
                            }
                        }
                        if(new_connect == true){
                            //if client, create new connection struct
                            if(ntohs(tcp_hdr->th_dport) == 80){
                                connectionlist[con_counter].source_port = test.source_port;
                                connectionlist[con_counter].dest_port = test.dest_port;
                                connectionlist[con_counter].source_ip = test.source_ip;
                                connectionlist[con_counter].dest_ip = test.dest_ip;
                                if (payload_length > 0) {
                                    connectionlist[con_counter].cm.insert(pair<int, string>(ntohl(tcp_hdr->th_seq), payper));
                                    connectionlist[con_counter].uplink += payload_length;
                                }
                                con_counter++;
                            }
                            //if server, create new connection struct
                            else if(ntohs(tcp_hdr->th_sport) == 80){
                                connectionlist[con_counter].source_port = test.dest_port;
                                connectionlist[con_counter].dest_port = test.source_port;
                                connectionlist[con_counter].source_ip = test.dest_ip;
                                connectionlist[con_counter].dest_ip = test.source_ip;
                                if (payload_length > 0) {
                                    connectionlist[con_counter].sm.insert(pair<int, string>(ntohl(tcp_hdr->th_seq), payper));
                                    connectionlist[con_counter].downlink += payload_length;
                                }
                                con_counter++;
                            }
                        }
                    
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

    string con_strings[1000];
    for(int i = 0; i < con_counter; i++){
        stringstream css;
    	css << connectionlist[i].source_ip << ' ' << connectionlist[i].source_port << ' ' << connectionlist[i].dest_ip << ' ' << connectionlist[i].dest_port << ' ' << connectionlist[i].uplink << ' ' << connectionlist[i].downlink;
        con_strings[i] = css.str();
    }

    for (int i = 0; i < con_counter; ++i)
    {
        bool swapped = false;
        for (int j = 0; j < con_counter - (i+1); ++j)
        {
            if (con_strings[j] > con_strings[j+1])
            {
                string temp_s = con_strings[j];
                tcp_connection temp_tcp = connectionlist[j];

                con_strings[j] = con_strings[j+1];
                connectionlist[j] = connectionlist[j+1];

                con_strings[j+1] = temp_s;
                connectionlist[j+1] = temp_tcp;

                swapped = true;
            }
        }
        
        if (!swapped) break;
    }

    for(int i = 0; i < con_counter; i++){
        cout << con_strings[i] << endl;
    }

    for(int i = 0; i < con_counter; i++){
        //cout << connectionlist[i].source_port << endl;
        //cout << connectionlist[i].dest_port << endl;
        typedef map<int, string>::iterator MapIterator;
        //cout << endl << "Uplink begin" << endl;

        for (MapIterator iter = connectionlist[i].cm.begin(); iter != connectionlist[i].cm.end(); iter++){
            //cout << endl << "Sequence number: " << iter->first << endl;
            cout << iter->second;
        }

    
        //cout << endl << "Downlink begin" << endl;
        for (MapIterator iter = connectionlist[i].sm.begin(); iter != connectionlist[i].sm.end(); iter++){
            //cout << endl << "Sequence number: " << iter->first << endl; 
            cout << iter->second;
        }
    }
    return 0;
}