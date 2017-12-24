#ifndef REMOTE_SERVICE_H
#define REMOTE_SERVICE_H
#include "tcp.h"

class Cmd;

class RemoteService
{
public:

    static void init();

    static void update();

    static void shutdown();

    static Tcp::State get_state();

    static Tcp::Result disconnect();

    static Tcp::Result connect(unsigned char in_ip[4], int in_port);

    static Tcp::Result send_cmd(const Cmd& in_cmd);
};


#endif // REMOTE_SERVICE_H
