/**
 * @file simple_serve.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>

#include "olc_net.h"

enum class CustomMsgTypes : uint32_t {
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomServer : public olc::net::server_interface<CustomMsgTypes> {
 public:
    CustomServer(uint16_t nPort)
        : olc::net::server_interface<CustomMsgTypes>(nPort) {}

 protected:
    bool OnClientConnect(
        std::shared_ptr<olc::net::connection<CustomMsgTypes>> client) override {
        olc::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerAccept;
        client->Send(msg);
        return true;
    }

    // Called when a client appears to have disconnected
    void OnClientDisconnect(
        std::shared_ptr<olc::net::connection<CustomMsgTypes>> client) override {
        std::cout << "Removing client [" << client->GetID() << "]\n";
    }

    // Called when a message arrives
    void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client,
                   olc::net::message<CustomMsgTypes>& msg) override {
        switch (msg.header.id) {
            case CustomMsgTypes::ServerPing: {
                std::cout << "[" << client->GetID() << "]: Server Ping\n";

                // Simply bounce message back to client
                client->Send(msg);
            } break;

            case CustomMsgTypes::MessageAll: {
                std::cout << "[" << client->GetID() << "]: Message All\n";

                // Construct a new message and send it to all clients
                olc::net::message<CustomMsgTypes> msg;
                msg.header.id = CustomMsgTypes::ServerMessage;
                msg << client->GetID();
                MessageAllClients(msg, client);

            } break;
        }
    }
};

int main() {
    CustomServer server(10000);
    server.Start();

    while (1) {
        server.Update(-1, true);
    }

    return 0;
}