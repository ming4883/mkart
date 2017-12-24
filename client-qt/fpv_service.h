#ifndef FPV_SERVICE_H
#define FPV_SERVICE_H

#include "tcp.h"

class FpvService
{
public:

    static void init();

    static void update();

    static void shutdown();

    static Tcp::State get_state();
    static Tcp::Result disconnect();
    static Tcp::Result connect(unsigned char in_ip[4], int in_port);

    static bool is_capturing();
    static void set_capturing(bool in_capture);
    static void remote_exit();

    static unsigned int get_texture_id();
    static unsigned int get_texture_width();
    static unsigned int get_texture_height();
    static float get_avg_recv_time();
};


#endif // FPV_SERVICE_H
