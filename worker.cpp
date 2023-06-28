#include "worker.h"

static void printChunks()
{
    while (!recognizer::terminated()) {
        auto chunk = recognizer::getChunk();
        if (chunk) {
            std::cout << "Hello from chunk: " << chunk << '\n';
            delete chunk;
        }
    }
}

void Worker::init(int argc, char *argv[])
{
    recognizer::init(argc, argv);
}

void Worker::start()
{
    std::thread recognizeThread(recognizer::recognize);
    std::thread printThread(printChunks);

    recognizeThread.join();
    printThread.join();
}
