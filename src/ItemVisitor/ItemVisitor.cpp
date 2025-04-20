#include "ItemVisitor.h"

#include "OriginalVisitor.h"
#include "ListInvalidator/ListInvalidator.h"
#include "RE/Misc.h"
#include "Settings/INISettings.h"

namespace ItemVisitor
{
	ItemListVisitor* ItemListVisitor::GetSingleton() {
		static ItemListVisitor singleton;
		return std::addressof(singleton);
	}

	void ItemListVisitor::Run() {
#ifndef NDEBUG
		const auto then = std::chrono::high_resolution_clock::now();
#endif
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
			if (!menuList->items.data() || menuList->items.empty()) {
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
			if (!menuList->items.data() || menuList->items.empty()) {
				return;
			}
			_list = menuList->items;
		}
		else if (m_menuName == RE::ContainerMenu::MENU_NAME) {
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

		for (const auto* spell : player->addedSpells) {
			if (!spell || spell->data.spellType == RE::MagicSystem::SpellType::kAbility) {
				continue;
			}
			playerSpells.insert(spell);
		}
		
		if (flagSpellBooks) {
			auto* playerBase = player->GetActorBase();
			if (!playerBase) {
				logger::error("ItemVisitor: Failed to get player actorbase.");
				return;
			}

			const auto* playerEffects = playerBase->actorEffects;
			auto** baseSpells = playerEffects ? playerEffects->spells : nullptr;
			if (baseSpells) {
				for (const auto* spell : std::span(baseSpells, playerEffects->numSpells)) {
					if (!spell || spell->data.spellType == RE::MagicSystem::SpellType::kAbility) {
						continue;
					}
					playerSpells.insert(spell);
				}
			}
		}

		Visit();
#ifndef NDEBUG
		const auto now = std::chrono::high_resolution_clock::now();
		const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count();
		logger::debug("Ran algorithm in {}ms", elapsed);
#endif
	}

	void ItemListVisitor::Dispose() {
		auto* invalidator = ItemVisitor::ItemListInvalidator::GetSingleton();
		if (invalidator) {
			invalidator->QueueTask(m_menuName);
		}

		m_menuName = "";
		queued = false;
		_list.clear();
		playerSpells.clear();
		optionalFlags.clear();
		best = std::array<StoredObject, ARRAY_SIZE>();
	}

	bool ItemListVisitor::PreloadForms() {
		auto* dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			logger::error("ItemVisitor: Failed to get data handler.");
			return false;
		}

		const auto* skyUIMod = dataHandler->LookupModByName("SkyUI_SE.esp");
		if (skyUIMod) {
			skyUIPresent = true;
		}

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
		auto* eitherSlot = dobj->GetObject<RE::BGSEquipSlot>(RE::DEFAULT_OBJECT::kRightHandEquip);
		if (!eitherSlot) {
			logger::error("ItemListVisitor: Failed to cache either slot.");
			return false;
		}
		either = eitherSlot;

		auto* headClothKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ClothingHead"sv);
		if (!headClothKeyword) {
			logger::error("ItemListVisitor: Failed to cache cloth head keyword.");
			return false;
		}
		auto* cuirassClothKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ClothingBody"sv);
		if (!cuirassClothKeyword) {
			logger::error("ItemListVisitor: Failed to cache cloth body keyword.");
			return false;
		}
		auto* armsClothKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ClothingHands"sv);
		if (!armsClothKeyword) {
			logger::error("ItemListVisitor: Failed to cache cloth arms keyword.");
			return false;
		}
		auto* bootsClothKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ClothingFeet"sv);
		if (!bootsClothKeyword) {
			logger::error("ItemListVisitor: Failed to cache cloth boots keyword.");
			return false;
		}
		clothCuirass = headClothKeyword;
		clothHelmet = cuirassClothKeyword;
		clothArms = armsClothKeyword;
		clothLegs = bootsClothKeyword;

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
		auto* clothKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ArmorClothing"sv);
		if (!lightKeyword) {
			logger::error("ItemListVisitor: Failed to cache clothing keyword.");
			return false;
		}
		heavyArmor = heavyKeyword;
		lightArmor = lightKeyword;
		clothArmor = clothKeyword;

		auto* iniManager = Settings::INI::Holder::GetSingleton();
		if (!iniManager) {
			logger::error("ItemListVisitor: Failed to cache ini settings.");
			return false;
		}

		const auto optFlagSpellBooks = iniManager->GetStoredSetting<bool>("General|bFlagUnreadSpellBooks");
		if (optFlagSpellBooks.has_value()) {
			flagSpellBooks = optFlagSpellBooks.value();
		}
		else {
			logger::error("ItemListVisitor: Invalid response from INI - General|bFlagUnreadSpellBooks");
			return false;
		}

		const auto optFlagSkillBooks = iniManager->GetStoredSetting<bool>("General|bFlagUnreadSkillBooks");
		if (optFlagSkillBooks.has_value()) {
			flagSkillBooks = optFlagSkillBooks.value();
		}
		else {
			logger::error("ItemListVisitor: Invalid response from INI - General|bFlagUnreadSkillBooks");
			return false;
		}

		const auto optInitialFunctionality = iniManager->GetStoredSetting<bool>("General|bOriginalFunctionality");
		if (optInitialFunctionality.has_value()) {
			originalFunctionality = optInitialFunctionality.value();
		}
		else {
			logger::error("ItemListVisitor: Invalid response from INI - General|bOriginalFunctionality");
			return false;
		}
		return true;
	}

