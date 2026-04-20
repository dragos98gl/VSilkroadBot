#include "Bot/MobTimer.hpp"
#include "utils.hpp"

MobTimer::MobTimer():
    settingsManager()
{
    settings = settingsManager.loadSettings("assets/settings.json");

    durations[MobType::Normal]   = std::chrono::seconds(settings.normalMobTimeout);
    durations[MobType::Champion] = std::chrono::seconds(settings.championMobTimeout);
    durations[MobType::Giant]    = std::chrono::seconds(settings.giantMobTimeout);
}

void MobTimer::update(MobType detectedMob)
{
    if (detectedMob == MobType::None)
    {
        reset();
        return;
    }

    if (detectedMob != currentMob)
    {
        if (detectedMob == MobType::Giant)
        {
            std::cout<<"[LOG][COMBAT][" << utils::getTimestamp() << "] Giant mob found.\n";
        }
        if (detectedMob == MobType::Normal)
        {
            std::cout<<"[LOG][COMBAT][" << utils::getTimestamp() << "] Normal mob found.\n";
        }
        if (detectedMob == MobType::Champion)
        {
            std::cout<<"[LOG][COMBAT][" << utils::getTimestamp() << "] Champion mob found.\n";
        }   
        
        currentMob = detectedMob;
        startTime = Clock::now();
        expired = false;
    }

    if (currentMob != MobType::None)
    {
        auto now = Clock::now();
        auto elapsed = now - startTime;

        if (elapsed >= durations[currentMob])
        {
            expired = true;
        }
    }
}

bool MobTimer::isExpired() const
{
    return expired;
}

MobType MobTimer::getCurrentMob() const
{
    return currentMob;
}

void MobTimer::reset()
{
    currentMob = MobType::None;
    expired = false;
}