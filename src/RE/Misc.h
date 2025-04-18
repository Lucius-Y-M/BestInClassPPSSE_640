#pragma once

#include "Offset.h"

namespace RE
{
	inline void InvalidateListData(GFxMovieView* a_list, const char* a_method, FxResponseArgs<0>& a_responseArgs)
	{
		using func_t = decltype(&InvalidateListData);
		static REL::Relocation<func_t> func{ Offset::GFxValue::Invoke };
		return func(a_list, a_method, a_responseArgs);
	}
}