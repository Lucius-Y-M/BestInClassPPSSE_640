#include "events.h"

#include "Hooks/Hooks.h"
#include "Settings/INISettings.h"

namespace Events
{
	bool Install() {
		logger::info("=========================================================="sv);
		logger::info("Installing Event Listeners..."sv);
		auto* equipListener = EquipListener::GetSingleton();
		if (!equipListener) {
			SKSE::stl::report_and_fail("Failed to get internal listener singletons. This is fatal."sv);
		}

		return equipListener->RegisterListener();
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

		hookManager->ProcessEquipEvent();
		return Control::kContinue;
	}
}