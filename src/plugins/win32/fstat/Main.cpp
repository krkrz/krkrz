#include "ncbind/ncbind.hpp"
#include <string>
#include <vector>
using namespace std;
#include <tchar.h>
#include <shlobj.h>
#include <ole2.h>
#include <shellapi.h> // SHGetFileInfo

// Date クラスメンバ
static iTJSDispatch2 *dateClass   = NULL;  // Date のクラスオブジェクト
static iTJSDispatch2 *dateSetTime = NULL;  // Date.setTime メソッド
static iTJSDispatch2 *dateGetTime = NULL;  // Date.getTime メソッド

static const tjs_nchar * StoragesFstatPreScript	= TJS_N("\
global.FILE_ATTRIBUTE_READONLY = 0x00000001,\
global.FILE_ATTRIBUTE_HIDDEN = 0x00000002,\
global.FILE_ATTRIBUTE_SYSTEM = 0x00000004,\
global.FILE_ATTRIBUTE_DIRECTORY = 0x00000010,\
global.FILE_ATTRIBUTE_ARCHIVE = 0x00000020,\
global.FILE_ATTRIBUTE_NORMAL = 0x00000080,\
global.FILE_ATTRIBUTE_TEMPORARY = 0x00000100;");

NCB_TYPECONV_CAST_INTEGER(tjs_uint64);


/**
 * メソッド追加用
 */
