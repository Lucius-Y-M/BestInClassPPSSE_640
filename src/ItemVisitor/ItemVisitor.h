#pragma once 

namespace ItemVisitor
{
	class ItemListVisitor final : public SKSE::detail::TaskDelegate
	{
	public:
		static ItemListVisitor* GetSingleton();

		void Run() override;
		void Dispose() override;

		bool PreloadForms();
		void QueueTask();

		ItemListVisitor(const ItemListVisitor&) = delete;
		ItemListVisitor(ItemListVisitor&&) = delete;
		ItemListVisitor& operator=(const ItemListVisitor&) = delete;
		ItemListVisitor& operator=(ItemListVisitor&&) = delete;

	protected:
		ItemListVisitor() = default;
		~ItemListVisitor() = default;

	private:
		void Visit();
		void SetBest();

		void EvaluateArmor(RE::TESObjectARMO* a_armor, 
			RE::InventoryEntryData* a_data, 
			RE::ItemList::Item* a_item);

		void EvaluateWeapon(RE::TESObjectWEAP* a_weap,
			RE::InventoryEntryData* a_data,
			RE::ItemList::Item* a_item);

		void EvaluateAmmo(RE::TESAmmo* a_weap,
			RE::InventoryEntryData* a_data,
			RE::ItemList::Item* a_item);

		struct StoredObject
		{
			RE::ItemList::Item* item{ nullptr };

			float value{ -1.0f };

			void Compare(RE::ItemList::Item* a_item, float a_rhs) {
				if (a_rhs > value || !item) {
					value = a_rhs;
					item = a_item;
				}
			}
		};

		bool queued{ false };

		// Cached variables
		RE::PlayerCharacter* player;

		RE::BGSKeyword* heavyArmor{ nullptr };
		RE::BGSKeyword* lightArmor{ nullptr };
		RE::BGSKeyword* clothArmor{ nullptr };
		
		RE::BGSEquipSlot* either{ nullptr };

		RE::BGSKeyword* wornCuirass{ nullptr };
		RE::BGSKeyword* wornHelmet{ nullptr };
		RE::BGSKeyword* wornArms{ nullptr };
		RE::BGSKeyword* wornLegs{ nullptr };

		// Class vars
		RE::BSTArray<RE::ItemList::Item*> _list{};
		RE::BSFixedString m_menuName{ ""sv };

		// Ammo definitions
		static constexpr uint64_t ARMOR_CUIRASS_INDEX = 0;
		static constexpr uint64_t ARMOR_HEAD_INDEX = 1;
		static constexpr uint64_t ARMOR_ARMS_INDEX = 2;
		static constexpr uint64_t ARMOR_BOOTS_INDEX = 3;
		static constexpr uint64_t ARMOR_SHIELD_INDEX = 4;

		static constexpr uint64_t ARMOR_HEAVY_START = 5;
		static constexpr uint64_t ARMOR_LIGHT_START = 10;
		static constexpr uint64_t ARMOR_CLOTH_START = 15;

		static constexpr uint64_t ARRAY_ARMOR_SIZE = 20;

		// Weapon definitions
		static constexpr uint64_t WEAPON_START_INDEX = ARRAY_ARMOR_SIZE;

		static constexpr uint64_t WEAPON_ONEHANDED_INDEX = 0;
		static constexpr uint64_t WEAPON_TWOHANDED_INDEX = 1;
		static constexpr uint64_t WEAPON_RANGED_INDEX = 2;

		static constexpr uint64_t WEAPON_UNEQUIPPED_START = 3;

		static constexpr uint64_t ARRAY_WEAPON_SIZE = 6;

		// Ammo definitions
		static constexpr uint64_t AMMO_START_INDEX = WEAPON_START_INDEX + ARRAY_WEAPON_SIZE;

		static constexpr uint64_t AMMO_BOLT = 0;
		static constexpr uint64_t AMMO_ARROW = 1;

		static constexpr uint64_t AMMO_UNEQUIPPED_START = 2;

		static constexpr uint64_t ARRAY_AMMO_SIZE = 4;

		static constexpr uint64_t ARRAY_SIZE = AMMO_START_INDEX + ARRAY_AMMO_SIZE;

		std::array<StoredObject, ARRAY_SIZE> best{};
	};
}