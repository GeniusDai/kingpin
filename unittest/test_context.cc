#include "kingpin/Context.h"

#include <iostream>
#include <unistd.h>
#include <thread>

using namespace std;
using namespace kingpin;

struct SimpleArgs {
    int a;
    int b;
    int ret;
    SimpleArgs(int a, int b) : a(a), b(b) {}
};

void *simple_func(void *arg) {
    SimpleArgs *ptr = static_cast<SimpleArgs *>(arg);
    cout << "Thread will sleep " << ptr->a << " milliseconds" << endl;
    cout << "Argument b is: " << ptr->b << endl;
    this_thread::sleep_for(chrono::milliseconds(ptr->a));
    ptr->ret = ptr->a + ptr->b;
    return NULL;
}

int main() {
    // set the num to 1000 or 10000, you will see the difference
    int num = 1000;
    Context ctx(num);
    cout << "Context timeout is " << num << " milliseconds" << endl;

    SimpleArgs arg(3000, 8888);
    ThrWithCtx t(simple_func, &arg, &ctx);
    if (t.run()) {
        cout << "Thread ends normally, return : " << arg.ret << endl;
    } else {
        cout << "Thread timeout and have been cancelled" << endl;
    }
    return 0;
}
