#include "NetworkClient_ts.h"

NetworkClient_ts::NetworkClient_ts(Client& client) : client(client)
{
}

bool NetworkClient_ts::connect(const char* host, uint16_t port)
{
  return client.connect(host, port);
}

size_t NetworkClient_ts::send(const uint8_t* data, size_t length)
{
  return client.write(data, length);
}
