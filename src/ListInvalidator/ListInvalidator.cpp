#include "ListInvalidator.h"

#include "RE/Misc.h"

namespace ItemVisitor
{
	ItemListInvalidator* ItemListInvalidator::GetSingleton() {
		static ItemListInvalidator singleton;
		return std::addressof(singleton);
	}

	void ItemListInvalidator::Run() {
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}

		RE::ItemList* itemList = nullptr;
		if (m_menuName == RE::BarterMenu::MENU_NAME) {
			auto menu = ui->GetMenu<RE::BarterMenu>();
			if (!menu) {
				logger::error("Failed to get menu."sv);
				return;
			}
			itemList = menu->itemList;
		}
		else if (m_menuName == RE::ContainerMenu::MENU_NAME) {
			auto menu = ui->GetMenu<RE::ContainerMenu>();
			if (!menu) {
				logger::error("Failed to get menu."sv);
				return;
			}
			itemList = menu->itemList;
		}
		else if (m_menuName == RE::InventoryMenu::MENU_NAME) {
			auto menu = ui->GetMenu<RE::InventoryMenu>();
			if (!menu) {
				logger::error("Failed to get menu."sv);
				return;
			}
			itemList = menu->itemList;
		}
		else {
			return;
		}

		if (!itemList || !itemList->view) {
			logger::error("ItemList pointer is null, this is likely an error."sv);
			return;
		}

		auto response = RE::FxResponseArgs<0>();
		RE::InvalidateListData(itemList->view.get(), "InvalidateListData", response);
	}

	void ItemListInvalidator::Dispose() {
		queued = false;
		m_menuName = "";
	}

	void ItemListInvalidator::QueueTask(const RE::BSFixedString& a_menuName) {
		if (queued) {
			return;
		}
		queued = true;
		m_menuName = a_menuName;
		SKSE::GetTaskInterface()->AddTask(reinterpret_cast<::TaskDelegate*>(this));
	}
}