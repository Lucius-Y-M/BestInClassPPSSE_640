#include "ItemVisitor.h"

namespace ItemVisitor
{
	ItemListVisitor::ItemListVisitor(const RE::BSTArray<RE::ItemList::Item*> a_itemList) :
		_list(a_itemList)
	{
	}


	void ItemListVisitor::Run()
	{
		Visit();
	}


	void ItemListVisitor::Dispose()
	{
		delete this;
	}


	void ItemListVisitor::Visit()
	{
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


	void ItemListVisitor::CompareArmor(RE::ItemList::Item* a_item)
	{
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


	void ItemListVisitor::CompareWeapon(RE::ItemList::Item* a_item)
	{
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


	void ItemListVisitor::CompareAmmo(RE::ItemList::Item* a_item)
	{
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


	auto ItemListVisitor::UnmaskToLowest(uint32_t a_slot)
		-> Slot
	{
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


	void ItemListVisitor::SetBest()
	{
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
}