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

	class MenuListener : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static MenuListener* GetSingleton();

		RE::BSFixedString GetCurrentMenuName();
		bool RegisterListener();

		MenuListener(const MenuListener&) = delete;
		MenuListener(MenuListener&&) = delete;
		MenuListener& operator=(const MenuListener&) = delete;
		MenuListener& operator=(MenuListener&&) = delete;

	protected:
		MenuListener() = default;
		~MenuListener() = default;

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		RE::BSFixedString CurrentMenu{ ""sv };
	};
}