class StoragesFstat {
	/**
	 * Win32APIの GetLastErrorのエラーメッセージを返す
	 * @param message メッセージ格納先
	 */
	static void getLastError(ttstr &message) {
		LPVOID lpMessageBuffer;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					   NULL, GetLastError(),
					   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					   (LPWSTR)&lpMessageBuffer, 0, NULL);
		message = ((tjs_char*)lpMessageBuffer);
		LocalFree(lpMessageBuffer);
	}

	/**
	 * ファイル時刻を Date クラスにして保存
	 * @param store 格納先
	 * @param filetime ファイル時刻
	 */
	static void storeDate(tTJSVariant &store, FILETIME const &filetime, iTJSDispatch2 *objthis)
	{
		// ファイル生成時
		tjs_uint64 ft = filetime.dwHighDateTime;
		ft *= 0x100000000;
		ft |= filetime.dwLowDateTime;
		if (ft > 0) {
			iTJSDispatch2 *obj;
			if (TJS_SUCCEEDED(dateClass->CreateNew(0, NULL, NULL, &obj, 0, NULL, objthis))) {
				// UNIX TIME に変換
				tjs_int64 unixtime = (ft - 0x19DB1DED53E8000 ) / 10000;
				tTJSVariant time(unixtime);
				tTJSVariant *param[] = { &time };
				dateSetTime->FuncCall(0, NULL, NULL, NULL, 1, param, obj);
				store = tTJSVariant(obj, obj);
				obj->Release();
			}
		}
	}
	/**
	 * Date クラスの時刻をファイル時刻に変換
	 * @param restore  参照先（Dateクラスインスタンス）
	 * @param filetime ファイル時刻結果格納先
	 * @return 取得できたかどうか
	 */
	static bool restoreDate(tTJSVariant &restore, FILETIME &filetime)
	{
		if (restore.Type() != tvtObject) return false;
		iTJSDispatch2 *date = restore.AsObjectNoAddRef();
		if (!date) return false;
		tTJSVariant result;
		if (dateGetTime->FuncCall(0, NULL, NULL, &result, 0, NULL, date) != TJS_S_OK) return false;
		tjs_uint64 ft = result.AsInteger();
		ft *= 10000;
		ft += 0x19DB1DED53E8000;
		filetime.dwLowDateTime  = (DWORD)( ft        & 0xFFFFFFFF);
		filetime.dwHighDateTime = (DWORD)((ft >> 32) & 0xFFFFFFFF);
		return true;
	}

	/**
	 * パスをローカル化する＆末尾の\を削除
	 * @param path パス名
	 */
	static void getLocalName(ttstr &path) {
		TVPGetLocalName(path);
		if (path.GetLastChar() == TJS_W('\\')) {
			tjs_int i,len = path.length();
			tjs_char* tmp = new tjs_char[len];
			const tjs_char* dp = path.c_str();
			for (i=0,len--; i<len; i++) tmp[i] = dp[i];
			tmp[i] = 0;
			path = tmp;
			delete[] tmp;
		}
	}

	/**
	 * ファイルハンドルを取得
	 * @param filename ファイル名（ローカル名であること）
	 * @param iswrite 読み書き選択
	 * @param out out_isdir ディレクトリかどうか
	 * @return ファイルハンドル
	 */
	static HANDLE _getFileHandle(ttstr const &filename, bool iswrite, bool *out_isdir = 0) {
		DWORD attr = GetFileAttributes(filename.c_str());
		bool isdir = (attr != 0xFFFFFFFF && (attr & FILE_ATTRIBUTE_DIRECTORY));
		if (out_isdir) *out_isdir = isdir;
		HANDLE hFile;
		if (iswrite) {
			hFile = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL ,
							   OPEN_EXISTING,    isdir ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL , NULL);
		} else {
			hFile = CreateFile(filename.c_str(), isdir ? READ_CONTROL               : GENERIC_READ, FILE_SHARE_READ, NULL ,
							   OPEN_EXISTING,    isdir ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL , NULL);
		}
		return hFile;
	}
	/**
	 * ファイルのタイムスタンプを取得する
	 * @param filename ファイル名（ローカル名であること）
	 * @param ctime 作成時刻
	 * @param atime アクセス時刻
	 * @param mtime 変更時刻
	 * @param size  ファイルサイズ
	 * @return 0:失敗 1:ファイル 2:フォルダ
	 */
	static int getFileTime(ttstr const &filename, tTJSVariant &ctime, tTJSVariant &atime, tTJSVariant &mtime, tTJSVariant *size = 0)
	{
		bool isdir = false;
		HANDLE hFile = _getFileHandle(filename, false, &isdir);
		if (hFile == INVALID_HANDLE_VALUE) return 0;

		if (!isdir && size != 0) {
			LARGE_INTEGER fsize;
			if (GetFileSizeEx(hFile, &fsize))
				*size = (tjs_int64)fsize.QuadPart;
		}
		FILETIME ftc, fta, ftm;
		if (GetFileTime(hFile , &ftc, &fta, &ftm)) {
			storeDate(ctime, ftc, NULL);
			storeDate(atime, fta, NULL);
			storeDate(mtime, ftm, NULL);
		}
		CloseHandle(hFile);
		return isdir ? 2 : 1;
	}
	/**
	 * ファイルのタイムスタンプを設定する
	 * @param filename ファイル名（ローカル名であること）
	 * @param ctime 作成時刻
	 * @param mtime 変更時刻
	 * @param atime アクセス時刻
	 * @return 0:失敗 1:ファイル 2:フォルダ
	 */
	static int setFileTime(ttstr const &filename, tTJSVariant &ctime, tTJSVariant &atime, tTJSVariant &mtime)
	{
		bool isdir = false;
		HANDLE hFile = _getFileHandle(filename, true, &isdir);
		if (hFile == INVALID_HANDLE_VALUE) return 0;

		FILETIME c, a, m;
		bool hasC = restoreDate(ctime, c);
		bool hasA = restoreDate(atime, a);
		bool hasM = restoreDate(mtime, m);

		BOOL r = SetFileTime(hFile, hasC?&c:0, hasA?&a:0, hasM?&m:0);
		if (r == 0) {
			ttstr mes;
			getLastError(mes);
			TVPAddLog(ttstr(TJS_W("setFileTime : ")) + filename + TJS_W(":") + mes);
		}
		CloseHandle(hFile);
		return (r == 0) ? 0 : isdir ? 2 : 1;
	}
	static tjs_error _getTime(tTJSVariant *result, tTJSVariant const *param, bool chksize) {
		// 実ファイルでチェック
		ttstr filename = TVPNormalizeStorageName(param->AsStringNoAddRef());
		getLocalName(filename);
		tTJSVariant size, ctime, atime, mtime;
		int sel = getFileTime(filename, ctime, atime, mtime, chksize ? &size : 0);
		if (sel > 0) {
			if (result) {
				iTJSDispatch2 *dict = TJSCreateDictionaryObject();
				if (dict != NULL) {
					if (chksize && sel == 1) dict->PropSet(TJS_MEMBERENSURE, L"size",  NULL, &size, dict);
					dict->PropSet(TJS_MEMBERENSURE, L"mtime", NULL, &mtime, dict);
					dict->PropSet(TJS_MEMBERENSURE, L"ctime", NULL, &ctime, dict);
					dict->PropSet(TJS_MEMBERENSURE, L"atime", NULL, &atime, dict);
					*result = dict;
					dict->Release();
				}
			}
			return TJS_S_OK;
		}

		TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + param->GetString()).c_str());
		return TJS_S_OK;
	}
