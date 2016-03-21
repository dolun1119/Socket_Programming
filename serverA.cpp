#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

//TCP client port number
#define TCP_PORT "25327"
#define HOST_NAME "nunki.usc.edu"
//UDP serverA static port number
#define SA_UDP_PORT "21327"

long buf[4][4];
string udpCip;
string udpSip;
string udplocalip;
int portn;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void udpListener (const char* port) {

    int sockfd;
    int rv;
    int numbytes;
    char s[INET6_ADDRSTRLEN];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return ;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) {

        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return ;
    }
    
    freeaddrinfo(servinfo);
  
    addr_len = sizeof their_addr;

    if ((numbytes = recvfrom(sockfd, buf, sizeof(buf) , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      perror("recvfrom");
      exit(1);
    }

//get serverA UDP port no.
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    //problem!!!!!
    //udpSip=inet_ntoa(sin.sin_addr);
    //packet size
    //printf("ServerA listener: packet is %d bytes long\n", numbytes);

//get UDP client IP
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    udpCip = s;
//get UDP client IP &  Port Num;
    addr_len = sizeof their_addr;
    getpeername(sockfd, (struct sockaddr*)&their_addr, &addr_len);
//IPv4 or IPv6
    if (their_addr.ss_family == AF_INET) {
        struct sockaddr_in *sockfd = (struct sockaddr_in *)&their_addr;
        portn = ntohs(sockfd->sin_port);
        inet_ntop(AF_INET, &sockfd->sin_addr, s, sizeof s);
    } 
    else { // AF_INET6
        struct sockaddr_in6 *sockfd = (struct sockaddr_in6 *)&their_addr;
        portn = ntohs(sockfd->sin6_port);
        inet_ntop(AF_INET6, &sockfd->sin6_addr, s, sizeof s);
    }
    //  cout << "Client UDP PORT NUMBER: " << ntohs(sin.sin_port);
    //  cout << "Client UDP IP: " << inet_ntoa(sin.sin_addr);
    
//get UDP server local IP
    const char hostname[] = "nunki.usc.edu";
    int i; 
    struct hostent *he;
    struct in_addr **addr_list;
    //    if ((he = gethostbyname(hostname) == NULL)) {  // get the host info
    //        herror("gethostbyname");
    //        return 2;
    //    }
    he = gethostbyname(hostname);
    //    print information about this host:
    //    printf("Official name is: %s\n", he->h_name);
    //    printf("    IP addresses: ");
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
    //    printf("%s ", inet_ntoa(*addr_list[i]));
//get UDP server ip
    udplocalip = inet_ntoa(*addr_list[i]);
    }
    
    close(sockfd);
}


int main()
{
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];    
//server boots up
    printf("\nThe Server A is up and running.\n\n");
//set parameters    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(HOST_NAME, TCP_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }    
// loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }     
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue; 
        }
        break; 
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }    
//get serverA TCP Port Num.
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    //  else
    //  printf("port number %d\n", ntohs(sin.sin_port));
    
//read neighbor info 
    cout << "The Server A has the following neighbor information:" << endl;
    cout << "Neighbor------Cost" << endl;
    FILE *fp;
    char str[60];
    fp = fopen("serverA.txt" , "r");
    if (fp == NULL) {
        perror("Error opening file");
        return (-1);
    }
    long cost[5]={0,0,0,0,0};
    while (fgets(str, sizeof(str), fp)) {
        string line(str);
        istringstream iss(line);
        string word;
        iss>>word;//header ="server*"
        cout << word << "       ";
        int idx = word[6]-'A';
        iss>>word;//cost
        cout << word << endl;
        cost[0] = 1;
        cost[idx+1] = atoi(word.c_str());
    }
    fclose(fp);

//get TCP Client IP
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    //printf("\nServerA: connecting to %s\n", s);
    string cliIP =s;
// all done with this structure
    freeaddrinfo(servinfo);
    
//send TCP
    if (send(sockfd, cost, 5*sizeof(long), 0) == -1) {
        perror("send");
    }
    cout << endl <<"The Server A finishes sending its neighbor information to the Client with TCP port number ";
    cout << TCP_PORT;
    cout << " and IP address ";
    cout << cliIP << endl << endl;
    cout << "For this connection with the Client, the Server A has TCP port number ";
    cout << ntohs(sin.sin_port);
    cout << " and IP address ";
    cout << inet_ntoa(sin.sin_addr) << endl;
//close TCP socket
    close(sockfd);
        
//run UDP
    udpListener (SA_UDP_PORT);
    cout << "\nThe server A has received the network topology from the Client with UDP port number ";
    cout << portn;
    cout << " and IP address ";
    cout << udpCip;
    cout << " as follows: " << endl << endl;
    cout << "Edge------Cost" << endl;    
//print out topology from client
        if (buf[0][1]!=0) {
            cout << "AB        " << buf[0][1] << endl;
        }
        if (buf[0][2]!=0) {
            cout << "AC        " << buf[0][2] << endl;
        }
        if (buf[0][3]!=0) {
            cout << "AD        " << buf[0][3] << endl;
        }
        if (buf[1][2]!=0) {
            cout << "BC        " << buf[1][2] << endl;
        }
        if (buf[1][3]!=0) {
            cout << "BD        " << buf[1][3] << endl;
        }
        if (buf[2][3]!=0) {
            cout << "CD        " << buf[2][3] << endl;
        }
    
    cout << endl << "For this connection with Client, The Server A has UDP port number ";
    cout << SA_UDP_PORT;
    cout << " and IP address ";
    cout << udplocalip << endl << endl;

    return (0); 
}


