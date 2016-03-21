#include <stdio.h>
#include <stdlib.h>
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
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

//TCP
#define PORT "25327"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 100

//udp
#define SA_UDP_PORT "21327"
#define SB_UDP_PORT "22327"
#define SC_UDP_PORT "23327"
#define SD_UDP_PORT "24327"
#define HOST_NAME "nunki.usc.edu"

long info[4][4] = { {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0} };

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;    
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string numto (int num) {
    if (num==0) {return "A";}
    else if (num==1) {return "B";}
    else if (num==2) {return "C";}
    else if (num==3) {return "D";}
    else {return "N.A";}
}

void udpTalker (const char* port, string sername) {
    
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    string udpSer = sername;
    //get server ip
    char s[INET6_ADDRSTRLEN];
    int uportNum = 0;
    if ( udpSer =="A") {uportNum=21327;}
    if ( udpSer =="B") {uportNum=22327;}
    if ( udpSer =="C") {uportNum=23327;}
    if ( udpSer =="D") {uportNum=24327;}
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(HOST_NAME, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return  ;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return ;
    }
    
//get UDP server IP
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    
    freeaddrinfo(servinfo);
    
    if ((numbytes = sendto(sockfd, info, sizeof(info), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
   
//get UDP client IP
    const char hostname[] = "nunki.usc.edu";
    int i;
    string udplocalip;
    struct hostent *he;
    struct in_addr **addr_list;
    //    if ((he = gethostbyname(hostname) == NULL)) {  // get the host info
    //        herror("gethostbyname");
    //        return 2;
    //    }
    he = gethostbyname(hostname);
    // print information about this host:
    //    printf("Official name is: %s\n", he->h_name);
    //    printf("    IP addresses: ");
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
          udplocalip = inet_ntoa(*addr_list[i]);
        //        printf("%s ", inet_ntoa(*addr_list[i]));
    }
        //    cout << "*udplocalip*" << udplocalip <<endl;
//get UDP client port num.
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    
    cout << "The Client has sent the network topology to the network topology to the Server ";
    cout << udpSer << " with UDP port number " << uportNum;
    cout << " and IP address " << s << " as follows:" << endl << endl;
    cout << "Edge------Cost" << endl;
//get topology
        if (info[0][1]!=0) {
            cout << "AB        " << info[0][1] << endl;
        }
        if (info[0][2]!=0) {
            cout << "AC        " << info[0][2] << endl;
        }
        if (info[0][3]!=0) {
            cout << "AD        " << info[0][3] << endl;
        }
        if (info[1][2]!=0) {
            cout << "BC        " << info[1][2] << endl;
        }
        if (info[1][3]!=0) {
            cout << "BD        " << info[1][3] << endl;
        }
        if (info[2][3]!=0) {
            cout << "CD        " << info[2][3] << endl;
        }
    cout << endl << "For this connection with Server " << udpSer;
    cout << ", The Client has UDP port number " << ntohs(sin.sin_port);
    cout << " and IP address " << udplocalip << "." << endl << endl;
    //    printf("Client talker: sent %d bytes to %s\n", numbytes, HOST_NAME);
    
    close(sockfd);
}


int main(void)
{
    int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int rv;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    long buf[MAXDATASIZE];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(HOST_NAME, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
}
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("server: socket");
            continue; 
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1); }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue; }
        
        break; }

//get Client TCP local IP & port number.
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    //   cout << "local ip" << inet_ntoa(sin.sin_addr) <<endl;
    //   cout << "local port#" ntohs(sin.sin_port) << endl ;
    cout << endl << "The Client has TCP port number " << ntohs(sin.sin_port);
    cout << " and IP address " << inet_ntoa(sin.sin_addr) << endl << endl;
    int tcpCport = ntohs(sin.sin_port);
    string tcpCip = inet_ntoa(sin.sin_addr);
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1); 
    }
    
    int got[4] = {0,0,0,0};
    string tcpSip;
    string ser;
    
    while(got[0]==0 || got[1]==0 || got[2]==0 || got[3]==0) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue; 
        }
        inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
        //get server ip
        tcpSip = s;
        //printf("Client: got TCP connection from Server A %s\n", s);      
        //not use fork()
        //if (!fork()) { // this is the child process
        //close(sockfd); // child doesn't need the listener        
