#ifndef DEBUGGING_H
#define DEBUGGING_H

#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL 0
#endif

#ifdef DEBUG_BUILD
  #define D_PRINT(x) Serial.print(x); Serial.flush()
  #define D_PRINTLN(x) Serial.println(x); Serial.flush();
  #define D_MSG(n, x) if(n > DEBUG_LEVEL) {for(int asdasdasdasd = 0; asdasdasdasd < n; ++asdasdasdasd) {Serial.print('\t');} Serial.println(x); Serial.flush();}
  #define D_INDENT(n) for(int asdasdasdasd = 0; asdasdasdasd < n; ++asdasdasdasd) {Serial.print('\t');}
#else
  #define D_PRINT(x) do {} while (0)
  #define D_PRINTLN(x) do {} while (0)
  #define D_MSG(n, x) do {} while (0)
  #define D_INDENT(n) do {} while (0)
#endif

#endif // DEBUGGING_H
