#pragma once

namespace Events
{
	bool Install();

	class MenuListener : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static MenuListener* GetSingleton();

		RE::BSFixedString GetCurrentMenuName();
		bool RegisterListener();
		bool GetShouldProcess();

		MenuListener(const MenuListener&) = delete;
		MenuListener(MenuListener&&) = delete;
		MenuListener& operator=(const MenuListener&) = delete;
		MenuListener& operator=(MenuListener&&) = delete;

	protected:
		MenuListener() = default;
		~MenuListener() = default;

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		bool shouldProcess{ false };
		RE::BSFixedString CurrentMenu{ ""sv};
	};
}