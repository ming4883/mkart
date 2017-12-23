#ifndef GPIO_H
#define GPIO_H

#include <pigpio.h>

class GPIO
{
public:
    bool startup(class RpiServer& in_server);
    void shutdown();

    void pin_mode_in(uint8_t in_pin);
    void pin_mode_out(uint8_t in_pin);
    bool pin_mode_clock(uint8_t in_pin, uint32_t in_freq);

private:
    void init_commands(void* in_script_ctx);
};


#endif // GPIO_H