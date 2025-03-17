#include "utils/HotkeyManager.h"
#include "utils/Settings.h"
#include <QHotkey>

HotkeyManager::HotkeyManager(Settings* settings, QObject* parent)
	: QObject(parent),
	m_settings(settings)
{
	loadFromSettings();
}

HotkeyManager::~HotkeyManager()
{
	saveToSettings();
	m_settings = nullptr;
	qDeleteAll(m_hotkeys);
	m_hotkeys.clear();
}

bool HotkeyManager::addHotkey(HotkeyActions::Action action, const QKeySequence& keySequence)
{
	QHotkey* hotkey = new QHotkey(this);
	if (!hotkey->setShortcut(keySequence, true))
	{
		qWarning() << "Failed to register hotkey for" << HotkeyActions::toString(action) << ":" << keySequence.toString();
		delete hotkey;
		return false;
	}

	if (m_hotkeys.contains(action))
	{
		delete m_hotkeys[action];
	}
	m_hotkeys[action] = hotkey;
	connect(hotkey, &QHotkey::activated, this, [this, action]() { emit hotkeyTriggered(action); });

	qDebug() << "Added hotkey for" << HotkeyActions::toString(action) << ":" << keySequence.toString();
	return true;
}

void HotkeyManager::removeHotkey(HotkeyActions::Action action)
{
	if (!m_hotkeys.contains(action)) return;

	delete m_hotkeys[action];
	m_hotkeys.remove(action);
	m_settings->saveValue(QString("hotkeys/%1").arg(HotkeyActions::toString(action)), QVariant());
	qDebug() << "Removed hotkey for" << HotkeyActions::toString(action);
}

void HotkeyManager::setHotkey(HotkeyActions::Action action, const QKeySequence& keySequence)
{
	if (!m_hotkeys.contains(action))
	{
		addHotkey(action, keySequence);
		return;
	}

	if (m_hotkeys[action]->setShortcut(keySequence, true))
	{
		qDebug() << "Hotkey for" << HotkeyActions::toString(action) << "changed to:" << keySequence.toString();
	}
	else 
	{
		qWarning() << "Failed to change hotkey for" << HotkeyActions::toString(action) << "to" << keySequence.toString();
	}
}
 
QKeySequence HotkeyManager::currentHotkey(HotkeyActions::Action action) const
{
	return m_hotkeys.contains(action) ? m_hotkeys[action]->shortcut() : QKeySequence();
}

QList<HotkeyActions::Action> HotkeyManager::hotkeyActions() const
{
	return m_hotkeys.keys();
}

void HotkeyManager::saveToSettings()
{
	if (!m_settings) return;

	QStringList actions;
	for (const auto& action : m_hotkeys.keys())
	{
		if (action == HotkeyActions::Action::None) continue;
		actions << HotkeyActions::toString(action);
	}

	m_settings->saveValue("hotkeys/actions", actions);
	for (const auto& action : m_hotkeys.keys())
	{
		if (action == HotkeyActions::Action::None) continue;
		m_settings->saveValue(QString("hotkey/%1").arg(HotkeyActions::toString(action)), m_hotkeys[action]->shortcut().toString());
	}
}

void HotkeyManager::loadFromSettings()
{
	if (!m_settings) {
		qWarning() << "No Settings provided to HotkeyManager!";
		return;
	}

	QStringList savedActions = m_settings->loadValue("hotkeys/actions", QStringList()).toStringList();

	if (savedActions.isEmpty())
	{
		qDebug() << "No hotkeys in settings, setting defaults.";
		setDefaults();
		return;
	}

	for (const QString& actionStr : savedActions)
	{
		HotkeyActions::Action action = HotkeyActions::fromString(actionStr);
		QString key = QString("hotkey/%1").arg(actionStr);
		QKeySequence defaultHotkey = HotkeyActions::defaultKeySequence(action);
		QString savedHotkey = m_settings->loadValue(key, defaultHotkey.toString()).toString();
		if (!savedHotkey.isEmpty()) {
			addHotkey(action, QKeySequence(savedHotkey));
		}
	}
}

void HotkeyManager::setDefaults()
{
	for (const auto& action : HotkeyActions::allValues())
	{
		QKeySequence defaultHotkey = HotkeyActions::defaultKeySequence(action);
		addHotkey(action, defaultHotkey);
	}
}
