#pragma once

#include <fstream>
#include <filesystem>
#include <vector>

#include <QPoint>
#include <QColor>

#include <nlohmann/json.hpp>

#include "types.hpp"

using json = nlohmann::json;

class SettingsManager {
public:
    // ================= LOAD =================
    BotSettings loadSettings(const std::string& filename);
    // ================= SAVE =================
    void saveSettings(const BotSettings& settings,
                      const std::string& filename);
private:
    void saveCombatSettings(json& j, const BotSettings& settings);
    // ================= PIXEL HELPERS =================
    void loadPixel(const json& j, const char* key, PixelInfo& pixel);
    void savePixel(json& j, const char* key, const PixelInfo& pixel);
    // ================= NAVIGATION =================
    void loadNavigation(const json& j, std::vector<NavigationStructure>& out, NavigationStructure& activeOut);
    void saveNavigation(json& j, const std::vector<NavigationStructure>& list);
    // ================= DEFAULTS =================
    BotSettings createDefault();
};
