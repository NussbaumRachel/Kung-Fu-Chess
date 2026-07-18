#pragma once

#include <optional>

/// מידע מעובד על קליק בתא — התוצאה של ClickPreparationService.
/// מועבר ל-GameStateMachine לצורך החלטה.
struct ClickContext
{
    int row = -1;
    int col = -1;
    bool isEmpty        = false;   // התא ריק
    bool hasFriendly    = false;   // יש כלי ידידותי בתא (אותו צבע כמו הנבחר)
    bool isInvolved     = false;   // התא מעורב במהלך/קפיצה פעילים (נעול)
    bool moveIsValid    = false;   // המהלך מהכלי הנבחר לתא זה חוקי
};