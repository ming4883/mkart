#ifndef TIMING_H
#define TIMING_H


#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point TimePt;

float delta_time_in_ms(TimePt& inout_last_time);

bool should_trigger_event(TimePt& inout_last_time, float in_interval_in_ms);

float current_time_in_ms();


#endif // TIMING_H