public:
	StoragesFstat(){};

	static void clearStorageCaches() {
		TVPClearStorageCaches();
	}
	
	/**
	 * 指定されたファイルの情報を取得する
	 * @param filename ファイル名
	 * @return サイズ・時刻辞書
	 */
	static tjs_error TJS_INTF_METHOD fstat(tTJSVariant *result,
										   tjs_int numparams,
										   tTJSVariant **param,
										   iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;

		ttstr filename = TVPGetPlacedPath(*param[0]);
		if (filename.length() > 0 && wcschr(filename.c_str(), '>')) {
			// アーカイブ内ファイル
			IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
			if (in) {
				STATSTG stat;
				in->Stat(&stat, STATFLAG_NONAME);
				tTJSVariant size((tjs_int64)stat.cbSize.QuadPart);
				if (result) {
					iTJSDispatch2 *dict;
					if ((dict = TJSCreateDictionaryObject()) != NULL) {
						dict->PropSet(TJS_MEMBERENSURE, L"size",  NULL, &size, dict);
						*result = dict;
						dict->Release();
					}
				}
				in->Release();
				return TJS_S_OK;
			}
		}
		return _getTime(result, param[0], true);
	}
	/**
	 * 指定されたファイルのタイムスタンプ情報を取得する（アーカイブ内不可）
	 * @param filename ファイル名
	 * @param dict     時刻辞書
	 * @return 成功したかどうか
	 */
	static tjs_error TJS_INTF_METHOD getTime(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		return _getTime(result, param[0], false);
	}
	/**
	 * 指定されたファイルのタイムスタンプ情報を設定する
	 * @param filename ファイル名
	 * @param dict     時刻辞書
	 * @return 成功したかどうか
	 */
	static tjs_error TJS_INTF_METHOD setTime(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;

		ttstr filename = TVPNormalizeStorageName(param[0]->AsStringNoAddRef());
		getLocalName(filename);
		tTJSVariant size, ctime, atime, mtime;
		iTJSDispatch2 *dict = param[1]->AsObjectNoAddRef();
		if (dict != NULL) {
			dict->PropGet(0, L"ctime", NULL, &ctime, dict);
			dict->PropGet(0, L"atime", NULL, &atime, dict);
			dict->PropGet(0, L"mtime", NULL, &mtime, dict);
		}
		int sel = setFileTime(filename, ctime, atime, mtime);
		if (result) *result = (sel > 0);

		return TJS_S_OK;
	}

	/**
	 * 更新日時取得・設定（Dateを経由しない高速版）
	 * @param target 対象
	 * @param time 時間（64bit FILETIME数）
	 */
	static tjs_uint64 getLastModifiedFileTime(ttstr target) {
		ttstr filename = TVPNormalizeStorageName(target);
		getLocalName(filename);
		HANDLE hFile = _getFileHandle(filename, false);
		FILETIME ft;
		if (hFile == INVALID_HANDLE_VALUE) return 0;
		bool rs = !! GetFileTime(hFile, 0, 0, &ft);
		CloseHandle(hFile);
		if (!rs) return 0;
		tjs_uint64 ret = ft.dwHighDateTime;
		ret <<= 32;
		ret |= ft.dwLowDateTime;
		return ret;
	}
	static bool setLastModifiedFileTime(ttstr target, tjs_uint64 time) {
		ttstr filename = TVPNormalizeStorageName(target);
		getLocalName(filename);
		HANDLE hFile = _getFileHandle(filename, true);
		if (hFile == INVALID_HANDLE_VALUE) return false;
		FILETIME ft;
		ft.dwHighDateTime = (time >> 32) & 0xFFFFFFFF;
		ft.dwLowDateTime  =  time        & 0xFFFFFFFF;
		bool rs = !! SetFileTime(hFile, 0, 0, &ft);
		CloseHandle(hFile);
		return rs;
	}

	/**
	 * 吉里吉里のストレージ空間中のファイルを抽出する
	 * @param src 保存元ファイル
	 * @param dest 保存先ファイル
	 */
	static void exportFile(ttstr filename, ttstr storename) {
		IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
		if (in) {
			IStream *out = TVPCreateIStream(storename, TJS_BS_WRITE);
			if (out) {
				BYTE buffer[1024*16];
				DWORD size;
				while (in->Read(buffer, sizeof buffer, &size) == S_OK && size > 0) {			
					out->Write(buffer, size, &size);
				}
				out->Release();
				in->Release();
			} else {
				in->Release();
				TVPThrowExceptionMessage((ttstr(TJS_W("cannot open storefile: ")) + storename).c_str());
			}
		} else {
			TVPThrowExceptionMessage((ttstr(TJS_W("cannot open readfile: ")) + filename).c_str());
		}
	}

	/**
	 * 吉里吉里のストレージ空間中の指定ファイルを削除する。
	 * @param file 削除対象ファイル
	 * @return 実際に削除されたら true
	 * 実ファイルがある場合のみ削除されます
	 */
	static bool deleteFile(ttstr filename) {
		BOOL r = false;
		filename = TVPGetPlacedPath(filename);
		if (filename.length() && !wcschr(filename.c_str(), '>')) {
			TVPGetLocalName(filename);
			r	= DeleteFile(filename.c_str());
			if (r == FALSE) {
				ttstr mes;
				getLastError(mes);
				TVPAddLog(ttstr(TJS_W("deleteFile : ")) + filename + TJS_W(" : ") + mes);
			} else {
				// 削除に成功した場合はストレージキャッシュをクリア
				TVPClearStorageCaches();
			}
		}
		return !! r;
	}

	/**
	 * 吉里吉里のストレージ空間中の指定ファイルのサイズを変更する(切り捨てる)
	 * @param file ファイル
	 * @param size 指定サイズ
	 * @return サイズ変更できたら true
	 * 実ファイルがある場合のみ処理されます
	 */
	static bool truncateFile(ttstr filename, tjs_int size) {
		BOOL r = false;
		filename = TVPGetPlacedPath(filename);
		if (filename.length() && !wcschr(filename.c_str(), '>')) {
			TVPGetLocalName(filename);
			HANDLE hFile = _getFileHandle(filename, true);
			if (hFile != INVALID_HANDLE_VALUE) {
				LARGE_INTEGER ofs;
				ofs.QuadPart = size;
				if (SetFilePointerEx(hFile, ofs, NULL, FILE_BEGIN) &&
					SetEndOfFile(hFile)) {
					r = true;
				} else {
					ttstr mes;
					getLastError(mes);
					TVPAddLog(ttstr(TJS_W("truncateFile : ")) + filename + TJS_W(" : ") + mes);
				}
				CloseHandle(hFile);
			}
		}
		return !! r;
	}
	
	/**
	 * 指定ファイルを移動する。
	 * @param fromFile 移動対象ファイル
	 * @param toFile 移動先パス
	 * @return 実際に移動されたら true
	 * 移動対象ファイルが実在し、移動先パスにファイルが無い場合のみ移動されます
	 */
	static bool moveFile(ttstr fromFile, ttstr toFile) {
		BOOL r = false;
		if (fromFile.length() && !wcschr(fromFile.c_str(), '>')
			&& toFile.length() && !wcschr(toFile.c_str(), '>')) {
			TVPGetLocalName(fromFile);
			TVPGetLocalName(toFile);
			r	= MoveFile(fromFile.c_str(), toFile.c_str());
			if (r == FALSE) {
				ttstr mes;
				getLastError(mes);
				TVPAddLog(ttstr(TJS_W("moveFile : ")) + fromFile + ", " + toFile + TJS_W(" : ") + mes);
			} else {
				TVPClearStorageCaches();
			}
		}
		return !! r;
	}

	/**
	 * 指定ディレクトリのファイル一覧を取得する
	 * @param dir ディレクトリ名
	 * @return ファイル名一覧が格納された配列
	 */
	static tTJSVariant dirlist(tjs_char const *dir) {
		return _dirlist(dir, &setDirListFile);
	}

	/**
	 * 指定ディレクトリのファイル一覧と詳細情報を取得する
	 * @param dir ディレクトリ名
	 * @return ファイル情報一覧が格納された配列
	 *         [ %[ name:ファイル名, size, attrib, mtime, atime, ctime ], ... ]
	 * dirlistと違いnameにおいてフォルダの場合の末尾"/"追加がないので注意(attribで判定のこと)
	 */
	static tTJSVariant dirlistEx(tjs_char const *dir) {
		return _dirlist(dir, &setDirListInfo);
	}

	typedef bool (*DirListCallback)(iTJSDispatch2 *array, tjs_int count, ttstr const &file, WIN32_FIND_DATA const *data);
