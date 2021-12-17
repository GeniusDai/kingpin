#include <iostream>
#include <unistd.h>
#include <thread>
#include <gtest/gtest.h>
#include "kingpin/Context.h"
using namespace std;
using namespace kingpin;

class ContextFixture : public testing::TestWithParam<int> {
public:
    struct SimpleArgs {
        int a;
        int b;
        int ret;
        SimpleArgs(int a, int b) : a(a), b(b) {}
    };

    static void *simple_func(void *arg) {
        SimpleArgs *ptr = static_cast<SimpleArgs *>(arg);
        this_thread::sleep_for(chrono::milliseconds(ptr->a));
        ptr->ret = ptr->a + ptr->b;
        return NULL;
    }

    void SetUp() override {}
    void TearDown() override {}

    int _time_out = 1000;
};

TEST_P(ContextFixture, MyTest) {
    // Param : time to sleep
    int a = GetParam();
    int b = 8888;
    Context ctx(_time_out);
    SimpleArgs arg(a, 8888);
    ThrWithCtx thr(simple_func, &arg, &ctx);
    if (a < _time_out) { EXPECT_TRUE(thr.run()); EXPECT_EQ(arg.ret, a + b); }
    else { EXPECT_FALSE(thr.run()); }
}

INSTANTIATE_TEST_SUITE_P(tc, ContextFixture, testing::Values(500, 2000));

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
