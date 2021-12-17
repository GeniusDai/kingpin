#include <iostream>
#include <vector>
#include <memory>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <ctime>
#include <fcntl.h>
#include <functional>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "kingpin/Buffer.h"

using namespace std;
using namespace kingpin;

class BufferTest : public testing::Test {
protected:
    const char *_file = "./file_for_test";
    const char *_str = "Kingpin is a high performance network library!";
    void SetUp() override {
        int fd = open(this->_file, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
        EXPECT_GT(fd, 2);
        EXPECT_EQ(write(fd, _str, strlen(_str)), strlen(_str));
        EXPECT_EQ(fsync(fd), 0);
        EXPECT_EQ(close(fd), 0);
    }

    void TearDown() override {
        EXPECT_EQ(remove(this->_file), 0);
    }
};

TEST_F(BufferTest, test_read) {
    Buffer buffer;
    int fd = open(this->_file, O_RDONLY);
    EXPECT_GT(fd, 2);
    int pos = buffer.readNioToBufferTillEnd(fd, "high", 5);
    EXPECT_THAT(buffer._buffer + pos, testing::HasSubstr("high"));
    buffer.readNioToBufferTillEnd(fd, "k ", 1);
    EXPECT_EQ(buffer.readNioToBufferTillBlockOrEOF(fd), 8);
    EXPECT_EQ(strcmp(buffer._buffer, this->_str), 0);
    EXPECT_EQ(close(fd), 0);
}

TEST_F(BufferTest, test_write) {
    Buffer buffer;
    int fd = open(this->_file, O_RDWR);
    EXPECT_GT(fd, 2);
    const char *str = "Hello, C++!";
    for (int i = 0; i < 10; ++i) { buffer.appendToBuffer(str); }
    EXPECT_EQ(buffer._start, 0);
    EXPECT_EQ(buffer._offset, 10 * strlen(str));
    EXPECT_EQ(buffer.writeNioFromBufferTillBlock(fd), 10 * strlen(str));
    EXPECT_EQ(fsync(fd), 0);
    EXPECT_EQ(close(fd), 0);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}