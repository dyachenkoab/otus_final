#include "listener.h"

namespace recognizer {
char *getChunk()
{
    return Listener::instance().getChunk();
}
void recognize(int argc, char *argv[])
{
    Listener::instance().recognize(argc, argv);
}
} // namespace recognizer
