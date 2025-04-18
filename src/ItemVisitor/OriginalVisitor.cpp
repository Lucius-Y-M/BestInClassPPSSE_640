#include "OriginalVisitor.h"

#include "RE/Misc.h"

namespace OriginalVisitor
{
	ItemListVisitor::ItemListVisitor(const RE::BSFixedString& a_menuName, bool a_skyUIPresent) {
		skyUIPresent = a_skyUIPresent;
		menuName = a_menuName;
	}

	void ItemListVisitor::Run() {
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}

		if (menuName == RE::InventoryMenu::MENU_NAME) {
			const auto menu = ui->GetMenu<RE::InventoryMenu>();
			const auto menuList = menu ? menu->itemList : nullptr;
			if (!menuList) {
				logger::error("ItemListVisitor failed to get Inventory Menu list."sv);
				return;
			}
			if (!menuList->items.data() || menuList->items.empty()) {
				return;
			}
			_list = menuList->items;
		}
		else if (menuName == RE::BarterMenu::MENU_NAME) {
			const auto menu = ui->GetMenu<RE::BarterMenu>();
			const auto menuList = menu ? menu->itemList : nullptr;
			if (!menuList) {
				logger::error("ItemListVisitor failed to get Barter Menu list."sv);
				return;
			}
			if (!menuList->items.data() || menuList->items.empty()) {
				return;
			}
			_list = menuList->items;
		}
		else if (menuName == RE::ContainerMenu::MENU_NAME) {
			const auto menu = ui->GetMenu<RE::ContainerMenu>();
			const auto menuList = menu ? menu->itemList : nullptr;
			if (!menuList) {
				logger::error("ItemListVisitor failed to get Container Menu list."sv);
				return;
			}
			if (!menuList->items.data() || menuList->items.empty()) {
				return;
			}
			_list = menuList->items;
		}
		else {
			return;
		}
		Visit();
	}


	void ItemListVisitor::Dispose() {
		delete this;
	}


	void ItemListVisitor::Visit() {
		if (!_list.data() || _list.empty()) {
			return;
		}

		for (auto item : _list) {
			if (!item) {
				continue;
			}

			// SeaSparrow: while these SHOULD be valid, extra nullchecks were added.
			auto* itemEntry = item ? item->data.objDesc : nullptr;
			auto* itemBase = itemEntry ? itemEntry->GetObject() : nullptr;
			auto itemForm = itemBase ? itemBase->GetFormType() : RE::FormType::None;

			switch (itemForm) {
			case RE::FormType::Armor:
			{
				CompareArmor(item, itemBase);
				break;
			}
			case RE::FormType::Weapon:
			{
				CompareWeapon(item, itemBase);
				break;
			}
			case RE::FormType::Ammo:
			{
				CompareAmmo(item, itemBase);
				break;
			}
			default:
				break;
			}
		}

		SetBest();
	}


	void ItemListVisitor::CompareArmor(RE::ItemList::Item* a_item,
		RE::TESBoundObject* a_base)
	{
		auto* const armor = a_base->As<RE::TESObjectARMO>();
		if (!armor) {
			return;
		}

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


	void ItemListVisitor::CompareWeapon(RE::ItemList::Item* a_item,
		RE::TESBoundObject* a_base)
	{
		auto* const weapon = a_base->As<RE::TESObjectWEAP>();
		if (!weapon) {
			return;
		}

		const auto typeMask = std::underlying_type_t<RE::WEAPON_TYPE>(weapon->GetWeaponType());

		auto& [item, value] = _bestStore.Weapon[typeMask];

		// compare weapon by damage, comapre staff by gold value
		const auto rhsCompare = typeMask == 8 ? weapon->GetGoldValue() : weapon->GetAttackDamage();

		if (value < rhsCompare) {
			item = a_item;
			value = rhsCompare;
		}
	}


	void ItemListVisitor::CompareAmmo(RE::ItemList::Item* a_item,
		RE::TESBoundObject* a_base)
	{
		auto* const ammo = a_base->As<RE::TESAmmo>();
		if (!ammo) {
			return;
		}

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

		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}

		RE::ItemList* menuList = nullptr;
		
		if (menuName == RE::BarterMenu::MENU_NAME) {
			auto menu = ui->GetMenu<RE::BarterMenu>();
			if (!menu) {
				logger::error("Failed to get menu."sv);
				return;
			}
			menuList = menu->itemList;
		}
		else if (menuName == RE::ContainerMenu::MENU_NAME) {
			auto menu = ui->GetMenu<RE::ContainerMenu>();
			if (!menu) {
				logger::error("Failed to get menu."sv);
				return;
			}
			menuList = menu->itemList;
		}
		else {
			auto menu = ui->GetMenu<RE::InventoryMenu>();
			if (!menu) {
				logger::error("Failed to get menu."sv);
				return;
			}
			menuList = menu->itemList;
		}
		if (!menuList || !menuList->view) {
			logger::error("Failed to get menu list view - old method"sv);
			return;
		}

		auto response = RE::FxResponseArgs<0>();
		RE::InvalidateListData(menuList->view.get(), "InvalidateListData", response);
	}
}