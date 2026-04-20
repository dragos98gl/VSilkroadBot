#include "Bot/NearbyPlayerDebouncer.hpp"

NearbyPlayerDebouncer::NearbyPlayerDebouncer(int confirmThreshold,
                                             int clearThreshold,
                                             double repeatAfterSec)
        : m_confirmThreshold(confirmThreshold)
        , m_clearThreshold(clearThreshold)
        , m_repeatAfterSec(repeatAfterSec)
{
}

void NearbyPlayerDebouncer::setOnDetected(Callback cb)  
{ 
    m_onDetected = cb; 
}

void NearbyPlayerDebouncer::setOnCleared(Callback cb)   
{ 
    m_onCleared  = cb; 
}

void NearbyPlayerDebouncer::update(bool detected)
{
    if (detected)
    {
        m_clearCount = 0;
        m_detectCount++;

        if (m_detectCount >= m_confirmThreshold)
        {
            if (!m_isActive)
            {
                m_isActive   = true;
                m_lastFired  = std::chrono::steady_clock::now();
                fire(m_onDetected);
                return;
            }

            auto now     = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - m_lastFired).count();
            if (elapsed >= m_repeatAfterSec)
            {
                m_lastFired = now;
                fire(m_onDetected);
            }
        }
    }
    else
    {
        m_detectCount = 0;
        if (m_isActive)
        {
            m_clearCount++;
            if (m_clearCount >= m_clearThreshold)
            {
                m_isActive    = false;
                m_clearCount  = 0;
                fire(m_onCleared);
            }
        }
    }
}

bool NearbyPlayerDebouncer::isActive() const
{
    return m_isActive; 
}

void NearbyPlayerDebouncer::fire(const Callback& cb) 
{ 
    if (cb) 
        cb(); 
}