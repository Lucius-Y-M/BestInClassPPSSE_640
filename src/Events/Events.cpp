#include "events.h"

namespace Events
{
	bool Install() {
		auto* listenerSingleton = MenuListener::GetSingleton();
		if (!listenerSingleton) {
			return false;
		}

		return listenerSingleton->RegisterListener();
	}

	MenuListener* MenuListener::GetSingleton() {
		static MenuListener singleton;
		return std::addressof(singleton);
	}

	RE::BSFixedString MenuListener::GetCurrentMenuName() {
		return CurrentMenu;
	}

	bool MenuListener::RegisterListener() {
		auto* eventHolder = RE::UI::GetSingleton();
		if (!eventHolder) return false;

		eventHolder->AddEventSink(this);
		return true;
	}

	bool MenuListener::GetShouldProcess() {
		return shouldProcess;
	}

	RE::BSEventNotifyControl MenuListener::ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
		RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
		using Control = RE::BSEventNotifyControl;

		shouldProcess = false;
		CurrentMenu = ""sv;

		if (!a_event) {
			return Control::kContinue;
		}
		if (a_event->menuName != RE::InventoryMenu::MENU_NAME &&
			a_event->menuName != RE::ContainerMenu::MENU_NAME && 
			a_event->menuName != RE::BarterMenu::MENU_NAME) {
			return Control::kContinue;
		}

		if (a_event->opening) {
			shouldProcess = true;
			CurrentMenu = a_event->menuName;
		}

		return Control::kContinue;
	}
}