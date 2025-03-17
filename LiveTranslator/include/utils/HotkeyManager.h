#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QMap>
#include "utils/HotkeyActions.h"

class Settings;
class QHotkey;

class HotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit HotkeyManager(Settings* settings, QObject* parent = nullptr);
    ~HotkeyManager() override;

    bool addHotkey(HotkeyActions::Action action, const QKeySequence& keySequence);
    void removeHotkey(HotkeyActions::Action action);
    void setHotkey(HotkeyActions::Action action, const QKeySequence& keySequence);
    QKeySequence currentHotkey(HotkeyActions::Action action) const;
    QList<HotkeyActions::Action> hotkeyActions() const;

signals:
    void hotkeyTriggered(HotkeyActions::Action action);

private:
    void saveToSettings();
    void loadFromSettings();
    void setDefaults();

    Settings* m_settings;
    QMap<HotkeyActions::Action, QHotkey*> m_hotkeys;
};

#endif