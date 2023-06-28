#include "listener.h"
#define SZ 4096 // 3200       /* Size of buffer */
//#define SZ 8192       /* Size of buffer */

static const char *arr = "[\"голосовое управление\"]";

static std::mutex bufferMutex;
static std::mutex chunkMutex;
static std::condition_variable bufferCondition;
static std::condition_variable chunkCondition;

static std::atomic<bool> fullStream = false;
static bool bufferProcessed = false;

static char cbuffer[SZ] { 0 };
static VoskRecognizer *recognizer = nullptr;
static VoskModel *model = nullptr;
static std::deque<char *> chunks;

static void handle_gstreamer_message(CustomData *data, GstMessage *msg);
static GstFlowReturn new_sample(GstAppSink *appsink, gpointer);
static GstFlowReturn new_preroll(GstAppSink *, gpointer);
static void innerStream(gboolean &terminate);

Listener &Listener::instance()
{
    static Listener instance;
    return instance;
}

Listener::~Listener()
{
    /* Free resources */
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    gst_object_unref(data.pipeline);
    vosk_recognizer_free(recognizer);
    vosk_model_free(model);
}

char *Listener::getChunk()
{
    std::unique_lock lk { chunkMutex };
    chunkCondition.wait(lk, [this] { return !chunks.empty() || data.terminate; });
    if (data.terminate) {
        return nullptr;
    }
    auto chunk = chunks.at(0);
    chunks.pop_front();
    return chunk;
}

void Listener::recognize()
{
    /* Wait until error or EOS */
    GstBus *bus = gst_element_get_bus(data.pipeline);
    int final = 0;
    do {
        GstMessage *msg = gst_bus_pop(bus);

        /* Parse message */
        if (msg != NULL) {
            handle_gstreamer_message(&data, msg);
        }

        if (!fullStream) {
            {
                std::unique_lock lk { bufferMutex };
                bufferCondition.wait(lk, [] { return bufferProcessed; });
                final = vosk_recognizer_accept_waveform(recognizer, cbuffer, SZ);
                bufferProcessed = false;
            }

            if (final) {
                const char *ch = vosk_recognizer_result(recognizer);
                auto token = json::parse(ch);
                std::cout << "token: " << token["text"] << '\n';

                if (token["text"] == "голосовое управление") {
                    std::cout << "Phrase caught\n";
                    vosk_recognizer_free(recognizer);
                    recognizer = nullptr;
                    recognizer = vosk_recognizer_new(model, 8000.0);
                    fullStream = true;
                    auto innerThread =
                        std::thread(innerStream, std::ref(data.terminate));
                    innerThread.join();
                    std::cout << "terminated? " << data.terminate;
                }
            }
        }
    } while (!data.terminate);

    gst_object_unref(bus);
    chunkCondition.notify_all();
}

static void innerStream(gboolean &terminate)
{
    int final = 0;
    while (fullStream) {
        {
            std::unique_lock lk { bufferMutex };
            bufferCondition.wait(lk, [] { return bufferProcessed; });
            final = vosk_recognizer_accept_waveform(recognizer, cbuffer, SZ);
            bufferProcessed = false;
        }

        if (final) {
            const char *ch = vosk_recognizer_result(recognizer);
            auto token = json::parse(ch)["text"].get<std::string>();

            if (strstr(token.c_str(), "выключ") && strstr(token.c_str(), "управл")) {
                fullStream = false;
            } else if (strstr(token.c_str(), "заверш") && strstr(token.c_str(), "управл")) {
                fullStream = false;
                terminate = TRUE;
                return;
            } else {
                {
                    std::unique_lock { chunkMutex };
                    char *p = new char(token.size() + 1u);
                    std::copy(token.data(), token.data() + token.size() + 1u,
                          &p[0]);
                    chunks.push_back(p);
                }
                chunkCondition.notify_all();
            }
        }
    }
    vosk_recognizer_free(recognizer);
    recognizer = nullptr;
    recognizer = vosk_recognizer_new_grm(model, 8000.0, arr);
}

void Listener::init(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    model = vosk_model_new("model");
    recognizer = vosk_recognizer_new_grm(model, 8000.0, arr);

    //    CustomData data;
    data.duration = GST_CLOCK_TIME_NONE;
    data.terminate = FALSE;

    /* Create the elements */
    data.source = gst_element_factory_make("pulsesrc", "psrc");
    data.fsink = gst_element_factory_make("filesink", "fsink");
    data.asink = gst_element_factory_make("appsink", "asink");
    data.audioconvert = gst_element_factory_make("audioconvert", "conv");
    data.wavenc = gst_element_factory_make("wavenc", "enc");
    data.capsfilter = gst_element_factory_make("capsfilter", "cap");
    data.pipeline = gst_pipeline_new("test-pipeline");

    if (!data.pipeline || !data.source || !data.fsink || !data.audioconvert || !data.wavenc
        || !data.capsfilter || !data.asink) {
        g_printerr("Not all elements could be created.\n");
        return;
    }

    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.capsfilter, data.asink,
             data.audioconvert, data.wavenc, NULL);

    if (gst_element_link_many(data.source, data.audioconvert, data.capsfilter, data.wavenc,
                  data.asink, NULL)
        != TRUE) {
        g_printerr("Elements src could not be linked.\n");
        gst_object_unref(data.pipeline);
        return;
    }

    GstCaps *caps = gst_caps_from_string("audio/x-raw,format=S16LE,channels=1,rate=8000");
    g_object_set(data.capsfilter, "caps", caps, NULL);
    g_object_set(data.source, "blocksize", SZ, NULL);
    g_object_set(data.asink, "blocksize", SZ, NULL);

    gst_app_sink_set_emit_signals((GstAppSink *)data.asink, TRUE);
    gst_app_sink_set_drop((GstAppSink *)data.asink, TRUE);
    gst_app_sink_set_max_buffers((GstAppSink *)data.asink, 1);
    GstAppSinkCallbacks callbacks = { NULL, new_preroll, new_sample, { NULL } };
    gst_app_sink_set_callbacks(GST_APP_SINK(data.asink), &callbacks, NULL, NULL);

    GstStateChangeReturn ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(data.pipeline);
        return;
    }
}

bool Listener::terminated() const
{
    return data.terminate;
}

static GstFlowReturn new_preroll(GstAppSink *, gpointer)
{
    g_print("Got preroll!\n");
    return GST_FLOW_OK;
}

static GstFlowReturn new_sample(GstAppSink *appsink, gpointer)
{
    static std::atomic<int> framecount = 0;
    framecount++;
    {
        std::unique_lock { bufferMutex };
        GstSample *sample = gst_app_sink_pull_sample(appsink);
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;
        gst_buffer_map(buffer, &map, GST_MAP_READ);
        memcpy(cbuffer, map.data, (size_t)map.size);
        gst_buffer_unmap(buffer, &map);

        gst_sample_unref(sample);
        bufferProcessed = true;
    }
    bufferCondition.notify_all();

    // print dot every 30 frames just for sure that we do not stack
    if (framecount % 30 == 0) {
        g_print(".");
    }

    return GST_FLOW_OK;
}

static void handle_gstreamer_message(CustomData *data, GstMessage *msg)
{
    g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(msg));
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(msg, &err, &debug);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);
            data->terminate = TRUE;
            break;
        }
        case GST_MESSAGE_EOS:
            g_print("End-Of-Stream reached.\n");
            data->terminate = TRUE;
            /* end-of-stream */
            break;
        default:
            /* unhandled message */
            break;
    }
    gst_message_unref(msg);
}
