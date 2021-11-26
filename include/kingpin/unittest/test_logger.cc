#include <iostream>
#include <vector>
#include <memory>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <ctime>

#include "kingpin/Logger.h"

using namespace std;

void log() {
    int n = 2;
    while (n--) {
        /*
         * We DONOT recommend using the Logger this way -- insert some code
         * between "INFO <<" and "<< END". You should always do this together
         * since thread will hold the lock in "INFO <<" and release lock
         * in "<< END", this will cause other threads block. You could use
         * "sleep" to better understand this as the first "sleep" that has
         * been commented out will severely block the concurrency.
         */
        INFO << "Hello " << "Kingpin!";
        ERROR << "TEST" << END << "NEW " << "ERROR" << END;
        // sleep(1);
        INFO << END;

        // sleep(1);

        /*
         * This is OK, you hold the lock and use the buffer and release it
         * right after you finish.
         */
        ERROR << "ERROR LINE:\n";
        ERROR << "ERROR LINE " << 1 << "\n" << "ERROR LINE 2" << END;
    }
}


int main() {
    int n_thr = 50;
    vector<shared_ptr<thread> > tp;
    for (int i = 0; i < n_thr; ++i) {
        tp.push_back(shared_ptr<thread>(new thread(&log)));
    }
    for (int i = 0; i < n_thr; ++i) {
        tp[i]->join();
    }
    return 0;
}
