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
bool active()
{
    return Listener::instance().active();
}
char *getChunk()
{
    return Listener::instance().getChunk();
}
} // namespace recognizer
