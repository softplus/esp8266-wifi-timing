
#ifndef TIMES_H
#define TIMES_H

#define TIME_START(timer_id) uint32_t timer_id = micros()
#define TIME_STOP(timer_id, timer_name_string) times_display(timer_name_string, micros()-timer_id)

void times_display(const char *name, uint32_t micro_count);

#endif

