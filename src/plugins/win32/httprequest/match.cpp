#include <tchar.h>
#include <string>
#include <regex>

typedef std::basic_string<TCHAR> tstring;
typedef std::tr1::basic_regex<TCHAR> tregex;
typedef std::tr1::match_results<tstring::const_iterator> tmatch;

// Content-Type メタタグをマッチングする正規表現。大文字小文字は無視
static tregex regctype(_T("<meta[ \\t]+http-equiv=(\\\"content-type\\\"|'content-type'|content-type)[ \\t]+content=(\\\"[^\\\"]*\\\"|'[^']*'|[^ \\t>]+).*>"), tregex::icase);

/**
 * text 中から Content-Type のメタタグを探して、指定されてる値 (content=) を返す。
 * @param text 探査対象
 * @param ctype 結果格納先
 */
bool
matchContentType(tstring &text, tstring &ctype)
{
	tmatch result;
	if (std::tr1::regex_search(text, result, regctype)) {
		tstring str = result.str(2);
		int len = str.size();
		const TCHAR *buf = str.c_str();
		if (len > 0) {
			if (buf[0] == '\'' || buf[0] == '"') {
				// クオートされてる場合はそれを取り除く
				ctype = tstring(buf+1, len-2);
			} else {
				ctype = tstring(buf, len);
			}
			return true;
		}
	}
	return false;
}
