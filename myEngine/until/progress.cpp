#include "progress.hpp"
#include <iostream>
#include <algorithm>

ProgressBar::ProgressBar(size_t total, size_t step)
    : mTotal(total), mStep((int)step), mCurrent(0), mPercent(0), mLastPercent(0) {}

void ProgressBar::update(size_t count)
{
    Guard guard(mSpinLock);
    mCurrent += count;
    mPercent = static_cast<int>(100 * static_cast<float>(mCurrent) / static_cast<float>(mTotal));
    if ((mPercent - mLastPercent >= mStep) || (mPercent == 100))
    {
        mLastPercent = mPercent;
        std::cout << mPercent << "%" << std::endl;
    }
}