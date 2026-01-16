#pragma once

#include <Arduino.h>
#include <Client.h>

class NetworkClient_ts
{
public:
  explicit NetworkClient_ts(Client& client);

  bool connect(const char* host, uint16_t port);
  size_t send(const uint8_t* data, size_t length);

private:
  Client& client;
};
