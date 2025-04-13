#include "ItemVisitor.h"

namespace ItemVisitor
{
	ItemListVisitor::ItemListVisitor(const RE::BSTArray<RE::ItemList::Item*> a_itemList) {
		_list = a_itemList;

		auto* gamePlayer = RE::PlayerCharacter::GetSingleton();
		if (!gamePlayer) {
			throw new std::exception("ItemListVisitor: Failed to cache player.");
		}
		player = gamePlayer;

		auto* dobj = RE::BGSDefaultObjectManager::GetSingleton();
		if (!dobj) {
			throw new std::exception("ItemListVisitor: Failed to cache dobj.");
		}
		auto* eitherSlot = dobj->GetObject<RE::BGSEquipSlot>(RE::DEFAULT_OBJECT::kEitherHandEquip);
		if (!eitherSlot) {
			throw new std::exception("ItemListVisitor: Failed to cache either slot.");
		}
		either = eitherSlot;

		auto* headKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorHelmet"sv);
		if (!headKeyword) {
			throw new std::exception("ItemListVisitor: Failed to cache head keyword.");
		}
		auto* cuirassKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorCuirass"sv);
		if (!cuirassKeyword) {
			throw new std::exception("ItemListVisitor: Failed to cache body keyword.");
		}
		auto* armsKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorGauntlets"sv);
		if (!armsKeyword) {
			throw new std::exception("ItemListVisitor: Failed to cache arms keyword.");
		}
		auto* bootsKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorBoots"sv);
		if (!bootsKeyword) {
			throw new std::exception("ItemListVisitor: Failed to cache boots keyword.");
		}
		wornCuirass = headKeyword;
		wornHelmet = cuirassKeyword;
		wornArms = armsKeyword;
		wornLegs = bootsKeyword;

		auto* heavyKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorHeavy"sv);
		if (!heavyKeyword) {
			throw new std::exception("ItemListVisitor: Failed to cache heavy keyword.");
		}
		auto* lightKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorLight"sv);
		if (!lightKeyword) {
			throw new std::exception("ItemListVisitor: Failed to cache light keyword.");
		}
		heavyArmor = heavyKeyword;
		lightArmor = lightKeyword;
	}

	void ItemListVisitor::Run() {
		Visit();
	}

	void ItemListVisitor::Dispose() {
		delete this;
	}

	void ItemListVisitor::Visit() {
		if (!_list.data()) {
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
		bool worn = a_data->IsWorn();

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
		try {
			best.at(index).Compare(a_item, value);
		}
		catch (std::out_of_range& e) {
			logger::error("Bad index created for {} ({}).", a_armor->GetName(), index);
		}
	}

	void ItemListVisitor::EvaluateWeapon(RE::TESObjectWEAP* a_weap,
		RE::InventoryEntryData* a_data,
		RE::ItemList::Item* a_item) {
		uint64_t index = WEAPON_START_INDEX;
		auto* slot = a_weap->GetEquipSlot();
		if (!slot) {
			return;
		}

		index += a_weap->IsRanged() ?
			WEAPON_RANGED_INDEX :
			slot == either ?
			WEAPON_ONEHANDED_INDEX :
			WEAPON_TWOHANDED_INDEX;

		index += a_data->IsWorn() ? 0 : WEAPON_UNEQUIPPED_START;

		float value = player->GetDamage(a_data);
		best.at(index).Compare(a_item, value);
	}

	void ItemListVisitor::EvaluateAmmo(RE::TESAmmo* a_weap,
		RE::InventoryEntryData* a_data,
		RE::ItemList::Item* a_item) {
		uint64_t index = AMMO_START_INDEX;

		index += a_data->IsWorn() ? 0 : AMMO_UNEQUIPPED_START;
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