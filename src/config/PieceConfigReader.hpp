#pragma once

#include <string>
#include <optional>

/// קורא ערכים מקובצי config.json של הכלים.
/// כל הפונקציות סטטיות — אין צורך במופע.
class PieceConfigReader
{
public:
    /// קריאה גולמית של ערך כמחרוזת. מחזיר std::nullopt אם המפתח לא נמצא.
    static std::optional<std::string> readValue(const std::string& configPath,
                                                const std::string& key);

    /// קריאת double
    static double readDouble(const std::string& configPath,
                             const std::string& key,
                             double defaultVal = 0.0);

    /// קריאת מחרוזת עם מירכאות (עבור next_state)
    static std::string readString(const std::string& configPath,
                                  const std::string& key,
                                  const std::string& defaultVal = "");

    /// קריאת bool
    static bool readBool(const std::string& configPath,
                         const std::string& key,
                         bool defaultVal = false);
};
