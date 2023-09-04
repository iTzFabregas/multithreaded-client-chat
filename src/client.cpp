#include "defines.hpp"

std::atomic<bool> stopThread(false);

void sendData(int clientSocket) {
    char buffer[1024];
    std::cout << "Enter a message ('exit' to close the connection): " << std::endl;
    while (true) {
        std::cin.getline(buffer, sizeof(buffer));
        send(clientSocket, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            break;
        }
    }
    return;
}

void recieveData(int clientSocket) {
    char buffer[1024];
    while (!stopThread) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Connection closed by server" << std::endl;
            break;
        }
        buffer[bytesRead] = '\0';
        std::cout << buffer;
    }
}

int main() {
    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Connect to the server
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // Port number
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr); // Server IP address

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        return -1;
    }
    std::cout << "Connected to server" << std::endl;

    // Send the name
    char buffer[1024];
    std::cout << "Enter your name: ";
    std::cin.getline(buffer, sizeof(buffer));

    send(clientSocket, buffer, strlen(buffer), 0);

    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Connection closed by server" << std::endl;
        close(clientSocket);
        return -1;
    }

    // Send and receive data
    std::thread sendThread(sendData, clientSocket);
    std::thread receiveThread(recieveData, clientSocket);

    // Wait the thread to end
    sendThread.join();
    stopThread.store(true, std::memory_order_release);
    receiveThread.join();

    // Close the socket
    close(clientSocket);

    return 0;
}
