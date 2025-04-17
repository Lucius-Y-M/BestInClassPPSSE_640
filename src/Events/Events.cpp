#include "events.h"

#include "Hooks/Hooks.h"
#include "Settings/INISettings.h"

namespace Events
{
	bool Install() {
		logger::info("=========================================================="sv);
		logger::info("Installing Event Listeners..."sv);
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

		logger::info("  >Installed Menu Listener."sv);
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

		const auto eventName = a_event->menuName;

		// When switching from a container view to your inventory view (container menu, barter menu)
		// you go into a magical land known as "Bad Design Decisions" that internally is called
		// cursor menu.
		CurrentMenu = 
			eventName == RE::CursorMenu::MENU_NAME ||
			eventName == RE::Console::MENU_NAME ?
				CurrentMenu : eventName;

		if (a_event->opening) {
			if (CurrentMenu != RE::BarterMenu::MENU_NAME &&
				CurrentMenu != RE::ContainerMenu::MENU_NAME &&
				CurrentMenu != RE::InventoryMenu::MENU_NAME) {
				CurrentMenu = ""sv;
				return Control::kContinue;
			}
		}
		else {
			if (eventName != RE::CursorMenu::MENU_NAME &&
				eventName != RE::Console::MENU_NAME &&
				(CurrentMenu == RE::BarterMenu::MENU_NAME ||
				CurrentMenu == RE::ContainerMenu::MENU_NAME ||
				CurrentMenu == RE::InventoryMenu::MENU_NAME)) {
				CurrentMenu = ""sv;
				return Control::kContinue;
			}
		}

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
		auto* iniManager = Settings::INI::Holder::GetSingleton();
		if (!iniManager) {
			logger::error("Failed to get ini manager in Equip Listener."sv);
			return false;
		}

		auto iniResponse = iniManager->GetStoredSetting<bool>("General|bOriginalFunctionality");
		if (iniResponse.has_value() && iniResponse.value()) {
			logger::info("  >Equip Listener will not be installed. (Original Functionality Mode)"sv);
			return true;
		}

		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder) {
			logger::error("Failed to get Script Event Source Holder"sv);
			return false;
		}

		logger::info("  >Installed Equip Listener."sv);
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