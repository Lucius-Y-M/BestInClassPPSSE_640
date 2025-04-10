#pragma once

#include "RE/Offset.h"

namespace Hooks {

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

	class ItemVisitor final : public SKSE::detail::TaskDelegate {
	public:
		explicit ItemVisitor(RE::BSTArray<RE::ItemList::Item*> a_itemList);
		~ItemVisitor() = default;

		void Run() override;
		void Dispose() override;

	private:
		using Slot = BestValueStorage::MaskedArmorSlot;

		void Visit();

		void CompareArmor(RE::ItemList::Item* a_item);
		void CompareWeapon(RE::ItemList::Item* a_item);
		void CompareAmmo(RE::ItemList::Item* a_item);

		Slot UnmaskToLowest(uint32_t a_slot);

		void SetBest();

		RE::BSTArray<RE::ItemList::Item*> _list{};
		BestValueStorage _bestStore;
	};

	class BestInClassListener
	{
	public:
		static BestInClassListener* GetSingleton();
		void                        Install();
		void                        SetMemberIfBestInClass(const RE::BSFixedString& a_menuName);

		BestInClassListener(const BestInClassListener&) = delete;
		BestInClassListener(BestInClassListener&&) = delete;
		BestInClassListener& operator=(const BestInClassListener&) = delete;
		BestInClassListener& operator=(BestInClassListener&&) = delete;

	protected:
		BestInClassListener() = default;
		~BestInClassListener() = default;
		
	private:
		struct BestInClassCall
		{
			static void Install();
			static void Thunk(void* a1, RE::FxDelegate* a_delegate, const char* a_funcName, void* a4, bool a_flag);

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};
	};

	void Install();
}