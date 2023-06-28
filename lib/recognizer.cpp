#include "listener.h"

namespace recognizer {
void init(int argc, char *argv[])
{
    Listener::instance().init(argc, argv);
}
void recognize()
{
    Listener::instance().recognize();
}
bool terminated()
{
    return Listener::instance().terminated();
}
char *getChunk()
{
    return Listener::instance().getChunk();
}
} // namespace recognizer
