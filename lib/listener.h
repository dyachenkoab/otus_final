#include <gst/app/app.h>
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <vosk_api.h>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <iostream>

class Listener
{
public:
    char *getChunk();
    void recognize(int argc, char *argv[]);
    static Listener &instance();

private:
    Listener() { }

public:
    Listener(Listener const &) = delete;
    void operator=(Listener const &) = delete;
};
