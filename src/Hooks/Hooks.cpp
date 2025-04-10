#include "Hooks.h"

#include <xbyak.h>

#include "Events/Events.h"
#include "ItemVisitor/ItemVisitor.h"

#undef GetObject

namespace Hooks 
{
	void Install()
	{
		SKSE::AllocTrampoline(64);
		BestInClassListener::GetSingleton()->Install();
	}

	BestInClassListener* BestInClassListener::GetSingleton() {
		static BestInClassListener singleton;
		return std::addressof(singleton);
	}

	void BestInClassListener::Install() {
		BestInInventory::Install();
	}

	void BestInClassListener::SetMemberIfBestInClass(const RE::BSFixedString& a_menuName) {
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}

		RE::BSTArray<RE::ItemList::Item*> items{};
		if (a_menuName == RE::BarterMenu::MENU_NAME) {
			const auto menu = static_cast<RE::BarterMenu*>(ui->GetMenu(a_menuName).get());
			items = menu->itemList->items;
		}
		else if (a_menuName == RE::ContainerMenu::MENU_NAME) {
			const auto menu = static_cast<RE::ContainerMenu*>(ui->GetMenu(a_menuName).get());
			items = menu->itemList->items;
		}
		else if (a_menuName == RE::InventoryMenu::MENU_NAME) {
			const auto menu = static_cast<RE::InventoryMenu*>(ui->GetMenu(a_menuName).get());
			items = menu->itemList->items;
		}
		else {
			return;
		}

		auto* visitor = new ItemVisitor::ItemListVisitor(items);
		TaskDelegate* delegate = reinterpret_cast<::TaskDelegate*>(visitor);
		SKSE::GetTaskInterface()->AddTask(delegate);
	}

	bool BestInClassListener::ShouldInvokeOriginal() {
		auto* eventManager = Events::MenuListener::GetSingleton();
		if (!eventManager) {
			return true;
		}

		const auto menuName = eventManager->GetCurrentMenuName();
		if (menuName.empty()) {
			return true;
		}

		SetMemberIfBestInClass(menuName);
		return false;
	}

	void BestInClassListener::BestInInventory::Install() {
		REL::Relocation<std::uintptr_t> target{ REL::ID(51866), 0x400 };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of Inventory Menu listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
	}

	void BestInClassListener::BestInContainer::Install() {
		REL::Relocation<std::uintptr_t> target{ REL::ID(51144), 0x4CE };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of Container Menu listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
	}

	void BestInClassListener::BestInBarter::Install() {
		REL::Relocation<std::uintptr_t> target{ REL::ID(50958), 0x300 };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of Barter Menu listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
	}

	void BestInClassListener::BestInInventory::Thunk(void* a1) {
		auto* manager = BestInClassListener::GetSingleton();
		if (!manager || manager->ShouldInvokeOriginal()) {
			_func(a1);
		}
	}

	void BestInClassListener::BestInContainer::Thunk(void* a1) {
		auto* manager = BestInClassListener::GetSingleton();
		if (!manager || manager->ShouldInvokeOriginal()) {
			_func(a1);
		}
	}

	void BestInClassListener::BestInBarter::Thunk(void* a1) {
		auto* manager = BestInClassListener::GetSingleton();
		if (!manager || manager->ShouldInvokeOriginal()) {
			_func(a1);
		}
	}
}