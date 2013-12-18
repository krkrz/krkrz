#include "tp_stub.h"

// dialog.hpp 用の各種設定

struct DialogConfig {

	// サイズ型
	typedef int SizeT;

	// 文字列参照用
	typedef tjs_char const * NameT;

	// 文字列型（c_str()でNameTを返すこと）
	typedef ttstr StringT;

};

