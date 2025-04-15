#include "ItemVisitor.h"

#include "Events/Events.h"

namespace ItemVisitor
{
	ItemListVisitor* ItemListVisitor::GetSingleton() {
		static ItemListVisitor singleton;
		return std::addressof(singleton);
	}

	void ItemListVisitor::Run() {
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			logger::error("ItemListVisitor failed to get UI singleton."sv);
			return;
		}

		if (m_menuName == RE::InventoryMenu::MENU_NAME) {
			const auto menu = ui->GetMenu<RE::InventoryMenu>();
			const auto menuList = menu ? menu->itemList : nullptr;
			if (!menuList) {
				logger::error("ItemListVisitor failed to get Inventory Menu list."sv);
				return;
			}
			if (menuList->items.empty()) {
				return;
			}
			_list = menuList->items;
		}
		else if (m_menuName == RE::BarterMenu::MENU_NAME) {
			const auto menu = ui->GetMenu<RE::BarterMenu>();
			const auto menuList = menu ? menu->itemList : nullptr;
			if (!menuList) {
				logger::error("ItemListVisitor failed to get Barter Menu list."sv);
				return;
			}
			if (menuList->items.empty()) {
				return;
			}
			_list = menuList->items;
		}
		else if (m_menuName == RE::ContainerMenu::MENU_NAME) {
			const auto menu = ui->GetMenu<RE::ContainerMenu>();
			const auto menuList = menu ? menu->itemList : nullptr;
			if (!menuList) {
				logger::error("ItemListVisitor failed to get Gift Menu list."sv);
				return;
			}
			if (menuList->items.empty()) {
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
		m_menuName = "";
		queued = false;
		_list.clear();
		best = std::array<StoredObject, ARRAY_SIZE>();
	}

	bool ItemListVisitor::PreloadForms() {
		auto* gamePlayer = RE::PlayerCharacter::GetSingleton();
		if (!gamePlayer) {
			logger::error("ItemListVisitor: Failed to cache player.");
			return false;
		}
		player = gamePlayer;

		auto* dobj = RE::BGSDefaultObjectManager::GetSingleton();
		if (!dobj) {
			logger::error("ItemListVisitor: Failed to cache dobj.");
			return false;
		}
		auto* eitherSlot = dobj->GetObject<RE::BGSEquipSlot>(RE::DEFAULT_OBJECT::kEitherHandEquip);
		if (!eitherSlot) {
			logger::error("ItemListVisitor: Failed to cache either slot.");
			return false;
		}
		either = eitherSlot;

		auto* headKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorHelmet"sv);
		if (!headKeyword) {
			logger::error("ItemListVisitor: Failed to cache head keyword.");
			return false;
		}
		auto* cuirassKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorCuirass"sv);
		if (!cuirassKeyword) {
			logger::error("ItemListVisitor: Failed to cache body keyword.");
			return false;
		}
		auto* armsKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorGauntlets"sv);
		if (!armsKeyword) {
			logger::error("ItemListVisitor: Failed to cache arms keyword.");
			return false;
		}
		auto* bootsKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorBoots"sv);
		if (!bootsKeyword) {
			logger::error("ItemListVisitor: Failed to cache boots keyword.");
			return false;
		}
		wornCuirass = headKeyword;
		wornHelmet = cuirassKeyword;
		wornArms = armsKeyword;
		wornLegs = bootsKeyword;

		auto* heavyKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorHeavy"sv);
		if (!heavyKeyword) {
			logger::error("ItemListVisitor: Failed to cache heavy keyword.");
			return false;
		}
		auto* lightKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorLight"sv);
		if (!lightKeyword) {
			logger::error("ItemListVisitor: Failed to cache light keyword.");
			return false;
		}
		heavyArmor = heavyKeyword;
		lightArmor = lightKeyword;
		return true;
	}

	void ItemListVisitor::QueueTask() {
		if (queued) {
			return;
		}

		auto* menuHandler = Events::MenuListener::GetSingleton();
		m_menuName = menuHandler ? menuHandler->GetCurrentMenuName() : "";
		if (m_menuName.empty() || (
			m_menuName != RE::InventoryMenu::MENU_NAME &&
			m_menuName != RE::ContainerMenu::MENU_NAME &&
			m_menuName != RE::BarterMenu::MENU_NAME)) {
			return;
		}

		queued = true;
		SKSE::GetTaskInterface()->AddTask(reinterpret_cast<::TaskDelegate*>(this));
	}

	void ItemListVisitor::Visit() {
		if (!_list.data() || _list.empty()) {
			return;
		}

		for (auto* item : _list) {
			auto* extraData = item ? item->data.objDesc : nullptr;
			auto* baseObject = extraData ? extraData->GetObject() : nullptr;
			auto formType = baseObject ? baseObject->GetFormType() : RE::FormType::None;

			if (!baseObject || !baseObject->GetPlayable()) {
				continue;
			}

			item->obj.SetMember("bestInClass", false);
			bool skip = false;

			RE::TESAmmo* ammo = nullptr;
			RE::TESObjectWEAP* weap = nullptr;
			RE::TESObjectARMO* armo = nullptr;

			switch (formType) {
			case RE::FormType::Ammo:
				ammo = baseObject->As<RE::TESAmmo>();
				break;
			case RE::FormType::Armor:
				armo = baseObject->As<RE::TESObjectARMO>();
				break;
			case RE::FormType::Weapon:
				weap = baseObject->As<RE::TESObjectWEAP>();
				break;
			default:
				skip = true;
				break;
			}
			if (skip) {
				continue;
			}

			if (ammo) {
				EvaluateAmmo(ammo, extraData, item);
			}
			else if (weap) {
				EvaluateWeapon(weap, extraData, item);
			}
			else if (armo) {
				EvaluateArmor(armo, extraData, item);
			}
		}

		SetBest();
	}

	void ItemListVisitor::SetBest() {
		for (uint64_t i = 0; i < 4; ++i) {
			auto& candidate = best.at(i);
			if (!candidate.item) {
				continue;
			}

			uint64_t secondary = i;
			auto* armor = candidate.item->data.objDesc->GetObject()->As<RE::TESObjectARMO>();
			if (armor->HasKeyword(heavyArmor->formID)) {
				secondary += ARMOR_HEAVY_START;
			}
			else if (armor->HasKeyword(lightArmor->formID)) {
				secondary += ARMOR_LIGHT_START;
			}
			else {
				secondary += ARMOR_CLOTH_START;
			}

			auto& alternate = best.at(secondary);
			alternate.Compare(candidate.item, candidate.value);
			alternate.item->obj.SetMember("bestInClass", true);
		}

		for (uint64_t i = WEAPON_START_INDEX; i < WEAPON_UNEQUIPPED_START + WEAPON_START_INDEX; ++i) {
			auto& candidate = best.at(i);
			if (!candidate.item) {
				continue;
			}

			uint64_t secondary = i + WEAPON_UNEQUIPPED_START;
			auto& alternate = best.at(secondary);
			alternate.Compare(candidate.item, candidate.value);
			alternate.item->obj.SetMember("bestInClass", true);
		}

		for (uint64_t i = AMMO_START_INDEX; i < AMMO_START_INDEX + AMMO_UNEQUIPPED_START; ++i) {
			auto& candidate = best.at(i);
			if (!candidate.item) {
				continue;
			}

			uint64_t secondary = i + AMMO_UNEQUIPPED_START;
			auto& alternate = best.at(secondary);
			alternate.Compare(candidate.item, candidate.value);
			alternate.item->obj.SetMember("bestInClass", true);
		}
	}

	void ItemListVisitor::EvaluateArmor(RE::TESObjectARMO* a_armor,
		RE::InventoryEntryData* a_data,
		RE::ItemList::Item* a_item) {
		uint64_t index = 0;

		bool worn = a_data ? a_data->IsWorn() : false;

		if (a_armor->HasKeyword(heavyArmor->formID)) {
			index += worn ? 0 : ARMOR_HEAVY_START;
		}
		else if (a_armor->HasKeyword(lightArmor->formID)) {
			index += worn ? 0 : ARMOR_LIGHT_START;
		}
		else if (a_armor->HasKeyword(clothArmor)) {
			index += worn ? 0 : ARMOR_CLOTH_START;
		}
		else {
			return;
		}

		if (a_armor->HasKeyword(wornHelmet->formID)) {
			index += ARMOR_HEAD_INDEX;
		}
		else if (a_armor->HasKeyword(wornArms->formID)) {
			index += ARMOR_ARMS_INDEX;
		}
		else if (a_armor->HasKeyword(wornLegs->formID)) {
			index += ARMOR_BOOTS_INDEX;
		}
		else if (!a_armor->HasKeyword(wornCuirass->formID)) {
			return;
		}

		float value = player->GetArmorValue(a_data);
		best.at(index).Compare(a_item, value);
	}

	void ItemListVisitor::EvaluateWeapon(RE::TESObjectWEAP* a_weap,
		RE::InventoryEntryData* a_data,
		RE::ItemList::Item* a_item) {
		uint64_t index = WEAPON_START_INDEX;
		auto* slot = a_weap->GetEquipSlot();
		if (!slot) {
			return;
		}

		bool equipped = a_data ? a_data->IsWorn() : false;

		index += a_weap->IsRanged() ?
			WEAPON_RANGED_INDEX :
			slot == either ?
			WEAPON_ONEHANDED_INDEX :
			WEAPON_TWOHANDED_INDEX;

		index += equipped ? 0 : WEAPON_UNEQUIPPED_START;

		float value = player->GetDamage(a_data);
		best.at(index).Compare(a_item, value);
	}

	void ItemListVisitor::EvaluateAmmo(RE::TESAmmo* a_weap,
		RE::InventoryEntryData* a_data,
		RE::ItemList::Item* a_item) {
		uint64_t index = AMMO_START_INDEX;

		bool equipped = a_data ? a_data->IsWorn() : false;

		index += equipped ? 0 : AMMO_UNEQUIPPED_START;
		index += a_weap->IsBolt() ? 0 : 1;

		float value = a_weap->data.damage;
		auto* proj = a_weap->data.projectile;
		auto* ench = proj ? proj->data.explosionType : nullptr;
		if (ench) {
			value += ench->data.damage;
		}

		best.at(index).Compare(a_item, value);
	}
}