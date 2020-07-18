#include <string>

#if !defined(THREADS_H)
#define THREADS_H

void t(std::string str);

struct FunctionObject
{
    void operator()(std::string str);
};
void FunctionObject::operator()(std::string str)
{
    t(str);
}

class ClassMethod
{
public:
    void start(std::string str);
};
inline void ClassMethod::start(std::string str)
{
    t(str);
}

void t_50(std::string str, int x);
void increment();
void single_incerement();
void unsync_increment();
void mutex_increment();
void atomic_increment();
void multi_increment(void (*increment_func)(), std::string str);

#endif // THREADS_H