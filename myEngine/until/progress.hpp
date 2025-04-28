#pragma once

#include "../../application/spinLock.hpp"

class ProgressBar
{
public:
    ProgressBar(size_t total, size_t step = 1);

    void update(size_t count);

private:
    size_t mTotal, mCurrent;
    int mPercent, mLastPercent, mStep;
    SpinLock mSpinLock;
};