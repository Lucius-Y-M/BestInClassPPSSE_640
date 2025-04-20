#pragma once

namespace Events
{
	bool Install();

	class EquipListener : public RE::BSTEventSink<RE::TESEquipEvent> {
	public:
		static EquipListener* GetSingleton();

		bool RegisterListener();

		EquipListener(const EquipListener&) = delete;
		EquipListener(EquipListener&&) = delete;
		EquipListener& operator=(const EquipListener&) = delete;
		EquipListener& operator=(EquipListener&&) = delete;

	protected:
		EquipListener() = default;
		~EquipListener() = default;

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>*) override;
	};
}