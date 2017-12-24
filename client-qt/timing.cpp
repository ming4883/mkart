#include "timing.h"


float delta_time_in_ms(TimePt& inout_last_time)
{
    TimePt now = Clock::now();
    float delta_t = (float)std::chrono::duration_cast<std::chrono::milliseconds>(now - inout_last_time).count();
    inout_last_time = now;

    return delta_t;
}

bool should_trigger_event(TimePt& inout_last_time, float in_interval_in_ms)
{
    TimePt now = Clock::now();
    float delta_t = (float)std::chrono::duration_cast<std::chrono::milliseconds>(now - inout_last_time).count();


    if (delta_t > in_interval_in_ms)
    {
        inout_last_time = now;
        return true;
    }

    return false;
}

float current_time_in_ms()
{
    static TimePt start = Clock::now();
    TimePt now = Clock::now();
    return (float)std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}
