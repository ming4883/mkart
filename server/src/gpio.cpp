#include "gpio.h"

#include "rpi_server.h"
#include <dukglue/dukglue.h>

bool GPIO::startup(RpiServer& in_server)
{
    if (gpioInitialise() < 0)
    {
        return false;
    }

    init_commands(in_server.script_context());

    return true;
}

void GPIO::shutdown()
{
    gpioTerminate();
}

void GPIO::pin_mode_in(uint8_t in_pin)
{
    gpioSetMode(in_pin, PI_INPUT);
}

void GPIO::pin_mode_out(uint8_t in_pin)
{
    gpioSetMode(in_pin, PI_OUTPUT);
}

bool GPIO::pin_mode_clock(uint8_t in_pin, uint32_t in_freq)
{
    gpioSetMode(in_pin, PI_ALT0);
    return 0 == gpioHardwareClock(in_pin, in_freq);
}


void GPIO::init_commands(void* in_script_ctx)
{
    ScriptContext duk_ctx = (ScriptContext)in_script_ctx;

    struct Cmds
    {
        
        enum Constants
        {
            BANK_1 = 1,
            BANK_2 = 2,
        };

        static void pin_mode_in(int in_gpio)
        {
            gpioSetMode(in_gpio, PI_INPUT);
        }

        static void pin_mode_out(int in_gpio)
        {
            gpioSetMode(in_gpio, PI_OUTPUT);
        }

        static bool pin_mode_clock(uint8_t in_gpio, uint32_t in_freq)
        {
            gpioSetMode(in_gpio, PI_ALT0);
            return 0 == gpioHardwareClock(in_gpio, in_freq);
        }

        static void get_0_31()
        {
            uint32_t val = gpioRead_Bits_0_31();
            send_gpio_value(BANK_1, val);
        }

        static void get_32_53()
        {
            uint32_t val = gpioRead_Bits_32_53();
            send_gpio_value(BANK_2, val);
        }

        static void send_gpio_value(uint32_t in_bank, uint32_t in_values)
        {
            static std::vector<uint8_t> sent_buf;

            sent_buf.resize(16);

            {
                uint32_t* ptr = (uint32_t*)&sent_buf[0];
                *ptr = (uint32_t)12; // 4cc + bank + values
            }

            sent_buf[4] = 'G';
            sent_buf[5] = 'P';
            sent_buf[6] = 'I';
            sent_buf[7] = 'O';

            {
                uint32_t* ptr = (uint32_t*)&sent_buf[8];
                *ptr = in_bank;
            }

            {
                uint32_t* ptr = (uint32_t*)&sent_buf[12];
                *ptr = in_values;
            }

            RpiServer::get()->send(&sent_buf[0], sent_buf.size());
        }

        static void set_0_31_low(uint32_t in_bitmask)
        {
            gpioWrite_Bits_0_31_Clear(in_bitmask);
        }

        static void set_0_31_high(uint32_t in_bitmask)
        {
            gpioWrite_Bits_0_31_Set(in_bitmask);
        }

        static void set_32_53_low(uint32_t in_bitmask)
        {
            gpioWrite_Bits_32_53_Clear(in_bitmask);
        }

        static void set_32_53_high(uint32_t in_bitmask)
        {
            gpioWrite_Bits_32_53_Set(in_bitmask);
        }
    };

    dukglue_register_function(duk_ctx, Cmds::pin_mode_in, "gpio_pin_mode_in");
    dukglue_register_function(duk_ctx, Cmds::pin_mode_out, "gpio_pin_mode_out");
    dukglue_register_function(duk_ctx, Cmds::pin_mode_clock, "gpio_pin_mode_clock");
    dukglue_register_function(duk_ctx, Cmds::get_0_31, "gpio_get_0_31");
    dukglue_register_function(duk_ctx, Cmds::set_0_31_low, "gpio_set_0_31_low");
    dukglue_register_function(duk_ctx, Cmds::set_0_31_high, "gpio_set_0_31_high");

    dukglue_register_function(duk_ctx, Cmds::get_32_53, "gpio_get_32_53");
    dukglue_register_function(duk_ctx, Cmds::set_32_53_low, "gpio_set_32_53_low");
    dukglue_register_function(duk_ctx, Cmds::set_32_53_high, "gpio_set_32_53_high");
}
