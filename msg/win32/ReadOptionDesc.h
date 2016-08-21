
#ifndef __READ_OPTION_DESC_H__
#define __READ_OPTION_DESC_H__

struct tTVPCommandOptionsValue {
	std::wstring Value;
	std::wstring Description;
	bool IsDefault;
};
struct tTVPCommandOption {
	enum ValueType {
		VT_Select,
		VT_String,
		VT_Unknown
	};
	std::wstring Caption;
	std::wstring Description;
	std::wstring Name;
	ValueType Type;
	tjs_int Length;
	std::wstring Value;
	std::vector<tTVPCommandOptionsValue> Values;
	bool User;
};
struct tTVPCommandOptionCategory {
	std::wstring Name;
	std::vector<tTVPCommandOption> Options;
};
struct tTVPCommandOptionList {
	std::vector<tTVPCommandOptionCategory> Categories;
};

extern tTVPCommandOptionList* TVPGetPluginCommandDesc( const wchar_t* name );
extern tTVPCommandOptionList* TVPGetEngineCommandDesc();
void TVPMargeCommandDesc( tTVPCommandOptionList& dest, const tTVPCommandOptionList& src );

#endif // __READ_OPTION_DESC_H__
