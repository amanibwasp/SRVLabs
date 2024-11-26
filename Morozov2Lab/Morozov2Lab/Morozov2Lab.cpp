#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>

std::mutex m;

void Func(std::string name)
{
    long double i = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (true)
    {
        i += 1e-9;
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - start_time).count();
        if (elapsed_time >= 1000)
            break;
    }

    m.lock();
    std::cout << name << ": " << i << std::endl;
    m.unlock();
}

int main()
{
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");

    thread1.join();
    thread2.join();
    thread3.join();

    system("pause");
}
