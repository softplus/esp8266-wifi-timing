
#ifndef MAIN_H
#define MAIN_H

#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DEBUG_OUT(x) {Serial.println(x);}
#define DEBUG_OUTS(x) {Serial.print(x);}
#else
#define DEBUG_OUT(x) {}
#define DEBUG_OUTS(x) {}
#endif

#endif
