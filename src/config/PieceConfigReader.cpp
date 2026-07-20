#include "config/PieceConfigReader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

std::optional<std::string> PieceConfigReader::readValue(const std::string& configPath,
                                                         const std::string& key)
{
    std::ifstream file(configPath);
    if (!file.is_open()) return std::nullopt;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    // חיפוש המפתח: "key"
    std::string searchKey = "\"" + key + "\"";
    auto pos = content.find(searchKey);
    if (pos == std::string::npos) return std::nullopt;

    auto colon = content.find(':', pos);
    if (colon == std::string::npos) return std::nullopt;

    std::string val = content.substr(colon + 1);

    // ניקוי: רווחים, פסיקים, ירידות שורה, סוגריים מסולסלים
    val.erase(std::remove_if(val.begin(), val.end(),
        [](char c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r'
                || c == ',' || c == '{' || c == '}';
        }),
        val.end());

    if (val.empty()) return std::nullopt;

    // הסרת מירכאות אם יש
    if (val.front() == '"' && val.back() == '"')
        val = val.substr(1, val.size() - 2);

    return val;
}

double PieceConfigReader::readDouble(const std::string& configPath,
                                      const std::string& key,
                                      double defaultVal)
{
    auto val = readValue(configPath, key);
    if (!val.has_value() || val->empty()) return defaultVal;
    try {
        return std::stod(val.value());
    } catch (...) {
        return defaultVal;
    }
}

std::string PieceConfigReader::readString(const std::string& configPath,
                                           const std::string& key,
                                           const std::string& defaultVal)
{
    auto val = readValue(configPath, key);
    return val.has_value() ? val.value() : defaultVal;
}

bool PieceConfigReader::readBool(const std::string& configPath,
                                  const std::string& key,
                                  bool defaultVal)
{
    auto val = readValue(configPath, key);
    if (!val.has_value()) return defaultVal;
    return (val.value() == "true");
}
