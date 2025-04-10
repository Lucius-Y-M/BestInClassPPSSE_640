#include "events.h"

#include "Hooks/Hooks.h"

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

		if (!a_event || !a_event->opening) {
			return Control::kContinue;
		}

		const auto ui = RE::UI::GetSingleton();
		if (!ui) {
			return Control::kContinue;
		}

		const auto eventName = a_event->menuName;
		if (eventName != RE::BarterMenu::MENU_NAME && 
			eventName != RE::ContainerMenu::MENU_NAME &&
			eventName != RE::InventoryMenu::MENU_NAME) {
			return Control::kContinue;
		}

		shouldProcess = true;
		CurrentMenu = eventName;

		if (auto* hookManager = Hooks::BestInClassListener::GetSingleton(); hookManager) {
			hookManager->SetMemberIfBestInClass(CurrentMenu);
		}

		return Control::kContinue;
	}
}