private:
	static tTJSVariant _dirlist(ttstr dir, DirListCallback cb)
	{
		// OSネイティブな表現に変換
		dir = TVPNormalizeStorageName(dir);
		if (dir.GetLastChar() != TJS_W('/')) {
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));
		}
		TVPGetLocalName(dir);

		// Array クラスのオブジェクトを作成
		iTJSDispatch2 * array = TJSCreateArrayObject();
		tTJSVariant result;

		try {
			ttstr wildcard = dir + "*.*";
			WIN32_FIND_DATA data;
			HANDLE handle = FindFirstFile(wildcard.c_str(), &data);
			if (handle != INVALID_HANDLE_VALUE) {
				tjs_int count = 0;
				do {
					ttstr file = data.cFileName;
					if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						// ディレクトリの場合は最後に / をつける
						file += "/";
					}
					if ((*cb)(array, count, file, &data)) count++;
				} while(FindNextFile(handle, &data));
				FindClose(handle);
			} else {
				TVPThrowExceptionMessage(TJS_W("Directory not found."));
			}
			result = tTJSVariant(array, array);
			array->Release();
		} catch(...) {
			array->Release();
			throw;
		}

		return result;
	}
	static bool setDirListFile(iTJSDispatch2 *array, tjs_int count, ttstr const &file, WIN32_FIND_DATA const *data) {
		// [dirlist] 配列に追加する
		tTJSVariant val(file);
		array->PropSetByNum(0, count, &val, array);
		return true;
	}
	static bool setDirListInfo(iTJSDispatch2 *array, tjs_int count, ttstr const &file, WIN32_FIND_DATA const *data) {
		// [dirlistEx] 配列に追加する
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		if (dict != NULL) try {
			{
				ttstr fname = data->cFileName;
				tTJSVariant name = fname;
				dict->PropSet(TJS_MEMBERENSURE, L"name", NULL, &name, dict);
			} {
				tjs_int64 fsize = data->nFileSizeHigh;
				fsize <<= 32;
				fsize  |= data->nFileSizeLow;
				tTJSVariant size = fsize;
				dict->PropSet(TJS_MEMBERENSURE, L"size", NULL, &size, dict);
			} {
				tTJSVariant attrib = (tjs_int)data->dwFileAttributes;
				dict->PropSet(TJS_MEMBERENSURE, L"attrib", NULL, &attrib, dict);
			} {
				tTJSVariant ctime, atime, mtime;
				storeDate(ctime, data->ftCreationTime,   NULL);
				storeDate(atime, data->ftLastAccessTime, NULL);
				storeDate(mtime, data->ftLastWriteTime,  NULL);
				dict->PropSet(TJS_MEMBERENSURE, L"mtime", NULL, &mtime, dict);
				dict->PropSet(TJS_MEMBERENSURE, L"ctime", NULL, &ctime, dict);
				dict->PropSet(TJS_MEMBERENSURE, L"atime", NULL, &atime, dict);
			}
			tTJSVariant val(dict, dict);
			array->PropSetByNum(0, count, &val, array);
			dict->Release();
		} catch(...) {
			dict->Release();
			throw;
		}
		return true;
	}

