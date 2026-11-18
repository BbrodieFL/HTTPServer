#include<iostream>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// the people really do not like this but whatver
using namespace std;

int main(){


    /// first things first let us create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){
        perror("Failed to create socket");
        return 1;
    }


    // Allow address reuse
    // copied and pasted this don't get it
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //next I need to associate my server with the things
    // But first we setup the server
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    //I don't understand this line
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(5174);

    if(bind(server_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        perror("Unable to bind");
        close(server_fd);
        return 1;
    }

    if(listen(server_fd,10)){
        perror("Unabled to listen on port 5174");
        close(server_fd);
        return 1;
    }

    cout << "Listening on server port 5174" << '\n';

    int client_fd = accept(server_fd,nullptr,nullptr);
    if(client_fd < 0){
        perror("Unable to accept connection");
        close(server_fd);
        return 1;
    }

    char buffer[4096];
    ssize_t bytes_recieved = recv(client_fd,buffer, sizeof(buffer), 0);

    if(bytes_recieved < 0){
        perror("Unable to recieve the request");
        close(client_fd);
        close(server_fd);
        return 1;
    }
    if(bytes_recieved == 0){
        cout << "Client closed the connection.\n";
        close(client_fd);
        close(server_fd);
        return 0;
    }

    if(bytes_recieved > 0){
        cout.write(buffer,bytes_recieved);
        cout << '\n';
    }

    string body = "Hello from my server!\n";
    string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    ssize_t bytes_sent = send(client_fd, response.data(), response.size(), 0);
    if(bytes_sent < 0){
        perror("Could send back to client");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    close(client_fd);
    close(server_fd);
    return 0;

}
