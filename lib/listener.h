#include <gst/app/app.h>
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <vosk_api.h>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <iostream>

typedef struct _CustomData
{
    GstElement *pipeline, *source, *fsink, *audioconvert, *wavenc, *capsfilter, *asink;
    gboolean playing; /* Are we in the PLAYING state? */
    gboolean terminate; /* Should we terminate execution? */
    gint64 duration;
} CustomData;


class Listener
{
public:
    char *getChunk();
    void recognize();
    void init(int argc, char *argv[]);
    bool active() const;
    static Listener &instance();

private:
    Listener() = default;
    CustomData data;
public:
    Listener(Listener const &) = delete;
    void operator=(Listener const &) = delete;
};
