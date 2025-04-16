#pragma once

struct BestValueStorage
{
	// shifted masks to unroll the combined equipment slots
	// also refer to https://github.com/Dakraid/BestInClassPP/issues/1
	enum class MaskedArmorSlot : uint32_t
	{
		kHelmet = 1,
		kBody = 2,
		kHand = 3,
		kFeet = 7,
		kShield = 9
	};

	// light 5; heavy 5; cloth 4
	std::pair<RE::ItemList::Item*, float> Armor[14];
	// animation type 10
	std::pair<RE::ItemList::Item*, int> Weapon[10];
	// arrow 1; bolt 1
	std::pair<RE::ItemList::Item*, float> Ammo[2];
};

namespace OriginalVisitor {
	class ItemListVisitor final : public SKSE::detail::TaskDelegate
	{
	public:
		// SeaSparrow: The original fetches the item list when the task is created, not ran.
		// this led to some crashes.
		explicit ItemListVisitor(const RE::BSFixedString& a_menuName, bool a_skyUIPresent);

		~ItemListVisitor() = default;

		void Run() override;
		void Dispose() override;

	private:
		using Slot = BestValueStorage::MaskedArmorSlot;


		void Visit();

		void CompareArmor(RE::ItemList::Item* a_item, 
			RE::TESBoundObject* a_base);
		void CompareWeapon(RE::ItemList::Item* a_item,
			RE::TESBoundObject* a_base);
		void CompareAmmo(RE::ItemList::Item* a_item,
			RE::TESBoundObject* a_base);

		Slot UnmaskToLowest(uint32_t a_slot);

		void SetBest();

		RE::BSFixedString menuName{ "" };
		RE::BSTArray<RE::ItemList::Item*> _list{};
		BestValueStorage _bestStore;

		bool skyUIPresent{ false };
	};
}