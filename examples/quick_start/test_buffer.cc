#include <iostream>
#include <vector>
#include <memory>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <ctime>

#include "kingpin/Logger.h"

using namespace std;
using namespace kingpin;

int main() {
    Buffer buffer;
    buffer.appendToBuffer("hello\n");
    cout << buffer.endsWith("o\n") << endl;
    cout << buffer.endsWith("\n") << endl;
    cout << buffer.endsWith("\t") << endl;
    return 0;
}