#ifndef RECOGNIZER_H
#define RECOGNIZER_H

namespace recognizer {
void init(int argc, char *argv[]);
void recognize();
bool terminated();
char* getChunk();
} // namespace recognizer

#endif // RECOGNIZER