public:

	/**
	 * 指定ディレクトリを削除する
	 * @param dir ディレクトリ名
	 * @return 実際に削除されたら true
	 * 中にファイルが無い場合のみ削除されます
	 */
	static bool removeDirectory(ttstr dir) {

		if (dir.GetLastChar() != TJS_W('/')) {
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));
		}

		// OSネイティブな表現に変換
		dir = TVPNormalizeStorageName(dir);
		TVPGetLocalName(dir);

		BOOL	r = RemoveDirectory(dir.c_str());
		if(r == FALSE) {
			ttstr mes;
			getLastError(mes);
			TVPAddLog(ttstr(TJS_W("removeDirectory : ")) + dir + TJS_W(" : ") + mes);
		}
		return !! r;
	}

	/**
	 * ディレクトリの作成
	 * @param dir ディレクトリ名
	 * @return 実際に作成できたら true
	 */
	static bool createDirectory(ttstr dir)
	{
		if(dir.GetLastChar() != TJS_W('/'))
		{
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));
		}
		dir	= TVPNormalizeStorageName(dir);
		TVPGetLocalName(dir);
		BOOL	r = CreateDirectory(dir.c_str(), NULL);
		if (r == FALSE) {
			ttstr mes;
			getLastError(mes);
			TVPAddLog(ttstr(TJS_W("createDirectory : ")) + dir + TJS_W(" : ") + mes);
		}
		return !! r;
	}

	/**
	 * カレントディレクトリの変更
	 * @param dir ディレクトリ名
	 * @return 実際に作成できたら true
	 */
	static bool changeDirectory(ttstr dir)
	{
		if(dir.GetLastChar() != TJS_W('/'))
		{
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));
		}
		dir	= TVPNormalizeStorageName(dir);
		TVPGetLocalName(dir);
		BOOL	r = SetCurrentDirectory(dir.c_str());
		if (r == FALSE) {
			ttstr mes;
			getLastError(mes);
			TVPAddLog(ttstr(TJS_W("changeDirectory : ")) + dir + TJS_W(" : ") + mes);
		}
		return !! r;
	}
	
	/**
	 * ファイルの属性を設定する
	 * @param filename ファイル/ディレクトリ名
	 * @param attr 設定する属性
	 * @return 実際に変更できたら true
	 */
	static bool setFileAttributes(ttstr filename, DWORD attr)
	{
		filename	= TVPNormalizeStorageName(filename);
		TVPGetLocalName(filename);

		attr	= attr & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN |
			FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY);

		DWORD	orgattr = GetFileAttributes(filename.c_str());

		return SetFileAttributes(filename.c_str(), orgattr | attr) == TRUE;
	}

	/**
	 * ファイルの属性を解除する
	 * @param filename ファイル/ディレクトリ名
	 * @param attr 解除する属性
	 * @return 実際に変更できたら true
	 */
	static bool resetFileAttributes(ttstr filename, DWORD attr)
	{
		filename	= TVPNormalizeStorageName(filename);
		TVPGetLocalName(filename);

		attr	= attr & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN |
			FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY);

		DWORD	orgattr = GetFileAttributes(filename.c_str());

		return SetFileAttributes(filename.c_str(), orgattr & ~attr) == TRUE;
	}

	/**
	 * ファイルの属性を取得する
	 * @param filename ファイル/ディレクトリ名
	 * @return 取得した属性
	 */
	static DWORD getFileAttributes(ttstr filename)
	{
		filename	= TVPNormalizeStorageName(filename);
		TVPGetLocalName(filename);

		return GetFileAttributes(filename.c_str());
	}

	/**
	 * フォルダ選択ダイアログを開く
	 * @param window ウィンドウ
	 * @param caption キャプション
	 * @param initialDir 初期ディレクトリ
	 * @param rootDir ルートディレクトリ
	 */
	static tjs_error TJS_INTF_METHOD selectDirectory(
		tTJSVariant	*result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis)
	{
		if(numparams < 1)
			return TJS_E_BADPARAMCOUNT;

		iTJSDispatch2*	tmp;
		tTJSVariant		val;
		BROWSEINFO		bi;
		ITEMIDLIST*		rootidl	= NULL;
		tjs_char		folder[2048+1];
		memset(&bi, 0, sizeof(bi));

		//	HWND
		iTJSDispatch2*	elm	= param[0]->AsObjectNoAddRef();
		if(elm->IsValid(0, L"window", NULL, elm) == TJS_S_TRUE &&
			TJS_SUCCEEDED(elm->PropGet(0, L"window", NULL, &val, elm)))
		{
			HWND	owner	= NULL;
			if(val.Type() != tvtVoid)
			{
				tmp	= val.AsObjectNoAddRef();
				if(TJS_SUCCEEDED(tmp->PropGet(0, L"HWND", NULL, &val, tmp)))
					owner	= (HWND)val.AsInteger();
			}
			bi.hwndOwner	= owner != NULL ? owner : TVPGetApplicationWindowHandle();
		}
		else
			bi.hwndOwner	= NULL;

		//	title
		if(elm->IsValid(0, L"title", NULL, elm) == TJS_S_TRUE &&
			TJS_SUCCEEDED(elm->PropGet(0, L"title", NULL, &val, elm)))
		{
			ttstr	title	= val.AsStringNoAddRef();
			bi.lpszTitle	= title.c_str();
		}
		else
			bi.lpszTitle	= NULL;

		//	name
		bi.pszDisplayName	= NULL;
		bi.ulFlags	= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		if(elm->IsValid(0, L"name", NULL, elm) == TJS_S_TRUE &&
			TJS_SUCCEEDED(elm->PropGet(0, L"name", NULL, &val, elm)) &&
			!val.NormalCompare(ttstr(L"")))
		{
			ttstr	name	= TVPNormalizeStorageName(val.AsStringNoAddRef());
			TVPGetLocalName(name);
			_tcscpy_s(folder, 2048, name.c_str());
		}
		else
			folder[0]	= 0;

		//	root dir
		if(elm->IsValid(0, L"rootDir", NULL, elm) == TJS_S_TRUE &&
			TJS_SUCCEEDED(elm->PropGet(0, L"rootDir", NULL, &val, elm)))
		{
			ttstr	rootDir	= TVPNormalizeStorageName(val.AsStringNoAddRef());
			TVPGetLocalName(rootDir);
			rootidl	= Path2ITEMIDLIST(rootDir.c_str());
		}
		bi.pidlRoot	= rootidl;

		bi.lpfn		= SelectDirectoryCallBack;
		bi.lParam	= (LPARAM)folder;

		ITEMIDLIST*	pidl;
		if((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if(::SHGetPathFromIDList(pidl, folder) != TRUE)
			{
				if(result) *result = 0;
			}
			else
			{
				if(result) *result = TJS_S_TRUE;
				val	= folder;
				val = TVPNormalizeStorageName(val);
				elm->PropSet(TJS_MEMBERENSURE, L"name", NULL, &val, elm);
			}
			FreeITEMIDLIST(pidl);
		}
		else
			if(result) *result	= 0;
		FreeITEMIDLIST(rootidl);

		return TJS_S_OK;
	}

	/**
	 * ディレクトリの存在チェック
	 * @param directory ディレクトリ名
	 * @return ディレクトリが存在すれば true/存在しなければ -1/ディレクトリでなければ false
	 */
	static int isExistentDirectory(ttstr dir)
	{
		if(dir.GetLastChar() != TJS_W('/'))
		{
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));
		}
		dir	= TVPNormalizeStorageName(dir);
		TVPGetLocalName(dir);
		DWORD	attr = GetFileAttributes(dir.c_str());
