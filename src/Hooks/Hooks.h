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

		/// <summary>
		/// Unused. Was meant to be a workaround for equipping from merchants and containers.
		/// </summary>
		struct EquipFromContainer
		{
			static void Install();
			static void Thunk(void* a1, void* a2, void* a3);

			inline static std::ptrdiff_t offset{ 0x196 };
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct BestInBarter
		{
			static void Install();
			static void Thunk(void* a1);

			inline static std::ptrdiff_t offset{ 0x300 };
			inline static REL::Relocation<decltype(&Thunk)> _func;
		};
	};

	void Install();
}