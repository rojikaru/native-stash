#ifndef CROSS_PLATFORM_LIBS_H
#define CROSS_PLATFORM_LIBS_H

// CROSS-PLATFORM HEADERS

#include "conditions.h"

// PLATFORM SPECIFIC HEADERS

#if win_defined
#include "../windows/libs.h"
#elif linux_defined
#include "../linux/libs.h"
#endif

// PLATFORM INDEPENDENT HEADERS

#include <iostream>

// FUNCTIONS

void hello();

#endif //CROSS_PLATFORM_LIBS_H