#if 0
		if(attr == 0xFFFFFFFF)
			return -1;	//	存在しない
		else if((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			return true;	//	存在する
		else
			return false;	//	ディレクトリではない
#else
		if(attr == 0xFFFFFFFF)
			return false;	//	存在しない
		else if((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			return true;	//	存在する
		else
			return false;	//	ディレクトリではない
#endif
	}

	/**
	 * 吉里吉里のストレージ空間中の指定ファイルをコピーする
	 * @param from コピー元ファイル
	 * @param to コピー先ファイル
	 * @param failIfExist ファイルが存在するときに失敗するなら ture、上書きするなら false
	 * @return 実際に移動できたら true
	 */
	static bool copyFile(ttstr from, ttstr to, bool failIfExist)
	{
		from	= TVPGetPlacedPath(from);
        to = TVPNormalizeStorageName(to);
		if(from.length() && to.length() && !wcschr(from.c_str(), '>') && !wcschr(to.c_str(), '>'))
		{
			TVPGetLocalName(from);
			TVPGetLocalName(to);
			if(CopyFile(from.c_str(), to.c_str(), failIfExist)) {
				TVPClearStorageCaches();
				return true;
			}
		}
		return false;
	}

	/**
	 * パスの正規化を行わず吉里吉里のストレージ空間中の指定ファイルをコピーする
	 * @param from コピー元ファイル
	 * @param to コピー先ファイル
	 * @param failIfExist ファイルが存在するときに失敗するなら ture、上書きするなら false
	 * @return 実際に移動できたら true
	 */
	static bool copyFileNoNormalize(ttstr from, ttstr to, bool failIfExist)
	{
		from	= TVPGetPlacedPath(from);
		if(from.length() && to.length() && !wcschr(from.c_str(), '>') && !wcschr(to.c_str(), '>'))
		{
			TVPGetLocalName(from);
			TVPGetLocalName(to);
			if(CopyFile(from.c_str(), to.c_str(), failIfExist)) {
				TVPClearStorageCaches();
				return true;
			}
		}
		return false;
	}

	/**
	 * パスの正規化を行なわず、autoPathからの検索も行なわずに
	 * ファイルの存在確認を行う
	 * @param fileame ファイルパス
	 * @return ファイルが存在したらtrue
	 */
	static bool isExistentStorageNoSearchNoNormalize(ttstr filename) 
	{
		return TVPIsExistentStorageNoSearchNoNormalize(filename);
	}

	/**
	 * 表示名取得
	 * @param fileame ファイルパス
	 */
	static ttstr getDisplayName(ttstr filename)
	{
		filename = TVPNormalizeStorageName(filename);
		TVPGetLocalName(filename);
		if (filename != "") {
			SHFILEINFO finfo;
			if (SHGetFileInfo(filename.c_str(), 0, &finfo, sizeof finfo, SHGFI_DISPLAYNAME)) {
				filename = finfo.szDisplayName;
			} else {
				TVPThrowExceptionMessage(TJS_W("SHGetFileInfo failed"));
			}
		}
		return filename;
	}

private:
	//	指定のパスからITEMIDLISTを取得
	static ITEMIDLIST*	Path2ITEMIDLIST(const tjs_char* path)
	{
		IShellFolder* isf;
		ITEMIDLIST* pidl;
		if(SUCCEEDED(::SHGetDesktopFolder(&isf)))
		{
			ULONG	tmp;
			if(SUCCEEDED(isf->ParseDisplayName(NULL, NULL, (LPOLESTR)path, &tmp, &pidl, &tmp)))
				return pidl;
		}
		return NULL;
	}

	//	ITEMIDLISTを解放
	static void	FreeITEMIDLIST(ITEMIDLIST* pidl)
	{
		IMalloc*	im;
		if(::SHGetMalloc(&im) != NOERROR)
			im	= NULL;
		if(im != NULL)
			im->Free((void*)pidl);
	}

	//	SHBrowserForFolderのコールバック関数
	static int	CALLBACK SelectDirectoryCallBack(HWND hwnd, UINT msg, LPARAM lparam, LPARAM lpdata)
	{
		//	初期化時
		if(msg == BFFM_INITIALIZED)
		{
			//	初期フォルダを指定
			ITEMIDLIST*	pidl;
			pidl	= Path2ITEMIDLIST((tjs_char*)lpdata);
			if(pidl != NULL)
			{
				::SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)pidl);
				FreeITEMIDLIST(pidl);
			}

			//	最前面へ移動
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		}
		return 0;
	}

public:
	/**
	 * MD5ハッシュ値の取得
	 * @param filename 対象ファイル名
	 * @return ハッシュ値（32文字の16進数ハッシュ文字列（小文字））
	 */
	static tjs_error TJS_INTF_METHOD getMD5HashString(tTJSVariant *result,
													  tjs_int numparams,
													  tTJSVariant **param,
													  iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;

		ttstr filename = TVPGetPlacedPath(*param[0]);
		IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
		if (!in) TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + param[0]->GetString()).c_str());

		TVP_md5_state_t st;
		TVP_md5_init(&st);

		tjs_uint8 buffer[1024]; // > 16 digestバッファ兼ねる
		DWORD size = 0;
		while (in->Read(buffer, sizeof buffer, &size) == S_OK && size > 0) {
			TVP_md5_append(&st, buffer, (int)size);
		}
		in->Release();

		TVP_md5_finish(&st, buffer);

		tjs_char ret[32+1], *hex = TJS_W("0123456789abcdef");
		for (tjs_int i=0; i<16; i++) {
			ret[i*2  ] = hex[(buffer[i] >> 4) & 0xF];
			ret[i*2+1] = hex[(buffer[i]     ) & 0xF];
		}
		ret[32] = 0;
		if (result) *result = ttstr(ret);
		return TJS_S_OK;
	}

	/**
	 * パスの検索
	 * @param filename   検索対象ファイル名
	 * @param searchpath 検索対象パス（ローカル表記(c:\～等)で";"区切り，省略時はシステムのデフォルト検索パス）
	 * @return 見つからなかった場合はvoid，見つかった場合はファイルのフルパス(file://./～)
	 */
	static tjs_error TJS_INTF_METHOD searchPath(tTJSVariant *result,
												tjs_int numparams,
												tTJSVariant **param,
												iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		ttstr filename(*param[0]), searchpath;
		if (TJS_PARAM_EXIST(1)) searchpath = *param[1];
		WCHAR tmp[MAX_PATH+1];
		DWORD len = ::SearchPathW(searchpath.length() ? searchpath.c_str() : NULL,
								  filename.c_str(), NULL, MAX_PATH, tmp, NULL);
		if (len > 0) {
			// if (len > MAX_PATH) ...
			tmp[MAX_PATH] = 0;
			if (result) *result = TVPNormalizeStorageName(tmp);
		} else {
			// not found
			if (result) result->Clear();
		}
		return TJS_S_OK;
	}

	/*----------------------------------------------------------------------
	 * カレントディレクトリ
     ----------------------------------------------------------------------*/
	static ttstr getCurrentPath() {
		TCHAR crDir[MAX_PATH + 1];
		GetCurrentDirectory(MAX_PATH + 1 , crDir);
		ttstr result(crDir);
		return TVPNormalizeStorageName(result + L"\\");
	}
	
	static void setCurrentPath(ttstr path) {
		if (!changeDirectory(path)) {
			ttstr mes;
			getLastError(mes);
			TVPThrowExceptionMessage(TJS_W("setCurrentPath failed:%1"), mes);
		}
	}
};

