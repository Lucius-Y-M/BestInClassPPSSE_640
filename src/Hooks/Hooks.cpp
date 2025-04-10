#include "Hooks.h"

#include <xbyak.h>

#include "Events/Events.h"

#undef GetObject

namespace Hooks 
{
	using Slot = BestValueStorage::MaskedArmorSlot;

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
		BestInClassCall::Install();
	}

	void BestInClassListener::SetMemberIfBestInClass(const RE::BSFixedString& a_menuName) {
		auto* ui = RE::UI::GetSingleton();
		auto* intfcStr = RE::InterfaceStrings::GetSingleton();
		const auto* task = SKSE::GetTaskInterface();

		if (a_menuName == intfcStr->barterMenu) {
			const auto menu = static_cast<RE::BarterMenu*>(ui->GetMenu(a_menuName).get());
			task->AddTask(new ItemVisitor(menu->itemList->items));
		}
		if (a_menuName == intfcStr->containerMenu) {
			const auto menu = static_cast<RE::ContainerMenu*>(ui->GetMenu(a_menuName).get());
			task->AddTask(new ItemVisitor(menu->itemList->items));
		}
		if (a_menuName == intfcStr->inventoryMenu) {
			const auto menu = static_cast<RE::InventoryMenu*>(ui->GetMenu(a_menuName).get());
			task->AddTask(new ItemVisitor(menu->itemList->items));
		}
	}

	void BestInClassListener::BestInClassCall::Install() {
		REL::Relocation<std::uintptr_t> target{ REL::ID(82292) };
		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_originalFuncAddr, std::size_t a_originalByteLength)
			{
				for (size_t i = 0; i < a_originalByteLength; i++) {
					db(*reinterpret_cast<uint8_t*>(a_originalFuncAddr + i));
				}
				jmp(qword[rip]);
				dq(a_originalFuncAddr + a_originalByteLength);
			}
		};
		Patch p(target.address(), 6);
		p.ready();

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<5>(target.address(), Thunk);

		auto alloc = trampoline.allocate(p.getSize());
		memcpy(alloc, p.getCode(), p.getSize());
		_func = reinterpret_cast<std::uintptr_t>(alloc);
	}

	void BestInClassListener::BestInClassCall::Thunk(void* a1, RE::FxDelegate* a_delegate, const char* a_funcName, void* a4, bool a_flag) {
		if (strcmp(a_funcName, "bestInClass") != 0) {
			_func(a1, a_delegate, a_funcName, a4, a_flag);
			return;
		}

		auto* eventManager = Events::MenuListener::GetSingleton();
		auto* bestInClassManager = BestInClassListener::GetSingleton();
		if (!eventManager || !bestInClassManager || !eventManager->GetShouldProcess()) {
			_func(a1, a_delegate, a_funcName, a4, a_flag);
			return;
		}

		const auto menuName = eventManager->GetCurrentMenuName();
		if (menuName.empty()) {
			_func(a1, a_delegate, a_funcName, a4, a_flag);
			return;
		}
		bestInClassManager->SetMemberIfBestInClass(menuName);
	}

	void ItemVisitor::Visit() {
		if (!_list.data()) {
			return;
		}

		for (auto item : _list) {
			if (!item) {
				continue;
			}

			switch (item->data.objDesc->GetObject()->GetFormType()) {
			case RE::FormType::Armor:
			{
				CompareArmor(item);
				break;
			}
			case RE::FormType::Weapon:
			{
				CompareWeapon(item);
				break;
			}
			case RE::FormType::Ammo:
			{
				CompareAmmo(item);
				break;
			}
			default:;
			}
		}

		SetBest();
	}
	void ItemVisitor::CompareArmor(RE::ItemList::Item* a_item) {
		auto* const armor = static_cast<RE::TESObjectARMO*>(a_item->data.objDesc->GetObject());
		const auto type = std::underlying_type_t<RE::BIPED_MODEL::ArmorType>(armor->GetArmorType()) * 5;
		const auto slotMask = std::underlying_type_t<RE::BIPED_MODEL::BipedObjectSlot>(armor->GetSlotMask());
		uint32_t typeMask;

		switch (UnmaskToLowest(slotMask)) {
		case Slot::kHelmet:
		{
			typeMask = type + 0;
			break;
		}
		case Slot::kBody:
		{
			typeMask = type + 1;
			break;
		}
		case Slot::kHand:
		{
			typeMask = type + 2;
			break;
		}
		case Slot::kFeet:
		{
			typeMask = type + 3;
			break;
		}
		case Slot::kShield:
		{
			typeMask = type + 4;
			break;
		}
		default:
		{
			typeMask = -1;
			break;
		}
		}

		// discard the rest and ClothingShield, because there's no ClothingShield
		if (typeMask >= 0 && typeMask < 14) {
			auto& [item, value] = _bestStore.Armor[typeMask];

			// compare armor by rating, compare clothing by amount of enchantments / maybe gold value?
			const auto rhsCompare = typeMask < 10 ? armor->GetArmorRating() : armor->amountofEnchantment;

			if (value < rhsCompare) {
				item = a_item;
				value = rhsCompare;
			}
		}
	}

	void ItemVisitor::CompareWeapon(RE::ItemList::Item* a_item) {
		auto* const weapon = static_cast<RE::TESObjectWEAP*>(a_item->data.objDesc->GetObject());
		const auto typeMask = std::underlying_type_t<RE::WEAPON_TYPE>(weapon->GetWeaponType());

		auto& [item, value] = _bestStore.Weapon[typeMask];

		// compare weapon by damage, comapre staff by gold value
		const auto rhsCompare = typeMask == 8 ? weapon->GetGoldValue() : weapon->GetAttackDamage();

		if (value < rhsCompare) {
			item = a_item;
			value = rhsCompare;
		}
	}

	void ItemVisitor::CompareAmmo(RE::ItemList::Item* a_item) {
		auto* const ammo = static_cast<RE::TESAmmo*>(a_item->data.objDesc->GetObject());
		const auto typeMask = ammo->IsBolt() ? 0 : 1;

		auto& [item, value] = _bestStore.Ammo[typeMask];

		// compare ammo by damage
		const auto rhsCompare = ammo->data.damage;

		if (value < rhsCompare) {
			item = a_item;
			value = rhsCompare;
		}
	}

	Slot ItemVisitor::UnmaskToLowest(uint32_t a_slot) {
		// maybe reserve a certain size to increase performance -unbenchmarked
		std::vector<uint32_t> mask;

		while (a_slot > 0) {
			mask.push_back(a_slot % 2);
			a_slot = a_slot / 2;
		}

		for (auto i = 0; i < mask.size(); i++) {
			// return the lowest (the first) non-zero mask
			if (mask[i] == 1 && i != 0) {
				return static_cast<Slot>(i);
			}
		}

		return static_cast<Slot>(0);
	}

	void ItemVisitor::SetBest() {
		for (auto [item, value] : _bestStore.Armor) {
			if (item) {
				item->obj.SetMember("bestInClass", true);
			}
		}

		for (auto [item, value] : _bestStore.Weapon) {
			if (item) {
				item->obj.SetMember("bestInClass", true);
			}
		}

		for (auto [item, value] : _bestStore.Ammo) {
			if (item) {
				item->obj.SetMember("bestInClass", true);
			}
		}
	}

	ItemVisitor::ItemVisitor(const RE::BSTArray<RE::ItemList::Item*> a_itemList) :
		_list(a_itemList)
	{
	}

	void ItemVisitor::Run()
	{
		Visit();
	}


	void ItemVisitor::Dispose()
	{
		delete this;
	}
}