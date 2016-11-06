
#ifndef __READ_OPTION_DESC_H__
#define __READ_OPTION_DESC_H__

struct tTVPCommandOptionsValue {
	tjs_string Value;
	tjs_string Description;
	bool IsDefault;
};
struct tTVPCommandOption {
	enum ValueType {
		VT_Select,
		VT_String,
		VT_Unknown
	};
	tjs_string Caption;
	tjs_string Description;
	tjs_string Name;
	ValueType Type;
	tjs_int Length;
	tjs_string Value;
	std::vector<tTVPCommandOptionsValue> Values;
	bool User;
};
struct tTVPCommandOptionCategory {
	tjs_string Name;
	std::vector<tTVPCommandOption> Options;
};
struct tTVPCommandOptionList {
	std::vector<tTVPCommandOptionCategory> Categories;
};

extern tTVPCommandOptionList* TVPGetPluginCommandDesc( const tjs_char* name );
extern tTVPCommandOptionList* TVPGetEngineCommandDesc();
void TVPMargeCommandDesc( tTVPCommandOptionList& dest, const tTVPCommandOptionList& src );

#endif // __READ_OPTION_DESC_H__
