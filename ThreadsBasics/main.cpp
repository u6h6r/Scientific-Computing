#include "threads.h"
#include <thread>
#include <string>
#include <chrono>
#include <mutex>
#include <array>


void task_1()
{
    std::thread global_thread (t, "Global Function Thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread function_object (FunctionObject(), "Function Object Thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread class_method (&ClassMethod::start, ClassMethod(), "Class Method Thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread lambda_function ([=](std::string str)
    {
        t(str);
    }, "Lambda Function Thread");
    global_thread.join();
    function_object.join();
    class_method.join();
    lambda_function.join();
}

void task_2()
{   
    std::array<std::thread, 20> t_array;
    for (int i = 0; i < 20; ++i)
        t_array[i] = std::thread (t_50, "printing: ", i);
    for (auto& thread : t_array)
        thread.join();
}

void task_3()
{
    single_incerement();
    multi_increment(unsync_increment, "g_unsync");
    multi_increment(mutex_increment, "g_mutex");
    multi_increment(atomic_increment, "g_atomic");
}

int main()
{
    task_1();
    task_2();
    task_3();
    return 0;
}