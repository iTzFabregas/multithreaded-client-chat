#include "defines.hpp"

std::vector<int> client_sockets;

void handle_input(int serverSocket) {
    char buffer[1024];
    std::cout << "Digite 'EXIT' para fechar o server" << std::endl;
    std::cin.getline(buffer, 1024);

	if (strcmp("EXIT", buffer) == 0){
        for (int client : client_sockets) close(client);
	    close(serverSocket); 
	    exit(0);
	}
}

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
        if (bytesRead <= 0 || !strcmp(buffer, "exit")) {
            std::cerr << "Connection closed by the client [ " << client_name << " ]" << std::endl;
            break;
        }
        buffer[bytesRead] = '\0';
        char data[1024];
        sprintf(data, "[ %s ]: %s\n", client_name, buffer);

        std::cout << data;

        // Send the data to all users
        for (int client : client_sockets) {
            if (client != clientSocket) send(client, data, strlen(data), 0);
            else {
                if (!strcmp(buffer, "exit")) send(client, data, strlen(data), 0);
            }
        }
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
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CONNECTIONS) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return -1;
    }
    std::cout << "Server listening on port " << PORT << " ..." << std::endl;

    std::thread inputThread(handle_input, serverSocket);

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

        // Save the client socket and create a thread for this client
        client_sockets.push_back(clientSocket);
        threads.emplace_back(handle_client, clientSocket);
    }

    // Close sockets
    close(serverSocket);

    return 0;
}
