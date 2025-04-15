#pragma once

#include "RE/Offset.h"

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

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct BestInContainer
		{
			static void Install();
			static void Thunk(void* a1);

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct BestInBarter
		{
			static void Install();
			static void Thunk(void* a1);

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};
	};

	void Install();
}