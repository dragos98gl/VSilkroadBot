#include "Bot/SettingsManager.hpp"

BotSettings SettingsManager::loadSettings(const std::string& filename)
{
    if (!std::filesystem::exists(filename)) {
        BotSettings defaults = createDefault();
        saveSettings(defaults, filename);
        return defaults;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        return createDefault();
    }

    json j;
    file >> j;

    BotSettings settings = createDefault();

    loadPixel(j, "mobHealthPixel",      settings.mobHealthPixel);
    loadPixel(j, "isEliteMobPixel",     settings.isEliteMobPixel);
    loadPixel(j, "isPartyMobPixel",     settings.isPartyMobPixel);
    loadPixel(j, "isNormalMobPixel",    settings.isNormalMobPixel);
    loadPixel(j, "isChampionMobPixel",  settings.isChampionMobPixel);
    loadPixel(j, "isGiantMobPixel",     settings.isGiantMobPixel);
    loadPixel(j, "skill1Pixel",         settings.Skill1Pixel);
    loadPixel(j, "skill2Pixel",         settings.Skill2Pixel);
    loadPixel(j, "skill3Pixel",         settings.Skill3Pixel);
    loadPixel(j, "skill4Pixel",         settings.Skill4Pixel);
    loadPixel(j, "skill5Pixel",         settings.Skill5Pixel);
    loadPixel(j, "buff1Pixel",          settings.Buff1Pixel);
    loadPixel(j, "buff2Pixel",          settings.Buff2Pixel);
    loadPixel(j, "buff3Pixel",          settings.Buff3Pixel);

    if (j.contains("waitForMobValue"))
        settings.waitForMobValue = std::chrono::milliseconds(j["waitForMobValue"].get<long long>());

    if (j.contains("waitForHealthBarValue"))
        settings.waitForHealthBarValue = std::chrono::milliseconds(j["waitForHealthBarValue"].get<long long>());

    if (j.contains("doubleClickOnMob"))
        settings.doubleClickOnMob = j["doubleClickOnMob"].get<bool>();

    if (j.contains("noMobsFoundThreshold"))
        settings.noMobsFoundThreshold = j["noMobsFoundThreshold"].get<int>();

    if (j.contains("mobsSelectionFailureThreshold"))
        settings.mobsSelectionFailureThreshold = j["mobsSelectionFailureThreshold"].get<int>();

    if (j.contains("mobsSeekLimit"))
        settings.mobsSeekLimit = j["mobsSeekLimit"].get<int>();

    if (j.contains("windowName"))
        settings.windowName = j["windowName"].get<std::string>();

    if (j.contains("normalMobTimeout"))
        settings.normalMobTimeout = j["normalMobTimeout"].get<int>();

    if (j.contains("championMobTimeout"))
        settings.championMobTimeout = j["championMobTimeout"].get<int>();

    if (j.contains("giantMobTimeout"))
        settings.giantMobTimeout = j["giantMobTimeout"].get<int>();

    loadNavigation(j, settings.navigationStructure, settings.activeNavigationStructure);

    return settings;
}

void SettingsManager::saveSettings(const BotSettings& settings,
                    const std::string& filename)
{
    json j;

    savePixel(j, "mobHealthPixel",      settings.mobHealthPixel);
    savePixel(j, "isEliteMobPixel",     settings.isEliteMobPixel);
    savePixel(j, "isPartyMobPixel",     settings.isPartyMobPixel);
    savePixel(j, "isNormalMobPixel",    settings.isNormalMobPixel);
    savePixel(j, "isChampionMobPixel",  settings.isChampionMobPixel);
    savePixel(j, "isGiantMobPixel",     settings.isGiantMobPixel);
    savePixel(j, "skill1Pixel",         settings.Skill1Pixel);
    savePixel(j, "skill2Pixel",         settings.Skill2Pixel);
    savePixel(j, "skill3Pixel",         settings.Skill3Pixel);
    savePixel(j, "skill4Pixel",         settings.Skill4Pixel);
    savePixel(j, "skill5Pixel",         settings.Skill5Pixel);
    savePixel(j, "buff1Pixel",          settings.Buff1Pixel);
    savePixel(j, "buff2Pixel",          settings.Buff2Pixel);
    savePixel(j, "buff3Pixel",          settings.Buff3Pixel);

    saveNavigation(j, settings.navigationStructure);
    saveCombatSettings(j, settings);

    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);
    }
}

void SettingsManager::loadPixel(const json& j, const char* key, PixelInfo& pixel)
{
    if (!j.contains(key)) return;

    const auto& p = j[key];
    pixel.position.setX(p.value("x", 0));
    pixel.position.setY(p.value("y", 0));

    int r = p.value("r", 0);
    int g = p.value("g", 0);
    int b = p.value("b", 0);
    pixel.color = QColor(r, g, b);
}

void SettingsManager::savePixel(json& j, const char* key, const PixelInfo& pixel)
{
    j[key] = {
        {"x", pixel.position.x()},
        {"y", pixel.position.y()},
        {"r", pixel.color.red()},
        {"g", pixel.color.green()},
        {"b", pixel.color.blue()}
    };
}

