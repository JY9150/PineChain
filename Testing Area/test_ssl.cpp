#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define BACKLOG_NUM 5

using namespace std;

int sockfd;
char certPath[50] = "cert.pem";
char keyPath[50] = "key.pem";

int create_socket(int portNum){
    cout << "creating a server socket" << endl;
    
    // create socket
    int sockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1){
        cout << "Error: Fail to create a socket." << endl; return (-1);
    }else{
        cout << "Successed: Socket created." << endl;
    }
    
    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; //IPv4
    addr.sin_port = htons(portNum); // convert endianness to NBO(Network Byte Order) & port: 80
    addr.sin_addr.s_addr = INADDR_ANY; //listen to all IP
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        cout << "Error : Fail to bind." << endl; return(-1);
    }
    
    // listen
    if (listen(sockfd, BACKLOG_NUM) == -1){
        cout << "Error : Fail to listen." << endl; return(-1);
    } 
    return sockfd;
}

SSL_CTX* create_context(){
    const SSL_METHOD *method;
    SSL_CTX* ctx;
    method = TLS_server_method(); //orginal: method = SSL_server_method()?;

    ctx = SSL_CTX_new(method);
    if(!ctx){
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
} 

void configure_context(SSL_CTX* ctx){    
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void signal_callback_handler(int signum){
    cout << "caught signal " << signum << endl;
    
    close(sockfd);
    exit(signum);
}



int main(){
    SSL_CTX* ctx;

    signal(SIGINT, signal_callback_handler);
    
    ctx = create_context();
    sockfd = create_socket(8000);

   //accept
    int client;
    char clientIP[INET_ADDRSTRLEN] = "";
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    const char message[255] = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 16\n\n<h1>testing</h1>";
    
    while(true){
        cout << "listening..." << endl;
        client = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        cout << "accept" << endl;
        if (client < 0){
            cout << "Error: fail to accept client with IP: " << clientIP << endl;
            continue; //?
        }
        

        SSL* ssl;
        configure_context(ctx); //<--fixing
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);
        int ret;
        if ((ret = SSL_accept(ssl))  <= 0){ // <- problem here
            ERR_print_errors_fp(stderr);
            cout << "ssl fail to accept" << endl;
            int ret2 = SSL_get_error(ssl, ret);
            //cout << "ssl get error, errorcode: "<< ret << endl;
            if(ret2 == SSL_ERROR_ZERO_RETURN) cout << "SSL_ERROR_ZERO_RETURN" << endl;
            if(ret2 == SSL_ERROR_WANT_READ) cout << " SSL_ERROR_WANT_READ" << endl;
            if(ret2 == SSL_ERROR_WANT_CONNECT) cout << "SSL_ERROR_WANT_CONNECT" << endl;
            if(ret2 == SSL_ERROR_WANT_X509_LOOKUP) cout << "SSL_ERROR_WANT_X509_LOOKUP" << endl;
            if(ret2 == SSL_ERROR_WANT_ASYNC) cout << "SSL_ERROR_WANT_ASYNC" << endl;
            if(ret2 == SSL_ERROR_WANT_ASYNC_JOB) cout << "SSL_ERROR_WANT_ASYNC_JOB" << endl;
            if(ret2 == SSL_ERROR_SYSCALL) cout << "SSL_ERROR_SYSCALL" << endl;
            if(ret2 == SSL_ERROR_SSL) cout << "SSL_ERROR_SSL" << endl;
            //break;
        }else{
            cout << "sending" << endl;
            SSL_write(ssl, message, sizeof(message));
        }

        // inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        // cout << "connect..." << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
        
        // send  
        // send(client, buffer, sizeof(buffer), 0);

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client);
    }

    close(sockfd);
    SSL_CTX_free(ctx);

    return 0;

}