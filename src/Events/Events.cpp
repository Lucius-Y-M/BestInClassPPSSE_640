#include "events.h"

#include "Hooks/Hooks.h"

namespace Events
{
	bool Install() {
		auto* menuListener = MenuListener::GetSingleton();
		auto* equipListener = EquipListener::GetSingleton();
		if (!menuListener || !equipListener) {
			SKSE::stl::report_and_fail("Failed to get internal listener singletons. This is fatal."sv);
		}

		return menuListener->RegisterListener() && equipListener->RegisterListener();
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
		if (!eventHolder) {
			logger::error("Failed to get UI Event Source Holder"sv);
			return false;
		}

		eventHolder->AddEventSink(this);
		return true;
	}

	RE::BSEventNotifyControl MenuListener::ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
		RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
		using Control = RE::BSEventNotifyControl;
		
		const auto ui = RE::UI::GetSingleton();
		if (!a_event || !ui) {
			CurrentMenu = ""sv;
			return Control::kContinue;
		}

		if (!a_event->opening && !CurrentMenu.empty()) {
			CurrentMenu = ""sv;
			return Control::kContinue;
		}

		const auto eventName = a_event->menuName;
		if (eventName != RE::BarterMenu::MENU_NAME && 
			eventName != RE::ContainerMenu::MENU_NAME &&
			eventName != RE::InventoryMenu::MENU_NAME) {
			return Control::kContinue;
		}

		CurrentMenu = eventName;

		auto* hookManager = Hooks::BestInClassListener::GetSingleton();
		if (hookManager) {
			hookManager->SetMemberIfBestInClass();
		}

		return Control::kContinue;
	}

	EquipListener* EquipListener::GetSingleton() {
		static EquipListener singleton;
		return std::addressof(singleton);
	}

	bool EquipListener::RegisterListener() {
		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder) {
			logger::error("Failed to get Script Event Source Holder"sv);
			return false;
		}

		eventHolder->AddEventSink(this);
		return true;
	}

	RE::BSEventNotifyControl EquipListener::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>*) {
		using Control = RE::BSEventNotifyControl;

		if (!a_event ||
			!a_event->actor ||
			!a_event->actor->IsPlayerRef()) {
			return Control::kContinue;
		}

		auto* hookManager = Hooks::BestInClassListener::GetSingleton();
		if (!hookManager) {
			return Control::kContinue;
		}

		hookManager->SetMemberIfBestInClass();
		return Control::kContinue;
	}
}