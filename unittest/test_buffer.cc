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
#include "kingpin/Buffer.h"

using namespace std;
using namespace kingpin;

class BufferFixture : public testing::Test {
protected:
     const char *_file = "./file_for_test";
     const char *_str = "Kingpin is a high performance network library!";
     void SetUp() override {
         int fd = open(this->_file, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
         write(fd, _str, strlen(_str));
         fsync(fd);
         close(fd);
     }

     void TearDown() override {
        remove(this->_file);
     }
};

TEST_F(BufferFixture, test_read1) {
    Buffer buffer;
    int fd = open(this->_file, O_RDONLY);
    buffer.readNioToBufferTillEnd(fd, "high", 1);
    EXPECT_FALSE(strcmp(buffer._buffer, "Kingpin is a high"));
    EXPECT_EQ(buffer.readNioToBuffer(fd, 2), 2);
    EXPECT_FALSE(strcmp(buffer._buffer, "Kingpin is a high p"));
    buffer.readNioToBufferTillEnd(fd, "k ", 1);
    EXPECT_EQ(buffer.readNioToBufferTillBlockNoExp(fd), 8);
    EXPECT_FALSE(strcmp(buffer._buffer, this->_str));
    close(fd);
}

TEST_F(BufferFixture, test_write1) {
    Buffer buffer;
    int fd = open(this->_file, O_RDWR);
    const char *str = "Hello, C++!";
    for (int i = 0; i < 10; ++i) { buffer.appendToBuffer(str); }
    EXPECT_EQ(buffer._start, 0);
    EXPECT_EQ(buffer._offset, 10 * strlen(str));
    EXPECT_EQ(buffer.writeNioFromBufferTillBlock(fd), 10 * strlen(str));
    EXPECT_EQ(fsync(fd), 0);
    close(fd);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}