	void ItemListVisitor::QueueTask(const RE::BSFixedString& a_menuName) {
		if (a_menuName.empty() || (
			a_menuName != RE::InventoryMenu::MENU_NAME &&
			a_menuName != RE::ContainerMenu::MENU_NAME &&
			a_menuName != RE::BarterMenu::MENU_NAME)) {
			return;
		}

		if (originalFunctionality) {
			SKSE::GetTaskInterface()->
				AddTask(reinterpret_cast<::TaskDelegate*>
					(new OriginalVisitor::ItemListVisitor(a_menuName)));
		}
		else {
			if (queued) {
				return;
			}
			m_menuName = a_menuName;
			queued = true;
			SKSE::GetTaskInterface()->AddTask(reinterpret_cast<::TaskDelegate*>(this));
		}
	}

	void ItemListVisitor::Visit() {
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
			RE::TESObjectBOOK* book = nullptr;

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
			case RE::FormType::Book:
				book = baseObject->As<RE::TESObjectBOOK>();
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
			else if (book) {
				EvaluateBook(book, item);
			}
		}

		SetBest();
	}

	void ItemListVisitor::SetBest() {
		for (uint64_t i = 0; i < 5; ++i) {
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
			else if (armor->HasKeyword(clothArmor->formID)) {
				secondary += ARMOR_CLOTH_START;
			}
			else if (armor->IsHeavyArmor()) {
				secondary += ARMOR_HEAVY_START;
			}
			else {
				secondary += ARMOR_LIGHT_START;
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

		for (auto* item : optionalFlags) {
			if (!item || !item->data.objDesc) {
				continue;
			}
			item->obj.SetMember("bestInClass", true);
		}
	}

	void ItemListVisitor::EvaluateArmor(RE::TESObjectARMO* a_armor,
		RE::InventoryEntryData* a_data,
		RE::ItemList::Item* a_item) {
		if (!a_data) {
			return;
		}

		uint64_t index = 0;

		bool worn = a_data->IsWorn();
		bool isClothing = a_armor->HasKeyword(clothArmor->formID);

		if (worn) {
			index = 0;
		}
		else if (a_armor->HasKeyword(heavyArmor->formID)) {
			index += ARMOR_HEAVY_START;
		}
		else if (a_armor->HasKeyword(lightArmor->formID)) {
			index += ARMOR_LIGHT_START;
		}
		else if (isClothing) {
			index += ARMOR_CLOTH_START;
		}
		else if (a_armor->IsHeavyArmor()) {
			index += ARMOR_HEAVY_START;
		}
		else if (a_armor->IsLightArmor()) {
			index += ARMOR_LIGHT_START;
		}
		else {
			return;
		}

		if (a_armor->HasKeyword(wornCuirass->formID) ||
			a_armor->HasKeyword(clothCuirass->formID)) {
			index += ARMOR_CUIRASS_INDEX;
		}
		else if (a_armor->HasKeyword(wornHelmet->formID) || 
			a_armor->HasKeyword(clothHelmet->formID)) {
			index += ARMOR_HEAD_INDEX;
		}
		else if (a_armor->HasKeyword(wornArms->formID) ||
			a_armor->HasKeyword(clothArms->formID)) {
			index += ARMOR_ARMS_INDEX;
		}
		else if (a_armor->HasKeyword(wornLegs->formID) ||
			a_armor->HasKeyword(clothLegs->formID)) {
			index += ARMOR_BOOTS_INDEX;
		}
		else if (a_armor->IsShield()) {
			index += ARMOR_SHIELD_INDEX;
		}
		else {
			return;
		}

		float value = isClothing ? 
			static_cast<float>(a_data->GetValue()) : 
			player->GetArmorValue(a_data);
		best.at(index).Compare(a_item, value);
	}

	void ItemListVisitor::EvaluateWeapon(RE::TESObjectWEAP* a_weap,
		RE::InventoryEntryData* a_data,
		RE::ItemList::Item* a_item) {
		uint64_t index = WEAPON_START_INDEX;
		auto* slot = a_weap->GetEquipSlot();
		if (!slot || !a_data) {
			return;
		}

		bool equipped = a_data->IsWorn();
		bool staff = a_weap->IsStaff();

		if (staff) {
			index += WEAPON_STAFF_INDEX;
		}
		else if (a_weap->IsRanged()) {
			index += WEAPON_RANGED_INDEX;
		}
		else if (slot == either) {
			index += WEAPON_ONEHANDED_INDEX;
		}
		else {
			index += WEAPON_TWOHANDED_INDEX;
		}

		index += equipped ? 0 : WEAPON_UNEQUIPPED_START;

		float value = staff ? a_data->GetValue() : player->GetDamage(a_data);
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

	void ItemListVisitor::EvaluateBook(RE::TESObjectBOOK* a_book, 
		RE::ItemList::Item* a_item) {
		if (!flagSpellBooks && !flagSkillBooks) {
			return;
		}
		if (!a_item) {
			return;
		}

		if (a_book->TeachesSpell() && flagSpellBooks) {
			const auto* bookSpell = a_book->GetSpell();
			if (bookSpell && !playerSpells.contains(bookSpell)) {
				optionalFlags.push_back(a_item);
			}
		}
		else if (a_book->TeachesSkill() && !a_book->IsRead() && flagSkillBooks) {
			optionalFlags.push_back(a_item);
		}
	}
}