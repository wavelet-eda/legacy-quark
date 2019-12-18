#pragma once

#include <functional>
#include <stdint.h>
#include <iostream>
#include <exception>
#include <vector>
#include <set>
// #include <gmp.h>
#include <gmpxx.h>

#define likely(x)      __builtin_expect(!!(x), 1) 
#define unlikely(x)    __builtin_expect(!!(x), 0) 

