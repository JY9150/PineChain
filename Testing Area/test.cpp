#include <iostream>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BACKLOG_NUM 5 // max

using namespace std;

int main()
{  
    cout << "This is Server" << endl;
    
    // create socket
    int sockfd=0;
    sockfd=socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd== -1){
        cout << "Error: Fail to create a socket." << endl; return (-1);
    }else{
        cout << "Successed: Socket created." << endl;
    }
    
    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; //IPv4
    addr.sin_port = htons(8000); // convert endianness to NBO(Network Byte Order) & port: 80
    addr.sin_addr.s_addr = INADDR_ANY; //listen to all IP
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        cout << "Error : Fail to bind." << endl; return(-1);
    }
    
    // listen
    if (listen(sockfd, BACKLOG_NUM) == -1){
        cout << "Error : Fail to listen." << endl; return(-1);
    }    
    
    //accept
    int conn;
    char clientIP[INET_ADDRSTRLEN] = "";
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    while(true){
        cout << "listening..." << endl;
        conn = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        cout << "accept" << endl;
        if (conn < 0){
            cout << "Error: fail to accept client with IP: " << clientIP << endl;
            continue; //?
        }
        
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "connect..." << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
        
        // send  
        cout << "sending msg" << endl;
        char buffer[255] = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 16\n\n<h1>testing</h1>";
        send(conn, buffer, sizeof(buffer), 0);
    }




    
    
    
    return 0;
}

