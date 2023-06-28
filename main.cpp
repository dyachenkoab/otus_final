#include "worker.h"

int main(int argc, char *argv[])
{
    Worker worker;
    worker.init(argc, argv);
    worker.start();

    return 0;
}