//get TCP SERVER IP & port num;
        int tcpSport;
        sin_size = sizeof their_addr;
        getpeername(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        // IPv4 or IPv6
        if (their_addr.ss_family == AF_INET) {
            struct sockaddr_in *sockfd = (struct sockaddr_in *)&their_addr;
            tcpSport = ntohs(sockfd->sin_port);
            inet_ntop(AF_INET, &sockfd->sin_addr, s, sizeof s);
        }
        else { // AF_INET6
            struct sockaddr_in6 *sockfd = (struct sockaddr_in6 *)&their_addr;
            //port num
            tcpSport = ntohs(sockfd->sin6_port);
            //IP: s
            inet_ntop(AF_INET6, &sockfd->sin6_addr, s, sizeof s);
        }
            
        if ((numbytes = recv(new_fd, buf, (MAXDATASIZE-1)*sizeof(long), 0)) == -1) {
            perror("recv");
            close(new_fd);
            exit(1); 
        }    
//recv from A
            if (buf[0] == 1) {
                got[0]=1;
                ser="A";
                for (int j=0; j < 4; j++) { info[0][j] = buf[j+1];}
            }
//recv from B
            if (buf[0] == 2) {
                got[1]=1;
                ser="B";
                for (int j=0; j < 4; j++) { info[1][j] = buf[j+1];}
            }
//recv from C
            if (buf[0] == 3) {
                got[2]=1;
                ser="C";
                for (int j=0; j < 4; j++) { info[2][j] = buf[j+1];}
            }
//recv from D
            if (buf[0] == 4) {
                got[3]=1;
                ser="D";
                for (int j=0; j < 4; j++) { info[3][j] = buf[j+1];}
            }

            cout << "The Client receivers neighbor information from the Server ";
            cout << ser << " with TCP port number ";
            cout << tcpSport << " and IP address ";
            cout << tcpSip << endl << endl;
        
            cout << "The Server " << ser << " has the following neighbor information: " << endl;
            cout << "Neighbor------Cost" << endl;
                if (buf[1] != 0) {cout << "serverA       " << buf[1] << endl;}
                if (buf[2] != 0) {cout << "serverB       " << buf[2] << endl;}
                if (buf[3] != 0) {cout << "serverC       " << buf[3] << endl;}
                if (buf[4] != 0) {cout << "serverD       " << buf[4] << endl;}
            
            cout << endl << "For this connection with Server " << ser;
            cout << ", The Client has TCP port number " << tcpCport;
            cout << " and IP address " << tcpCip << endl << endl << endl;
                
        close(new_fd);  // parent doesn't need this
    }
    
    //Run UDP
    udpTalker (SA_UDP_PORT, "A");
    udpTalker (SB_UDP_PORT, "B");
    udpTalker (SC_UDP_PORT, "C");
    udpTalker (SD_UDP_PORT, "D");
    //calculate spanning tree
    long small1, small2, small3;
    small1 = small2 = small3 = LONG_MAX;
    int visit [4] = {0,0,0,0};
    int row1,row2,row3,col1,col2,col3;
    for (int i=0; i<4; i++) {
        for (int j=i; j<4; j++) {
            if (info[i][j]!=0 && info[i][j] < small1) {
                small1 = info[i][j];
                row1 = i;
                col1 = j;
            }
        }
    }
    info[row1][col1]=0;
    for (int i=0; i<4; i++) {
        for (int j=i; j<4; j++) {
            if (info[i][j]!=0 && info[i][j] < small2) {
                small2 = info[i][j];
                row2 = i;
                col2 = j;
            }
        }
    }
    info[row2][col2]=0;
    visit[row1]++;
    visit[col1]++;
    visit[row2]++;
    visit[col2]++;
    for (int i=0; i<4; i++) {
        for (int j=i; j<4; j++) {
            if (info[i][j]!=0 && info[i][j] < small3) {
                small3 = info[i][j];
                row3 = i;
                col3 = j;
            }
        }
    }
    visit[row3]++;
    visit[col3]++;
    while (visit[0]==0 || visit[1]==0 || visit[2]==0 ||visit[3]==0) {
        info[row3][col3]=0;
        small3= LONG_MAX;
        visit[row3]--;
        visit[col3]--;
        for (int i=0; i<4; i++) {
            for (int j=i; j<4; j++) {
                if (info[i][j]!=0 && info[i][j] < small3) {
                    small3 = info[i][j];
                    row3 = i;
                    col3 = j;
                }
            }
        }
        visit[row3]++;
        visit[col3]++;
    }
    //test
    //    cout << numto(row1) << numto(col1) << endl;
    //    cout << numto(row2) << numto(col2) << endl;
    //    cout << numto(row3) << numto(col3) << endl;
    //    cout << small1 << endl;
    //    cout << small2 << endl;
    //    cout << small3 << endl;
    //    for (int i =0; i<4; i++){cout << visit[i] << ",";}
    
    cout << "The Client has calculated a tree. The tree cost is " << small1+small2+small3 << endl;
    cout << "Edge------Cost" << endl;
    cout << numto(row1) << numto(col1) << "        " << small1 << endl;
    cout << numto(row2) << numto(col2) << "        " << small2 << endl;
    cout << numto(row3) << numto(col3) << "        " << small3 << endl;
    
return 0; 
}



