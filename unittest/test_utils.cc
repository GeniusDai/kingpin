#include <gtest/gtest.h>
#include <mutex>
#include <memory>
#include <condition_variable>
#include "kingpin/AsyncLogger.h"
#include "kingpin/Utils.h"
#include "kingpin/Exception.h"

using namespace kingpin;

TEST(FUNC_split, test) {
    vector<pair<string, string> > args = {
        {"hello, kingpin", ","},
        {"hello, kingpin", ", "},
        {"hello, kingpin", "    "}
    };
    vector<vector<string> > ans = {
        {"hello", " kingpin"},
        {"hello", "kingpin"},
        {"hello, kingpin"}
    };
    for (uint i = 0; i < ans.size(); ++i) {
        vector<string> res;
        split(args[i].first, args[i].second, res);
        EXPECT_EQ(res, ans[i]);
    }
}

class UtilsTest : public testing::Test {
protected:
    static condition_variable _cv_listen;
    static condition_variable _cv_conn;
    static mutex _m;
    static const int _port;
    static bool _end;
    static bool _listen_ready;
    static unique_ptr<thread>_t;

    static void _run_listen() {
        initListen(_port, 1);
        unique_lock<mutex> lk(_m);
        _listen_ready = true;
        _cv_conn.notify_all();
        _cv_listen.wait(lk, []()->bool { return _end; } );
    }

    static void _run_connect(int n) {
        {
            unique_lock<mutex> lk(_m);
            _cv_conn.wait(lk, []()->bool { return _listen_ready; } );
        }
        int sock;
        int timeout = 1;
        try {
            if (n % 2) { sock = connectHost("localhost", _port, timeout); }
            else { sock = connectIp("localhost", _port, timeout); }
        } catch (const TimeoutException &e) {
            return;
        }
        const char *str = "hello, kingpin";
        if (::write(sock, str, ::strlen(str)) < 0) {
            fatalError("syscall write error");
        }
        INFO << "write complete" << END;
    }

    static void SetUpTestSuite() {
        _t = unique_ptr<thread>(new thread(UtilsTest::_run_listen));
    }

    static void TearDownTestSuite() {
        {
            unique_lock<mutex> lk(_m);
            _end = true;
        }
        _cv_listen.notify_one();
        _t->join();
    }
};

const int UtilsTest::_port = 9000;
condition_variable UtilsTest::_cv_listen;
condition_variable UtilsTest::_cv_conn;
mutex UtilsTest::_m;
bool UtilsTest::_end = false;
bool UtilsTest::_listen_ready = false;
unique_ptr<thread> UtilsTest::_t;

TEST_F(UtilsTest, test_connect) {
    vector<thread> vt;
    for (uint i = 0; i < 30; ++i) {
        vt.emplace_back(_run_connect, i);
    }
    for (uint i = 0; i < vt.size(); ++i) {
        vt[i].join();
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
