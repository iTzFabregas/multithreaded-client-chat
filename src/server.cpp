#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <thread>
#include <vector>

void handle_client(int clientSocket) {


    // Receive the name of the new client
    char buffer[1024];
    char client_name[16];
    int bytesRead = recv(clientSocket, client_name, sizeof(client_name), 0);
    if (bytesRead <= 0) {
        std::cerr << "Connection closed by client" << std::endl;
        close(clientSocket);
        return;
    }
    client_name[bytesRead] = '\0';
    std::cout << "New client [ " << client_name << " ] connected!" << std::endl;

    // Echo the received data back to the client
    send(clientSocket, client_name, strlen(client_name), 0);
    
    // Send and receive data
    while (true) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Connection closed by client" << std::endl;
            break;
        }
        buffer[bytesRead] = '\0';
        std::cout << "[ "<< client_name << " ]: " << buffer << std::endl;

        send(clientSocket, buffer, strlen(buffer), 0);
    }
    close(clientSocket);
}

int main() {
    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Bind the socket to an address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Accept connections on any IP address

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1) { // Allow up to 5 clients to queue for connection
        std::cerr << "Error listening on socket" << std::endl;
        return -1;
    }

    std::cout << "Server listening on port 12345..." << std::endl;


    std::vector<std::thread> threads;
    while(true) {
        // Accept incoming connections
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
        if (clientSocket == -1) {
            std::cerr << "Error accepting client connection" << std::endl;
            return -1;
        }

        threads.emplace_back(handle_client, clientSocket);
    }

    // Close sockets
    close(serverSocket);

    return 0;
}
