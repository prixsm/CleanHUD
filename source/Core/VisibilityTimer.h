#pragma once

#include <cstdint>

class VisibilityTimer
{
public:

    void ShowFor(uint32_t milliseconds);

    bool Visible() const;

    void Reset();

    void Show();

    void Hide();

private:
    uint32_t mVisibleUntil = 0;
};