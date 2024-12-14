#include "testing/test_cases.h"
#include "testing/test_cases.h"

#include "thread"

bool tests_running = true;

void run_test_forever(int i)
{
    while (tests_running)
    {
        test_all(get_abi(), i);

        ++i;
    }
}

int main()
{
    run_test_forever(0);

    return -1;

    std::vector<std::thread*> all_threads;

    for (int i = 0; i < 1; ++i)
    {
        std::thread* runner = new std::thread(run_test_forever,rand());

        all_threads.push_back(runner);
    }

    std::cin.get();

    tests_running = false;

    for (auto i : all_threads)
    {
        i->join();
        delete i;
    }
}