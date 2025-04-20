#include "Hooks.h"

#include "Events/Events.h"
#include "ItemVisitor/ItemVisitor.h"
#include "ListInvalidator/ListInvalidator.h"
#include "RE/Offset.h"

#undef GetObject

namespace Hooks 
{
	void Install() {
		SKSE::AllocTrampoline(84);
		BestInClassListener::GetSingleton()->Install();
	}

	BestInClassListener* BestInClassListener::GetSingleton() {
		static BestInClassListener singleton;
		return std::addressof(singleton);
	}

	void BestInClassListener::Install() {
		BestInInventory::Install();
		BestInBarter::Install();
		BestInContainer::Install();
		InvalidateListData::Install();
	}

	void BestInClassListener::ProcessEquipEvent() {
		SetMemberIfBestInClass(lastMenuName);
	}

	void BestInClassListener::SetMemberIfBestInClass(const RE::BSFixedString& a_menuName) {
		auto* visitor = ItemVisitor::ItemListVisitor::GetSingleton();
		if (!visitor) {
			lastMenuName = "";
			return;
		}
		visitor->QueueTask(a_menuName);
		lastMenuName = a_menuName;
	}

	void BestInClassListener::BestInInventory::Install() {
		REL::Relocation<std::uintptr_t> target{ RE::Offset::InventoryMenu::SetBestInClass, offset };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of Inventory Menu listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
	}

	void BestInClassListener::BestInContainer::Install() {
		REL::Relocation<std::uintptr_t> target{ RE::Offset::ContainerMenu::SetBestInClass, offset };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of Container Menu listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
	}

	void BestInClassListener::BestInBarter::Install() {
		REL::Relocation<std::uintptr_t> target{ RE::Offset::BarterMenu::SetBestInClass, offset };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of Barter Menu listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
	}

	void BestInClassListener::BestInInventory::Thunk(RE::InventoryMenu* a_this) {
		auto* hookManager = BestInClassListener::GetSingleton();
		if (!hookManager) {
			_func(a_this);
			logger::error("Best In Class Listerner failed to get manager singleton."sv);
			return;
		}
		hookManager->SetMemberIfBestInClass(a_this->MENU_NAME);
	}

	void BestInClassListener::BestInContainer::Thunk(RE::ContainerMenu* a_this) {
		auto* hookManager = BestInClassListener::GetSingleton();
		if (!hookManager) {
			_func(a_this);
			logger::error("Best In Class Listerner failed to get manager singleton."sv);
			return;
		}
		hookManager->SetMemberIfBestInClass(a_this->MENU_NAME);
	}

	void BestInClassListener::BestInBarter::Thunk(RE::BarterMenu* a_this) {
		auto* hookManager = BestInClassListener::GetSingleton();
		if (!hookManager) {
			_func(a_this);
			logger::error("Best In Class Listerner failed to get manager singleton."sv);
			return;
		}
		hookManager->SetMemberIfBestInClass(a_this->MENU_NAME);
	}

	void BestInClassListener::InvalidateListData::Install() {
		REL::Relocation<std::uintptr_t> targetBarter{ RE::Offset::BarterMenu::SetBestInClass, offsetBarter };
		REL::Relocation<std::uintptr_t> targetContainer{ RE::Offset::ContainerMenu::SetBestInClass, offsetContainer };
		REL::Relocation<std::uintptr_t> targetInventory{ RE::Offset::InventoryMenu::SetBestInClass, offsetInventory };
		if (!(REL::make_pattern<"E8">().match(targetBarter.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of the invalidate data listener."sv);
		}
		else if (!(REL::make_pattern<"E8">().match(targetContainer.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of the invalidate data listener."sv);
		}
		else if (!(REL::make_pattern<"E8">().match(targetInventory.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of the invalidate data listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_funcBarter = trampoline.write_call<5>(targetBarter.address(), &ThunkBarter);
		_funcContainer = trampoline.write_call<5>(targetContainer.address(), &ThunkContainer);
		_funcInventory = trampoline.write_call<5>(targetInventory.address(), &ThunkInventory);
	}

	void BestInClassListener::InvalidateListData::ThunkInventory(RE::GFxMovieView* a_list,
		const char* a_methodName,
		void* a_responseArgs) {
		auto* invalidator = ItemVisitor::ItemListInvalidator::GetSingleton();
		if (invalidator) {
			invalidator->QueueTask(RE::InventoryMenu::MENU_NAME);
		}
		else {
			logger::error("Failed to get internal invalidator");
			_funcInventory(a_list, a_methodName, a_responseArgs);
		}
	}

	void BestInClassListener::InvalidateListData::ThunkContainer(RE::GFxMovieView* a_list,
		const char* a_methodName,
		void* a_responseArgs) {
		auto* invalidator = ItemVisitor::ItemListInvalidator::GetSingleton();
		if (invalidator) {
			invalidator->QueueTask(RE::ContainerMenu::MENU_NAME);
		}
		else {
			logger::error("Failed to get internal invalidator");
			_funcContainer(a_list, a_methodName, a_responseArgs);
		}
	}

	void BestInClassListener::InvalidateListData::ThunkBarter(RE::GFxMovieView* a_list,
		const char* a_methodName,
		void* a_responseArgs) {
		auto* invalidator = ItemVisitor::ItemListInvalidator::GetSingleton();
		if (invalidator) {
			invalidator->QueueTask(RE::BarterMenu::MENU_NAME);
		}
		else {
			logger::error("Failed to get internal invalidator");
			_funcBarter(a_list, a_methodName, a_responseArgs);
		}
	}
}