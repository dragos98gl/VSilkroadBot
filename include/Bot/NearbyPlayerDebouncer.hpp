#pragma once
#include <functional>
#include <chrono>

class NearbyPlayerDebouncer
{
public:
    using Callback = std::function<void()>;

    NearbyPlayerDebouncer(int confirmThreshold = 5,
                     int clearThreshold   = 5,
                     double repeatAfterSec = 10.0);

    void setOnDetected(Callback cb);
    void setOnCleared(Callback cb);

    void update(bool detected);

    bool isActive() const;

private:
    void fire(const Callback& cb);

    Callback m_onDetected;
    Callback m_onCleared;

    int    m_confirmThreshold;
    int    m_clearThreshold;
    double m_repeatAfterSec;

    int  m_detectCount = 0;
    int  m_clearCount  = 0;
    bool m_isActive    = false;

    std::chrono::steady_clock::time_point m_lastFired;
};