NCB_ATTACH_CLASS(StoragesFstat, Storages) {
	NCB_METHOD(clearStorageCaches);
	RawCallback("fstat",               &Class::fstat,               TJS_STATICMEMBER);
	RawCallback("getTime",             &Class::getTime,             TJS_STATICMEMBER);
	RawCallback("setTime",             &Class::setTime,             TJS_STATICMEMBER);
	NCB_METHOD(getLastModifiedFileTime);
	NCB_METHOD(setLastModifiedFileTime);
	NCB_METHOD(exportFile);
	NCB_METHOD(deleteFile);
	NCB_METHOD(truncateFile);
	NCB_METHOD(moveFile);
	NCB_METHOD(dirlist);
	NCB_METHOD(dirlistEx);
	NCB_METHOD(removeDirectory);
	NCB_METHOD(createDirectory);
	NCB_METHOD(changeDirectory);
	NCB_METHOD(setFileAttributes);
	NCB_METHOD(resetFileAttributes);
	NCB_METHOD(getFileAttributes);
	RawCallback("selectDirectory",     &Class::selectDirectory,     TJS_STATICMEMBER);
	NCB_METHOD(isExistentDirectory);
	NCB_METHOD(copyFile);
	NCB_METHOD(copyFileNoNormalize);
	NCB_METHOD(isExistentStorageNoSearchNoNormalize);
	NCB_METHOD(getDisplayName);
	RawCallback("getMD5HashString",    &Class::getMD5HashString,    TJS_STATICMEMBER);
	RawCallback("searchPath",          &Class::searchPath,          TJS_STATICMEMBER);
	Property("currentPath", &Class::getCurrentPath, &Class::setCurrentPath);
	Method(TJS_W("getTemporaryName"), &TVPGetTemporaryName);
};

