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
#include <string>
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

struct http_req_pkt{
    int seq_num;
    int ack_num;
    
    int time_sec;
    int time_msec;
    
    int true_length;

    string method;
    string req_url;
    string host;

    string code;
    string display_length; 

    string message;
};

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

    map<int, http_req_pkt> cm;
    map<int, http_req_pkt> sm;
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
    http_req_pkt requestlist[10000];
    int http_req_counter = 0;

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
    while (packet = pcap_next(handle, &header)) { 
        // header contains information about the packet (e.g. timestamp) 
        int time_sec = header.ts.tv_sec;
        int time_msec = header.ts.tv_usec;
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

                    http_req_pkt temp_http;

                    temp_http.seq_num = ntohl(tcp_hdr->th_seq);
                    temp_http.ack_num = ntohl(tcp_hdr->th_ack);
                    temp_http.message = payper;
                    temp_http.true_length = payload_length;
                    temp_http.time_sec = time_sec;
                    temp_http.time_msec = time_msec;
            

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
                                
                                bool parsed = false;
                                int parse_count = 0;
                                int space_count = 0;
                                int req_url_start;
                                int req_url_end;
                                while(!parsed){
                                    if (payper[parse_count] == ' ' && space_count == 0){
                                        temp_http.method = payper.substr(0, parse_count);
                                        req_url_start = parse_count + 1;
                                        space_count++;
                                    }
                                    else if (payper[parse_count] == ' ' && space_count == 1){
                                        req_url_end = parse_count;
                                        temp_http.req_url = payper.substr(req_url_start, req_url_end - req_url_start);
                                        parsed = true;
                                    }
                                    parse_count++;
                                }
                                parsed = false;
                                int host_start = req_url_end + 17;
                                parse_count = req_url_end + 17;
                                while(!parsed){
                                    if(payper[parse_count] == 0x0d){
                                        temp_http.host = payper.substr(host_start, parse_count - host_start);
                                        parsed = true;
                                    }
                                    parse_count++;
                                }


                                requestlist[0] = temp_http;
                                http_req_counter++;
                                connectionlist[0].client_pktcounter++;

                                connectionlist[0].cm.insert(pair<int, http_req_pkt>(ntohl(tcp_hdr->th_seq), temp_http));
                                connectionlist[0].uplink += payload_length;
                            }

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
                                


                                connectionlist[0].sm.insert(pair<int, http_req_pkt>(ntohl(tcp_hdr->th_seq), temp_http));
                                connectionlist[0].downlink += payload_length;

                                connectionlist[0].server_pktcounter++;
                            }

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
                                    bool parsed = false;
                                    int parse_count = 0;
                                    int space_count = 0;
                                    int req_url_start;
                                    int req_url_end;
                                    while(!parsed){
                                        if (payper[parse_count] == ' ' && space_count == 0){
                                            temp_http.method = payper.substr(0, parse_count);
                                            req_url_start = parse_count + 1;
                                            space_count++;
                                        }
                                        else if (payper[parse_count] == ' ' && space_count == 1){
                                            req_url_end = parse_count;
                                            temp_http.req_url = payper.substr(req_url_start, req_url_end - req_url_start);
                                            parsed = true;
                                        }
                                        parse_count++;
                                    }
                                    parsed = false;
                                    int host_start = req_url_end + 17;
                                    parse_count = req_url_end + 17;
                                    while(!parsed){
                                        if(payper[parse_count] == 0x0d){
                                            temp_http.host = payper.substr(host_start, parse_count - host_start);
                                            parsed = true;
                                        }
                                        parse_count++;
                                    }

                                    requestlist[http_req_counter] = temp_http;
                                    http_req_counter++;
                                    connectionlist[i].client_pktcounter++;

                                    connectionlist[i].cm.insert(pair<int, http_req_pkt>(ntohl(tcp_hdr->th_seq), temp_http));
                                    connectionlist[i].uplink += payload_length;
                                }
                                new_connect = false;
                            }
                            //if downlink, add payload to map sm
                            else if(connectionlist[i].dest_port == test.source_port && connectionlist[i].source_port == test.dest_port && connectionlist[i].dest_ip == test.source_ip && connectionlist[i].source_ip == test.dest_ip){
                                if (payload_length > 0) {
                                    


                                    connectionlist[i].sm.insert(pair<int, http_req_pkt>(ntohl(tcp_hdr->th_seq), temp_http));
                                    connectionlist[i].downlink += payload_length;
                                    connectionlist[i].server_pktcounter++;
                                }
                    
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
                                    bool parsed = false;
                                    int parse_count = 0;
                                    int space_count = 0;
                                    int req_url_start;
                                    int req_url_end;
                                    while(!parsed){
                                        if (payper[parse_count] == ' ' && space_count == 0){
                                            temp_http.method = payper.substr(0, parse_count);
                                            req_url_start = parse_count + 1;
                                            space_count++;
                                        }
                                        else if (payper[parse_count] == ' ' && space_count == 1){
                                            req_url_end = parse_count;
                                            temp_http.req_url = payper.substr(req_url_start, req_url_end- req_url_start);
                                            parsed = true;
                                        }
                                        parse_count++;
                                    }
                                    parsed = false;
                                    int host_start = req_url_end + 17;
                                    parse_count = req_url_end + 17;
                                    while(!parsed){
                                        if(payper[parse_count] == 0x0d){
                                            temp_http.host = payper.substr(host_start, parse_count - host_start);
                                            parsed = true;
                                        }
                                        parse_count++;
                                    }

                                    requestlist[http_req_counter] = temp_http;
                                    http_req_counter++;
                                    connectionlist[con_counter].client_pktcounter++;

                                    connectionlist[con_counter].cm.insert(pair<int, http_req_pkt>(ntohl(tcp_hdr->th_seq), temp_http));
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
                                    


                                    connectionlist[con_counter].sm.insert(pair<int, http_req_pkt>(ntohl(tcp_hdr->th_seq), temp_http));
                                    connectionlist[con_counter].downlink += payload_length;

                                    connectionlist[con_counter].server_pktcounter++;
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

    for(int i = 0; i < con_counter; i++){
        //cout << connectionlist[i].client_pktcounter << endl;
        //cout << connectionlist[i].server_pktcounter << endl;
    }
//    for(int i = 0; i < http_req_counter; i++){
//        cout << requestlist[i].req_url << endl;
//        cout << requestlist[i].method << endl;
//        cout << requestlist[i].host << endl;
//    }
    int trans_count;
    string http_transactionsx[1000];
    map<int, string> http_transactions;
    
    for(int i = 0; i < con_counter; i++){

        typedef map<int, http_req_pkt>::const_iterator MapIterator;

        //cout << endl << "Uplink begin" << endl;
        for (MapIterator iter = connectionlist[i].cm.begin(); iter != connectionlist[i].cm.end(); iter++){
            //cout << endl << "Sequence number: " << iter->first << endl;
            //cout << iter->second.message << endl;
            //cout << iter->second.method << endl;
            //cout << iter->second.req_url << endl;
            //cout << iter->second.host << endl;
            //cout << "Seq:" << iter->second.seq_num << endl;
            //cout << "Ack:" << iter->second.ack_num << endl;

            for (MapIterator iter = connectionlist[i].sm.begin(); iter != connectionlist[i].sm.end(); iter++){
                //cout << endl << "Sequence number: " << iter->first << endl; 
                //cout << "Seq:" << iter->second.seq_num << endl;
                //cout << "Ack:" << iter->second.ack_num << endl;
                //cout << "Length:" << iter->second.true_length << endl;
                //cout << iter->second.message << endl;
            }

            string req_host;

            string req_data = iter->second.message;
            int req_length = iter->second.true_length;

            int req_parse_count;
            bool req_found = false;
            if (req_length >= 6){
                for(req_parse_count = 0; req_parse_count < (req_length - 6); req_parse_count++){
                    if(req_data.substr(req_parse_count, 6) == "Host: "){
                        //cout << "Host: " << req_parse_count << endl;
                        req_found = true;
                        break;
                    } 
                }
            }
            //cout << req_parse_count << endl;
            if(req_found){
                for(int i = req_parse_count + 6; i < req_length - 2; i++){
                    if(req_data[i] == 0x0d && req_data[i+1] == 0x0a){
                        req_host = req_data.substr(req_parse_count + 6, i - (req_parse_count + 6));
                        break;
                    }
                }
            }
            else{
                req_host = "n/a";
            }

            //cout << req_host << endl;
            stringstream full_response;


            string rep_data = connectionlist[i].sm.at(iter->second.ack_num).message;
            full_response << rep_data;
            int rep_seq = connectionlist[i].sm.at(iter->second.ack_num).seq_num;
            
            int rep_length = connectionlist[i].sm.at(iter->second.ack_num).true_length;
            rep_seq += rep_length;

            bool done = false;
            
            while(connectionlist[i].sm.find(rep_seq) != connectionlist[i].sm.end()){ 
                full_response << connectionlist[i].sm.at(rep_seq).message;
                rep_length += connectionlist[i].sm.at(rep_seq).true_length;
                rep_seq += connectionlist[i].sm.at(rep_seq).true_length;
            }
            //cout << rep_length << endl;
            //cout << rep_data << endl;
            string rep_code = rep_data.substr(9,3);
            //cout << rep_code << endl;

            string full_rep_data = full_response.str();

            int rep_display_length = rep_length;
            
            int rep_parse_count;
            bool rep_found = false;
            bool chunked = false;
            if (rep_length >= 19){
                for(rep_parse_count = 0; rep_parse_count < (rep_length - 19); rep_parse_count++){
                    if(rep_data.substr(rep_parse_count, 16) == "Content-Length: "){
                        //cout << "Content-Length: " << rep_parse_count << endl;
                        rep_found = true;
                        break;
                    }
                    else if(rep_data.substr(rep_parse_count, 19) == "Transfer-Encoding: "){
                        //cout << "Transfer-Encoding: " << rep_parse_count << endl;
                        rep_found = true;
                        chunked = true;
                        break;
                    }
                }
            }

            stringstream imagess;
            if(rep_found){
                if(chunked){
                    rep_display_length = 0;
                    int chunk_start = 0;
                    int chunk_length = 0;
                    for(int i = rep_parse_count + 19; i < rep_length - 4; i++){
                        //chunk start
                        if(full_rep_data[i] == 0x0d && full_rep_data[i+1] == 0x0a && full_rep_data[i+2] == 0x0d && full_rep_data[i+3] == 0x0a){
                            //first chunk
                            for(int j = i + 4; j < rep_length - 2; j++){
                                if(full_rep_data[j] == 0x0d && full_rep_data[j+1] == 0x0a){
                                    rep_display_length += stoi(full_rep_data.substr(i+4,j-(i+4)), NULL, 16);
                                    chunk_length = stoi(full_rep_data.substr(i+4,j-(i+4)), NULL, 16);
                                    chunk_start = j+2;
                                    //cout << "Chunk length: " << chunk_length << endl;
                                    //cout << "Rep_display: " << rep_display_length << endl;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    bool finished = false;
                    int overflow = 0;
                    while(!finished){
                        //cout << "new chunk" << endl;
                        imagess << full_rep_data.substr(chunk_start, chunk_length);
                        int next_num = chunk_start + chunk_length;
                        //cout << next_num << endl;
                        if(full_rep_data[next_num] == 0x0d && full_rep_data[next_num+1] == 0x0a){
                            for(int i = next_num + 2; i < next_num + 10; i++){
                                if(full_rep_data[i] == 0x0d && full_rep_data[i+1] == 0x0a){
                                    rep_display_length += stoi(full_rep_data.substr(next_num + 2, i - (next_num+2)), NULL, 16);
                                    chunk_length = stoi(full_rep_data.substr(next_num + 2, i - (next_num+2)), NULL, 16);
                                    chunk_start = i+2;
                                    if(chunk_length == 0){
                                        //cout << "Done chunking this bitch." << endl;
                                        finished = true;
                                        break;
                                    }
                                    break;
                                }
                            }
                        }
                        else{
                            cout << "Major error lolz ;)" << endl;
                            finished = true;
                            break;
                        }
                        //cout << rep_display_length;
                    }



                }
                else{
                    for(int i = rep_parse_count + 16; i < rep_length - 2; i++){
                        if(rep_data[i] == 0x0d && rep_data[i+1] == 0x0a){
                            rep_display_length = stoi(rep_data.substr(rep_parse_count + 16, i - (rep_parse_count + 16)));
                            break;
                        }
                    }
                }
            }
            else{
                rep_display_length = 0;
            }

            if(iter->second.true_length > 0){
                stringstream httpss;
                httpss << iter->second.req_url << ' ' << req_host << ' ' << rep_code << ' ' << rep_display_length;
                http_transactions.insert(pair<int, string>(trans_count, httpss.str()));
                trans_count++;
            }
        }

        //cout << endl << "Downlink begin" << endl;
        for (MapIterator iter = connectionlist[i].sm.begin(); iter != connectionlist[i].sm.end(); iter++){
            //cout << endl << "Sequence number: " << iter->first << endl; 
            //cout << "Seq:" << iter->second.seq_num << endl;
            //cout << "Ack:" << iter->second.ack_num << endl;
            //cout << "Length:" << iter->second.true_length << endl;
            //cout << iter->second.message << endl;
        }
    } 
    typedef map<int, string>::const_iterator httpIterator;
    for (httpIterator iter = http_transactions.begin(); iter != http_transactions.end(); iter++){
        cout << iter->second << endl;
    }

    return 0;
}