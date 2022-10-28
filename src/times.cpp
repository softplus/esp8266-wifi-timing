
#include <Arduino.h>
#include "main.h"
#include "times.h"

void times_display(const char *name, uint32_t micro_count) {
    DEBUG_OUTS("<"); 
    DEBUG_OUTS(name); DEBUG_OUTS("=");
    DEBUG_OUTS(micro_count); 
    DEBUG_OUT(">");
}
