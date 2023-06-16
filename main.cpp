#include "recognizer.c"

void printChunks()
{
    while (true) {
        auto chunk = getChunk();
        if (chunk) {
            std::cout << "Hello from chunk: " << chunk << '\n';
        }
    }
}

int main(int argc, char *argv[])
{
    std::thread recognizeThread(recognize, argc, argv);
    std::thread printThread(printChunks);

    recognizeThread.join();
    printThread.join();

    return 0;
}
