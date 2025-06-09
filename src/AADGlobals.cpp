#include "AADNumber.h"

thread_local Tape* Number::tape = nullptr;
bool Tape::multi = false;
size_t ADNode::numAdj = 1;

