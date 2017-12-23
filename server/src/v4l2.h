#ifndef V4L2_H
#define V4L2_H

#include <stdint.h>
#include "timing.h"

class V4L2
{
public:
    V4L2();

    bool startup(class RpiServer& in_server);
    void shutdown();
    void update();
    
    bool is_capturing();
    void set_capturing(bool in_enabled);
    
private:

    struct HWBuffer
    {
        void* ptr;
        uint32_t sz;
    };

    enum
    {
        DEV_BUFFER_CNT = 2,
    };

    int m_fd;

    int m_dev_buf_idx;
    HWBuffer m_dev_buf[DEV_BUFFER_CNT];
    bool m_capture;
    TimePt m_last_time;
    long m_frame_time;

    void init_commands(void* in_script_ctx);
    bool capture();
    static int xioctl(int fh, int request, void *arg);
};

#endif // V4L2_H