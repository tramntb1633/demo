#include <iostream>
#include <winsock2.h>
#include <random>
#include <thread>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
#define PORT 2808 
using namespace std;

int generateReward() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 5);
    int rewards[] = {100, 200, 300, 0, 50, 150};
    return rewards[dis(gen)];
}

void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if (bytesReceived <= 0) {
            cerr << "Client ngat ket noi hoac loi: " << WSAGetLastError() << endl;
            break;
        }
        buffer[bytesReceived] = '\0'; 
        cout << "Du lieu nhan duoc tu client: " << buffer << endl;
        if (strcmp(buffer, "SPIN") == 0) {
            int reward = generateReward();
            string result = "RESULT:" + to_string(reward);
            send(clientSocket, result.c_str(), static_cast<int>(result.length()) + 1, 0);
        } else {
            send(clientSocket, "Da nhan du lieu", 15, 0);
        }
    }
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Ket noi WSAS that bai" << endl;
        return 1;
    }
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Khong the tao socket" << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Khong the bind socket" << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Khong the lang nghe socket" << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "Server dang chay tren cong " << PORT << endl;

    vector<thread> clientThreads;
    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Chap nhan ket noi khong thanh cong" << WSAGetLastError() << endl;
            continue;
        }
        cout << "Client moi ket noi!" << endl;
        clientThreads.emplace_back(thread(handleClient, clientSocket));
    }

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}