#include <gtest/gtest.h>
#include "kingpin/Utils.h"

using namespace kingpin;

TEST(FUNC_split, positive) {
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
