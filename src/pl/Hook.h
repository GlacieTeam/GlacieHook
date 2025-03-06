#pragma once

#include <set>

namespace pl::hook {

typedef void* FuncPtr;

enum Priority : int {
    PriorityHighest = 0,
    PriorityHigh    = 100,
    PriorityNormal  = 200,
    PriorityLow     = 300,
    PriorityLowest  = 400,
};

int pl_hook(FuncPtr target, FuncPtr detour, FuncPtr* originalFunc, Priority priority);

bool pl_unhook(FuncPtr target, FuncPtr detour);

} // namespace pl::hook