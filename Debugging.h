#ifndef DEBUGGING_H
#define DEBUGGING_H
inline void debug(char* message) {
#ifdef DEBUG
  Serial.print("\t->"); Serial.println(message); Serial.flush();
#endif
}
#endif // DEBUGGING_H
