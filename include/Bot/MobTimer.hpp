#pragma once
#include <chrono>
#include <unordered_map>
#include <iostream>

#include "types.hpp"
#include "Bot/SettingsManager.hpp"

class MobTimer
{
public:
    MobTimer();
    void update(MobType detectedMob);
    bool isExpired() const;
    MobType getCurrentMob() const;
    void reset();

private:
    using Clock = std::chrono::steady_clock;

    std::unordered_map<MobType, std::chrono::milliseconds> durations;

    SettingsManager settingsManager;
    BotSettings settings;

    MobType currentMob = MobType::None;
    Clock::time_point startTime;
    bool expired = false;
};