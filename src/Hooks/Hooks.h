#pragma once

namespace Hooks 
{
	class BestInClassListener
	{
	public:
		static BestInClassListener* GetSingleton();
		void                        Install();
		void                        SetMemberIfBestInClass();

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
			static void Thunk(void* a1);

			inline static std::ptrdiff_t offset{ 0x400 };
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct BestInContainer
		{
			static void Install();
			static void Thunk(void* a1);

			inline static std::ptrdiff_t offset{ 0x4CE };
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct BestInBarter
		{
			static void Install();
			static void Thunk(void* a1);

			inline static std::ptrdiff_t offset{ 0x300 };
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct InvalidateListData
		{
			static void Install();
			static void ThunkInventory(void* a1, void* a2, void* a3);
			static void ThunkContainer(void* a1, void* a2, void* a3);
			static void ThunkBarter(void* a1, void* a2, void* a3);

			inline static std::ptrdiff_t offsetInventory{ 0x452 };
			inline static std::ptrdiff_t offsetContainer{ 0x53c };
			inline static std::ptrdiff_t offsetBarter{ 0x36d };

			inline static REL::Relocation<decltype(&ThunkInventory)> _funcInventory;
			inline static REL::Relocation<decltype(&ThunkInventory)> _funcContainer;
			inline static REL::Relocation<decltype(&ThunkInventory)> _funcBarter;
		};
	};

	void Install();
}