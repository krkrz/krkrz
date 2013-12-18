#include "ncbind/ncbind.hpp"
#include "sqlite3.h"

//
// SQL拡張
// 
// cnt(a,b)   文字列 a に b が含まれれば真
// ncnt(a,b)  正規化された文字列 a に 正規化された文字列 b が含まれてれば真

static const tjs_char *normalizeBefore = 
L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ"
L"ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ"
L"１２３４５６７８９０"
L"あいうえおかきくけこさしすせそたちつてとなにぬねの"
L"はひふへほまみむめもやゆよらりるれろわゐゑをんぁぃぅぇぉっゃゅょ"
L"がぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽ"
L"アイウエオカキクケコサシスセソタチツテトナニヌネノ"
L"ハヒフヘホマミムメモヤユヨラリルレロワヰヱヲンァィゥェォッャュョ"
L"ガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポ"
L"ｧｨｩｪｫｯｬｭｮ"
L"ー・、。ｰ"
L"[]{}"
L"，．：；？！´｀＾￣＿〇ー―‐／＼～"
L"｜‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－×＝"
L"＜＞￥＄％＃＆＊＠★●◎◆■▲▼※";

static const tjs_char *normalizeAfter = 
L"abcdefghijklmnopqrstuvwxyz"
L"abcdefghijklmnopqrstuvwxyz"
L"abcdefghijklmnopqrstuvwxyz"
L"1234567890"
L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉ"
L"ﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｲｴｦﾝｱｲｳｴｵﾂﾔﾕﾖ"
L"ｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾊﾋﾌﾍﾎﾊﾋﾌﾍﾎ"
L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉ"
L"ﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｲｴｦﾝｱｲｳｴｵﾂﾔﾕﾖ"
L"ｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾊﾋﾌﾍﾎﾊﾋﾌﾍﾎ"
L"ｱｲｳｴｵﾂﾔﾕﾖ"
L"-･,.-"
L"()()"
L",.:;?!'`^~_◯---/＼-"
L"|`'\"\"()()()()()()｢｣｢｣()+-x="
L"<>\\$%#&*@☆○○◇□△▽*";

// 消去文字
static const tjs_char *clearChar = L"ﾞ゛゜";

// 正規化テーブル
static tjs_char normalizeData[65536];

static void
normalize(ttstr &store, const tjs_char *str)
{
	while (*str) {
		store += normalizeData[*str++];
	}
}

void
initNormalize()
{
	// 正規化用データの初期化
	for (int i=0;i<65536;i++) {
		normalizeData[i] = i;
	}
	const tjs_char *p = normalizeBefore;
	const tjs_char *q = normalizeAfter;
	while (*p) {
		normalizeData[*p++] = *q++;
	}
	// 消去文字
	p = clearChar;
	while (*p) {
		normalizeData[*p++] = 0;
	}
}

static void
cntFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	if (argc < 2) return;
	sqlite3_result_int(context, wcsstr((const tjs_char*)sqlite3_value_text16(argv[0]),
									   (const tjs_char*)sqlite3_value_text16(argv[1])) != NULL);
}

static void
ncntFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	if (argc < 2) return;
	ttstr a,b;
	normalize(a, (const tjs_char*)sqlite3_value_text16(argv[0]));
	normalize(b, (const tjs_char*)sqlite3_value_text16(argv[1]));
	sqlite3_result_int(context, wcsstr(a.c_str(), b.c_str()) != NULL);
}

void
initContainFunc(sqlite3 *db)
{
	// 比較関数の登録
	sqlite3_create_function(db, "cnt",  2, SQLITE_ANY, NULL, cntFunc,  NULL, NULL);
	sqlite3_create_function(db, "ncnt", 2, SQLITE_ANY, NULL, ncntFunc, NULL, NULL);
}
