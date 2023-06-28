#ifndef WORKER_H
#define WORKER_H
#include <thread>
#include <iostream>
#include <recognizer.h>

class Worker
{
public:
    Worker() = default;
    ~Worker() = default;

    void init(int argc, char *argv[]);
    void start();
};

#endif // WORKER_H
