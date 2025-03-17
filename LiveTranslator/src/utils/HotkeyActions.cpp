#include "utils/HotkeyActions.h"

QString HotkeyActions::toString(Action action)
{
#define ENUM_VALUE(name, string, defaultKey) case Action::name: return string;
    switch (action)
    {
        ENUM_HOTKEY_ACTIONS
    default: return "unknown";
    }
#undef ENUM_VALUE
}

HotkeyActions::Action HotkeyActions::fromString(const QString& actionStr)
{
#define ENUM_VALUE(name, string, defaultKey) if (actionStr == string) return Action::name;
    ENUM_HOTKEY_ACTIONS
        return Action::None;
#undef ENUM_VALUE
}

QKeySequence HotkeyActions::defaultKeySequence(Action action)
{
#define ENUM_VALUE(name, string, defaultKey) case Action::name: return QKeySequence(defaultKey);
    switch (action) {
        ENUM_HOTKEY_ACTIONS
    default: return QKeySequence();
    }
#undef ENUM_VALUE
}

QList<HotkeyActions::Action> HotkeyActions::allValues()
{
#define ENUM_VALUE(name, string, defaultKey) Action::name,
    return { ENUM_HOTKEY_ACTIONS };
#undef ENUM_VALUE
}
