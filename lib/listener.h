#include <gst/app/app.h>
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <vosk_api.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <deque>
#include <atomic>
#include <condition_variable>
#include <iostream>

using json = nlohmann::json;

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
    bool terminated() const;
    static Listener &instance();

private:
    Listener() = default;
    ~Listener();
    CustomData data;
public:
    Listener(Listener const &) = delete;
    void operator=(Listener const &) = delete;
};