void SettingsManager::saveCombatSettings(json& j, const BotSettings& settings)
{
    j["waitForMobValue"]                = settings.waitForMobValue.count();
    j["waitForHealthBarValue"]          = settings.waitForHealthBarValue.count();
    j["doubleClickOnMob"]               = settings.doubleClickOnMob;
    j["noMobsFoundThreshold"]           = settings.noMobsFoundThreshold;
    j["mobsSelectionFailureThreshold"]  = settings.mobsSelectionFailureThreshold;
    j["mobsSeekLimit"]                  = settings.mobsSeekLimit;
    j["windowName"]                     = settings.windowName;
    
    j["normalMobTimeout"]               = settings.normalMobTimeout;
    j["championMobTimeout"]             = settings.championMobTimeout;
    j["giantMobTimeout"]                = settings.giantMobTimeout;
}

void SettingsManager::loadNavigation(const json& j, std::vector<NavigationStructure>& out, NavigationStructure& activeOut)
{
    if (!j.contains("navigation")) return;

    for (const auto& area : j["navigation"])
    {
        NavigationStructure navStruct;
        navStruct.name = area.value("name", "");
        navStruct.isActive = area.value("isActive", false);

        if (!area.contains("points")) continue;

        for (const auto& p : area["points"])
        {
            NavigationElement e;
            e.pos.first  = p.value("x", 0);
            e.pos.second = p.value("y", 0);
            e.radius     = p.value("radius", 0.0f);
            e.type       = static_cast<NavigationType>(p.value("type", 0));

            switch (e.type)
            {
                case NavigationType::ATTACKING_AREA:
                    navStruct.attackingArea.push_back(e);
                    break;

                case NavigationType::TO_GO_WHEN_NO_MOBS:
                    navStruct.toGoWhenNoMobs.push_back(e);
                    break;

                case NavigationType::TRAJECTORY:
                    navStruct.trajectory.push_back(e);
                    break;

                case NavigationType::BYPASS:
                    navStruct.bypass.push_back(e);
                    break;
            }
        }

        out.push_back(navStruct);
        if (navStruct.isActive)
        {
            activeOut = navStruct;
        }
    }
}

void SettingsManager::saveNavigation(json& j, const std::vector<NavigationStructure>& list)
{
    json navigationArray = json::array();

    for (const auto& zone : list)
    {
        json zoneObj;
        zoneObj["name"] = zone.name;
        zoneObj["isActive"] = zone.isActive;

        json pointsArray = json::array();

        auto addPoints = [&](const std::vector<NavigationElement>& vec)
        {
            for (const auto& e : vec)
            {
                pointsArray.push_back({
                    {"x", e.pos.first},
                    {"y", e.pos.second},
                    {"radius", e.radius},
                    {"type", static_cast<int>(e.type)}
                });
            }
        };

        addPoints(zone.attackingArea);
        addPoints(zone.toGoWhenNoMobs);
        addPoints(zone.trajectory);
        addPoints(zone.bypass);

        zoneObj["points"] = pointsArray;
        navigationArray.push_back(zoneObj);
    }

    j["navigation"] = navigationArray;
}

BotSettings SettingsManager::createDefault()
{
    BotSettings s;

    s.Skill1Pixel                   = { QPoint(561, 732), QColor(0, 0, 0) };
    s.Skill2Pixel                   = { QPoint(597, 732), QColor(0, 0, 0) };
    s.Skill3Pixel                   = { QPoint(632, 732), QColor(0, 0, 0) };
    s.Skill4Pixel                   = { QPoint(677, 732), QColor(0, 0, 0) };
    s.Skill5Pixel                   = { QPoint(708, 732), QColor(0, 0, 0) };
    s.Buff1Pixel                    = { QPoint(0, 0), QColor(0, 0, 0) };
    s.Buff2Pixel                    = { QPoint(0, 0), QColor(0, 0, 0) };
    s.Buff3Pixel                    = { QPoint(0, 0), QColor(0, 0, 0) };

    s.isEliteMobPixel              = { QPoint(657, 68), QColor(222, 255, 222) };
    s.isNormalMobPixel             = { QPoint(657, 69), QColor(33, 41, 115) };
    s.isPartyMobPixel              = { QPoint(657, 70), QColor(74, 0, 82) };
    s.isChampionMobPixel           = { QPoint(658, 68), QColor(165, 239, 66) };
    s.isGiantMobPixel              = { QPoint(657, 70), QColor(82, 0, 74) };

    s.mobHealthPixel               = { QPoint(579, 45), QColor(255, 198, 198) };

    s.waitForMobValue               = std::chrono::milliseconds(200);
    s.waitForHealthBarValue         = std::chrono::milliseconds(200);
    s.doubleClickOnMob              = false;
    s.noMobsFoundThreshold          = 2;
    s.mobsSelectionFailureThreshold = 2;
    s.mobsSeekLimit                 = 5;

    s.normalMobTimeout              = 3;
    s.championMobTimeout            = 3;
    s.giantMobTimeout               = 10;
    
    return s;
}