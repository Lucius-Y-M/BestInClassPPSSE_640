#pragma once

namespace ItemVisitor
{
	class ItemListInvalidator final : public SKSE::detail::TaskDelegate
	{
	public:
		static ItemListInvalidator* GetSingleton();

		void Run() override;
		void Dispose() override;

		void QueueTask(const RE::BSFixedString& a_menuName);

		ItemListInvalidator(const ItemListInvalidator&) = delete;
		ItemListInvalidator(ItemListInvalidator&&) = delete;
		ItemListInvalidator& operator=(const ItemListInvalidator&) = delete;
		ItemListInvalidator& operator=(ItemListInvalidator&&) = delete;

	protected:
		ItemListInvalidator() = default;
		~ItemListInvalidator() = default;

	private:
		bool queued{ false };
		RE::BSFixedString m_menuName{ "" };
	};
}