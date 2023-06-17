#include <recognizer.h>
#include <thread>
#include <iostream>

void printChunks()
{
    while (true) {
        auto chunk = recognizer::getChunk();
        if (chunk) {
            std::cout << "Hello from chunk: " << chunk << '\n';
        }
    }
}

int main(int argc, char *argv[])
{
    std::thread recognizeThread(recognizer::recognize, argc, argv);
    std::thread printThread(printChunks);

    recognizeThread.join();
    printThread.join();

    return 0;
}
