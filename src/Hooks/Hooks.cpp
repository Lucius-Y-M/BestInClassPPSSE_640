#include "Hooks.h"

#include "Events/Events.h"
#include "ItemVisitor/ItemVisitor.h"
#include "RE/Offset.h"

#undef GetObject

namespace Hooks 
{
	void Install() {
		SKSE::AllocTrampoline(42);
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
	}

	void BestInClassListener::SetMemberIfBestInClass() {
		auto* visitor = ItemVisitor::ItemListVisitor::GetSingleton();
		if (!visitor) {
			return;
		}
		visitor->QueueTask();
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

	void BestInClassListener::EquipFromContainer::Install() {
		REL::Relocation<std::uintptr_t> target{ RE::Offset::ContainerMenu::EquipItem, offset };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to validate pattern of Container Menu Equip listener."sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
	}

	void BestInClassListener::BestInInventory::Thunk(void* a1) {
		(void)a1;
		auto* hookManager = BestInClassListener::GetSingleton();
		if (!hookManager) {
			logger::error("Best In Class Listerner failed to get manager singleton."sv);
			return;
		}
		hookManager->SetMemberIfBestInClass();
	}

	void BestInClassListener::BestInContainer::Thunk(void* a1) {
		(void)a1;
		auto* hookManager = BestInClassListener::GetSingleton();
		if (!hookManager) {
			logger::error("Best In Class Listerner failed to get manager singleton."sv);
			return;
		}
		hookManager->SetMemberIfBestInClass();
	}

	void BestInClassListener::BestInBarter::Thunk(void* a1) {
		(void)a1;
		auto* hookManager = BestInClassListener::GetSingleton();
		if (!hookManager) {
			logger::error("Best In Class Listerner failed to get manager singleton."sv);
			return;
		}
		hookManager->SetMemberIfBestInClass();
	}

	void BestInClassListener::EquipFromContainer::Thunk(void* a1, void* a2, void* a3) {
		_func(a1, a2, a3);
	}
}