#pragma once

namespace Hooks 
{
	class BestInClassListener
	{
	public:
		static BestInClassListener* GetSingleton();
		void                        Install();
		void                        ProcessEquipEvent();
		void                        SetMemberIfBestInClass(const RE::BSFixedString& a_menuName);

		BestInClassListener(const BestInClassListener&) = delete;
		BestInClassListener(BestInClassListener&&) = delete;
		BestInClassListener& operator=(const BestInClassListener&) = delete;
		BestInClassListener& operator=(BestInClassListener&&) = delete;

	protected:
		BestInClassListener() = default;
		~BestInClassListener() = default;
		
	private:
		struct BestInInventory
		{
			static void Install();
			static void Thunk(RE::InventoryMenu* a_this);

			inline static std::ptrdiff_t offset{ 0x400 };
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct BestInContainer
		{
			static void Install();
			static void Thunk(RE::ContainerMenu* a_this);

			inline static std::ptrdiff_t offset{ 0x49E };	// 1130+ = 0x4CE; this is a crucial diff
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct BestInBarter
		{
			static void Install();
			static void Thunk(RE::BarterMenu* a_this);

			inline static std::ptrdiff_t offset{ 0x300 };
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct InvalidateListData
		{
			static void Install();
			static void ThunkInventory(RE::GFxMovieView* a_list,
				const char* a_methodName,
				void* a_responseArgs);
			static void ThunkContainer(RE::GFxMovieView* a_list,
				const char* a_methodName,
				void* a_responseArgs);
			static void ThunkBarter(RE::GFxMovieView* a_list,
				const char* a_methodName,
				void* a_responseArgs);

			inline static std::ptrdiff_t offsetInventory{ 0x452 };
			inline static std::ptrdiff_t offsetContainer{ 0x53c };
			inline static std::ptrdiff_t offsetBarter{ 0x36d };

			inline static REL::Relocation<decltype(&ThunkInventory)> _funcInventory;
			inline static REL::Relocation<decltype(&ThunkInventory)> _funcContainer;
			inline static REL::Relocation<decltype(&ThunkInventory)> _funcBarter;
		};

		RE::BSFixedString lastMenuName{ "" };
	};

	void Install();
}
