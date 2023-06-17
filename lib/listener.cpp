#include "listener.h"
#define SZ 4096 // 3200       /* Size of buffer */
//#define SZ 8192       /* Size of buffer */

static const char *arr = "[\"голосовое управление\"]";

static std::mutex bufferMutex;
static std::mutex chunkMutex;
static std::condition_variable bufferCondition;
static std::condition_variable chunkCondition;

static bool fullStream = false;
static bool bufferProcessed = false;

static char cbuffer[SZ] { 0 };
static VoskRecognizer *recognizer = nullptr;
static VoskModel *model = nullptr;
static std::deque<char *> chunks;

static void handle_gstreamer_message(CustomData *data, GstMessage *msg);
static GstFlowReturn new_sample(GstAppSink *appsink, gpointer);
static GstFlowReturn new_preroll(GstAppSink *, gpointer);
static void innerStream(VoskRecognizer *phraseRecognizer, gboolean &terminate);
static char *strTok(const char *str, const char *razd);
static char *tokenizer(const char *str);

Listener &Listener::instance()
{
    static Listener instance;
    return instance;
}

char *Listener::getChunk()
{
    std::unique_lock lk { chunkMutex };
    chunkCondition.wait(lk, [] { return !chunks.empty(); });
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
                char *ch = const_cast<char *>(vosk_recognizer_result(recognizer));
                auto token = tokenizer(ch);
                std::cout << "token: " << token << '\n';

                if (!strcmp(token, "голосовое управление")) {
                    std::cout << "Phrase caught\n";
                    vosk_recognizer_free(recognizer);
                    fullStream = true;
                    recognizer = nullptr;
                    recognizer = vosk_recognizer_new(model, 8000.0);
                    auto innerThread = std::thread(innerStream, recognizer,
                                       std::ref(data.terminate));
                    innerThread.join();
                }
            }
        }
    } while (!data.terminate);

    /* Free resources */
    gst_object_unref(bus);
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    gst_object_unref(data.pipeline);
    vosk_recognizer_free(recognizer);
    vosk_model_free(model);
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

bool Listener::active() const
{
    return data.terminate;
}

static void innerStream(VoskRecognizer *phraseRecognizer, gboolean &terminate)
{
    int final = 0;
    while (fullStream) {
        {
            std::unique_lock lk { bufferMutex };
            bufferCondition.wait(lk, [] { return bufferProcessed; });
            final = vosk_recognizer_accept_waveform(phraseRecognizer, cbuffer, SZ);
            bufferProcessed = false;
        }

        if (final) {
            const char *ch = vosk_recognizer_result(phraseRecognizer);
            auto token = tokenizer(ch);
            std::cout << "token: " << token << '\n';

            if (strstr(ch, "заверш") && strstr(ch, "управл")) {
                terminate = TRUE;
                fullStream = false;
            } else if (strstr(ch, "выключ") && strstr(ch, "управл")) {
                std::cout << "GetBack!\n";
                fullStream = false;
            } else {
                {
                    std::unique_lock { chunkMutex };
                    chunks.push_back(token);
                }
                chunkCondition.notify_all();
            }
        }
    }
    vosk_recognizer_free(recognizer);
    recognizer = nullptr;
    recognizer = vosk_recognizer_new_grm(model, 8000.0, arr);
}

static GstFlowReturn new_preroll(GstAppSink *, gpointer)
{
    g_print("Got preroll!\n");
    return GST_FLOW_OK;
}

static GstFlowReturn new_sample(GstAppSink *appsink, gpointer)
{
    static int framecount = 0;
    framecount++;
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    GstBuffer *buffer = gst_sample_get_buffer(sample);

    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    {
        std::unique_lock { bufferMutex };
        memcpy(cbuffer, map.data, (size_t)map.size);
        bufferProcessed = true;
    }
    bufferCondition.notify_all();
    gst_buffer_unmap(buffer, &map);

    // print dot every 30 frames
    if (framecount % 30 == 0) {
        g_print(".");
    }

    gst_sample_unref(sample);
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

static char *tokenizer(const char *str)
{
    auto startWord = const_cast<char *>(strrchr(str, ':'));
    startWord = startWord + 2 * sizeof(char);
    return strTok(startWord, "\"");
}

static char *strTok(const char *str, const char *razd)
{
    static char *p = nullptr, *p1 = nullptr, *p2 = nullptr;
    char *p3 = nullptr;
    int state = 1;
    int len = 0;

    auto isRazd = [&](char sym, const char *razd) {
        for (int i = 0; razd[i]; i++)
            if (razd[i] == sym)
                return true;
        return false;
    };

    if (str) {
        for (len = 0; str[len]; len++)
            ;
        p = (char *)malloc((len + 1) * sizeof(char));
        for (int k = 0; (p[k] = str[k]); k++)
            ;
        p1 = p2 = p;
    }

    while (true) {
        if (p == nullptr)
            return nullptr;
        switch (state) {
            case 1:
                if (*p2 == '\0') {
                    free(p);
                    p = nullptr;
                    return nullptr;
                }
                if (isRazd(*p2, razd)) {
                    p1 = ++p2;
                    break;
                }
                state = 2;
                [[fallthrough]];
            case 2:
                if (*p2 == '\0' || isRazd(*p2, razd)) {
                    p3 = p1;
                    if (*p2 == '\0')
                        return p3;
                    *p2 = '\0';
                    p1 = ++p2;
                    return p3;
                }
                p2++;
        }
    }
}
