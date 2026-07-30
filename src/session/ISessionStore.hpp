#pragma once

#include "user/User.hpp"

#include <string>

class ISessionStore
{
public:
    virtual ~ISessionStore() = default;

    /*
     * רושם session פעיל עבור המשתמש.
     *
     * מחזיר false אם למשתמש כבר קיים session פעיל אחר.
     * מחזיר true אם ה-session נרשם בהצלחה, או אם אותו
     * sessionId כבר רשום עבור אותו משתמש.
     */
    [[nodiscard]]
    virtual bool tryAcquire(
        UserId userId,
        const std::string& sessionId
    ) = 0;

    /*
     * מחדש את ה-TTL רק אם sessionId הוא הבעלים
     * הנוכחי של הרשומה.
     *
     * מחזיר false אם הרשומה אינה קיימת, פגה,
     * או שייכת ל-session אחר.
     */
    [[nodiscard]]
    virtual bool refresh(
        UserId userId,
        const std::string& sessionId
    ) = 0;

    /*
     * משחרר את ה-session רק אם sessionId הוא הבעלים
     * הנוכחי של הרשומה.
     *
     * תנאי זה מונע מחיבור ישן למחוק session חדש.
     */
    virtual void release(
        UserId userId,
        const std::string& sessionId
    ) = 0;
};