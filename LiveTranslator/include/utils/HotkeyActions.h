#ifndef HOTKEYACTIONS_H
#define HOTKEYACTIONS_H

#include <QString>
#include <QList>
#include <QKeySequence>

#define ENUM_HOTKEY_ACTIONS \
    ENUM_VALUE(Capture, "capture", "Ctrl+Alt+T") \
    ENUM_VALUE(Stop, "stop", "Ctrl+Alt+P") \
    ENUM_VALUE(None, "none", "")

#define ENUM_VALUE(name, string, defaultKey) name,

class HotkeyActions {
public:
    enum class Action {
        ENUM_HOTKEY_ACTIONS
    };

    static QString toString(Action action);
    static Action fromString(const QString& actionStr);
    static QKeySequence defaultKeySequence(Action action);
    static QList<Action> allValues();
};

#undef ENUM_VALUE
#endif // HOTKEYACTIONS_H