#include <windows.h>

#include "dialog_config.hpp"

struct DialogTemplate {
	// 設定から型をエイリアスする
	typedef DialogConfig::SizeT   SizeT;
	typedef DialogConfig::NameT   NameT;
	typedef DialogConfig::StringT StringT;

	virtual ~DialogTemplate() {}

	// データの書き出し
	// c に必要サイズを加算する（アライン分の予備だけ余分に加算されることがある）
	// p == 0 ならデータは書かない
	virtual void Write(BYTE* &p, SizeT &c) = 0;

	// バッファ出力＆サイズ計算用 ヘルパ関数

	// 1バイト書き出し
	static inline void WriteByte(BYTE* &p, SizeT &c, BYTE v) {
		c++;
		if (p) *p++ = v;
	}
	// 2バイト書き出し
	static inline void WriteWord(BYTE* &p, SizeT &c, WORD v) {
		c += 2;
		if (p) {
			*p++ = (BYTE)( v     & 0xFF);
			*p++ = (BYTE)((v>>8) & 0xFF);
		}
	}
	// 4バイト書き出し
	static inline void WriteDWord(BYTE* &p, SizeT &c, DWORD v) {
		c += 4;
		if (p) {
			*p++ = (BYTE)( v      & 0xFF);
			*p++ = (BYTE)((v>> 8) & 0xFF);
			*p++ = (BYTE)((v>>16) & 0xFF);
			*p++ = (BYTE)((v>>24) & 0xFF);
		}
	}

	// 文字列書き出し
	static inline void WriteString(BYTE* &p, SizeT &c, NameT r) {
		while (*r) WriteWord(p, c, *r++);
		/**/       WriteWord(p, c, 0);
	}

	// アラインメント
	static inline void Alignment(BYTE* &p, SizeT &c, SizeT al) {
		c += al; // 大きい分には問題ないのでサイズ計算用にはアライン分を足す
		if (p) {
			ULONG n = al-1;
			p = (BYTE*) (((ULONG)p + n) & ~n);
		}
	}

	// sz_Or_Ord 型：文字列かIDのどちらか
	//  0x0000 : 要素なし
	//  0xFFFF 0x???? : ID
	//  その他 : Null文字で終わるUnicode文字列

	typedef enum { SZORD_NONE, SZORD_ID, SZORD_STR } SZORD;
	struct sz_Or_Ord{
		SZORD sel;
		WORD id;
		StringT str;
		sz_Or_Ord() : sel(SZORD_NONE) {}
		inline void SetString(NameT s) { sel = SZORD_STR; str = s; }
		inline void SetID(WORD n)      { sel = SZORD_ID;  id  = n; }
		inline void Write(BYTE* &p, SizeT &c) {
			switch (sel) {
			case SZORD_ID:  WriteWord(  p, c, 0xFFFF); WriteWord(p, c, id); break;
			case SZORD_STR: WriteString(p, c, str.c_str()); break;
			default:        WriteWord(  p, c, 0); break;
			}
		}
	};
};

struct DialogHeader : public DialogTemplate {
	// DLGTEMPLATEEX; // 配置は必ず DWORD アラインのこと
	WORD dlgVer;			// バージョン（常に1）
	WORD signature;			// シグネチャ（常に0xFFFF）
	DWORD helpID;			// コンテキストヘルプID
	DWORD exStyle;			// exスタイル
	DWORD style;			// スタイル
	WORD dlgItems;			// アイテムの個数
	short x;				// x (dialog unit)
	short y;				// y (dialog unit)
	short cx;				// w (dialog unit)
	short cy;				// h (dialog unit)

	sz_Or_Ord menu;			// メニューID
	sz_Or_Ord windowClass;	// ウィンドウクラス
	StringT title;			// ダイアログボックスのタイトル(Unicode)

	// 以下は DS_SETFONT が指定された場合のみ存在
	short pointSize;		// フォントのサイズ
	short weight;			// フォントのウェイト(0～1000)
	BYTE  italic;			// フォントのイタリック(TRUEorFALSE)
	BYTE  charset;			// フォントのキャラセット
	StringT typeFace;		// フォントのタイプフェイス

	// 初期値
	DialogHeader()
		:   dlgVer(1),
			signature(0xFFFF),
			helpID(0),
			exStyle(0),
			style(0),
			dlgItems(0),
			x(0),
			y(0),
			cx(0),
			cy(0),
			pointSize(0),
			weight(0),
			italic(0),
			charset(0)
		{}

	// 書き出し
	void Write(BYTE* &p, SizeT &c) {
		Alignment( p, c, 4);

		WriteWord( p, c, dlgVer);
		WriteWord( p, c, signature);
		WriteDWord(p, c, helpID);
		WriteDWord(p, c, exStyle);
		WriteDWord(p, c, style);
		WriteWord( p, c, dlgItems);
		WriteWord( p, c, (WORD)x);
		WriteWord( p, c, (WORD)y);
		WriteWord( p, c, (WORD)cx);
		WriteWord( p, c, (WORD)cy);
		menu       .Write(p, c);
		windowClass.Write(p, c);
		WriteString(p, c, title.c_str());

		if (style & DS_SETFONT) {
			WriteWord( p, c, (WORD)pointSize);
			WriteWord( p, c, (WORD)weight);
			WriteByte( p, c, (BYTE)italic);
			WriteByte( p, c, (BYTE)charset);
			WriteString(p, c, typeFace.c_str());
		}
	}
};

struct DialogItems : public DialogTemplate {
	// DLGITEMTEMPLATEEX; // 配置は必ず DWORD アラインのこと
	DWORD helpID;			// コンテキストヘルプID
	DWORD exStyle;			// exスタイル
	DWORD style;			// スタイル
	short x;				// x (dialog unit)
	short y;				// y (dialog unit)
	short cx;				// w (dialog unit)
	short cy;				// h (dialog unit)
	DWORD id;				// このコントロールID

	sz_Or_Ord windowClass;	// ウィンドウクラス
	sz_Or_Ord title;		// タイトル
	WORD extraCount;		// 拡張データのサイズ（WORDアライン）
	BYTE const *extraData;

	DialogItems()
		:   helpID(0),
			exStyle(0),
			style(0),
			x(0),
			y(0),
			cx(0),
			cy(0),
			id(0),
			extraCount(0),
			extraData(0)
		{}

	// 書き出し
	void Write(BYTE* &p, SizeT &c) {
		Alignment( p, c, 4);

		WriteDWord(p, c, helpID);
		WriteDWord(p, c, exStyle);
		WriteDWord(p, c, style);
		WriteWord( p, c, (WORD)x);
		WriteWord( p, c, (WORD)y);
		WriteWord( p, c, (WORD)cx);
		WriteWord( p, c, (WORD)cy);
		WriteDWord(p, c, id);
		windowClass.Write(p, c);
		title      .Write(p, c);
		WriteWord(p, c, extraCount);
		if (extraCount > 0) {
			BYTE const *r = extraData;
			for (long i = 0; i < (long)extraCount; i++)
				WriteByte(p, c, *r++);

			Alignment( p, c, 2);
		}
	}
};