// テンポラリファイル処理用クラス
class TemporaryFiles
{
public:
	TemporaryFiles() {};

	~TemporaryFiles() {
		std::vector<HANDLE>::iterator it = handles.begin();
		while (it != handles.end()) {
			HANDLE h = *it;
			::CloseHandle(h);
		}
	}

	bool entry(ttstr filename) {
		return _entry(filename);
	}

	bool entryFolder(ttstr filename) {
		return _entry(filename, true);
	}
	
private:
	std::vector<HANDLE> handles;

	bool _entry(const ttstr &name, bool folder=false) {
		ttstr filename = TVPNormalizeStorageName(name);
		TVPGetLocalName(filename);
		if (filename.length()) {
			DWORD access = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
			DWORD flag = folder ? FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_DELETE_ON_CLOSE : FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE;
			HANDLE h = CreateFile(filename.c_str(),0,access,0,OPEN_EXISTING,flag,0);
			if (h != INVALID_HANDLE_VALUE) {
				handles.push_back(h);
				return true;
			}
		}
		return false;
	}
};

NCB_REGISTER_CLASS(TemporaryFiles) {
	Constructor();
	NCB_METHOD(entry);
	NCB_METHOD(entryFolder);
}

/**
 * 登録処理後
 */
static void PostRegistCallback()
{
	tTJSVariant var;
	TVPExecuteExpression(TJS_W("Date"), &var);
	dateClass = var.AsObject();
	var.Clear();
	TVPExecuteExpression(TJS_W("Date.setTime"), &var);
	dateSetTime = var.AsObject();
	var.Clear();
	TVPExecuteExpression(TJS_W("Date.getTime"), &var);
	dateGetTime = var.AsObject();
	var.Clear();
	TVPExecuteExpression(StoragesFstatPreScript);
}

#define RELEASE(name) name->Release();name= NULL

/**
 * 開放処理前
 */
static void PreUnregistCallback()
{
	RELEASE(dateClass);
	RELEASE(dateSetTime);
}

NCB_POST_REGIST_CALLBACK(PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK(PreUnregistCallback);
