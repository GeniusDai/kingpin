#ifndef _UTILS_H_1_
#define _UTILS_H_1_

#include "kingpin/core/Exception.h"

void fatalError(const char *str) {
    perror(str);
    throw FatalException(str);
}

void nonFatalError(const char *str) {
    perror(str);
    throw NonFatalException(str);
}

#endif