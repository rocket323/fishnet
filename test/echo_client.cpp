#include <stdio.h>
#include "event_loop.h"

void hello()
{
    printf("hello world\n");
}

int main()
{
    auto loop = EventLoop::Current();
    loop->RunPeriodic(1000, hello);

    loop->Loop();
    return 0;
}