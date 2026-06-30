#include "VisibilityTimer.h"

#include <CTimer.h>

void VisibilityTimer::ShowFor(uint32_t milliseconds)
{
    mVisibleUntil =
        CTimer::m_snTimeInMilliseconds + milliseconds;
}

bool VisibilityTimer::Visible() const
{
    return
        CTimer::m_snTimeInMilliseconds < mVisibleUntil;
}

void VisibilityTimer::Reset()
{
    mVisibleUntil = 0;
}