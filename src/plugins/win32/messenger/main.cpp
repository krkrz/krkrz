#include <windows.h>
#include <tchar.h>
#include "ncbind/ncbind.hpp"
#include <map>
using namespace std;

// 吉里吉里のウインドウクラス
#define KRWINDOWCLASS _T("TTVPWindowForm")
#define KEYSIZE 256

// -------------------------------------------------------------------
// アトム処理
// -------------------------------------------------------------------

// 確保したアトム情報
static map<ttstr,ATOM> *atoms = NULL;

// 名前からシステムグローバルアトム取得
static ATOM getAtom(const TCHAR *str)
{
	ttstr name(str);
	map<ttstr,ATOM>::const_iterator n = atoms->find(name);
	if (n != atoms->end()) {
		return n->second;
	}
	ATOM atom = GlobalAddAtom(str);
	(*atoms)[name] = atom;
	return atom;
}

// アトムから名前を取得する
static void getKey(tTJSVariant &key, ATOM atom)
{
	TCHAR buf[KEYSIZE+1];
	UINT len = GlobalGetAtomName(atom, buf, KEYSIZE);
	if (len > 0) {
		buf[len] = '\0';
		key = buf;
	}
}

// -------------------------------------------------------------------
// 処理本体
// -------------------------------------------------------------------

/**
 * メソッド追加用クラス
 */
class WindowMsg {

protected:
	iTJSDispatch2 *objthis; //< オブジェクト情報の参照
	bool messageEnable;     //< メッセージ処理が有効かどうか

	// --------------------------------------------------------

	typedef bool (__stdcall *NativeReceiver)(iTJSDispatch2 *obj, void *userdata, tTVPWindowMessage *Message);
	
	// ユーザ定義レシーバ情報
	struct ReceiverInfo {
		tTJSVariant userData;
		tTJSVariant receiver;
		// デフォルトコンストラクタ
		ReceiverInfo() {};
		// コンストラクタ
		ReceiverInfo(tTJSVariant &receiver, tTJSVariant &userData) : receiver(receiver), userData(userData) {}
		// コピーコンストラクタ
		ReceiverInfo(const ReceiverInfo &orig) {
			userData = orig.userData;
			receiver = orig.receiver;
		}
		// デストラクタ
		~ReceiverInfo(){}

		// 実行
		bool exec(iTJSDispatch2 *obj, tTVPWindowMessage *message) {
			switch (receiver.Type()) {
			case tvtObject:
				{
					tTJSVariant result;
					tTJSVariant wparam = (tjs_int)message->WParam;
					tTJSVariant lparam = (tjs_int)message->LParam;
					tTJSVariant *p[] = {&userData, &wparam, &lparam};
					receiver.AsObjectClosureNoAddRef().FuncCall(0, NULL, NULL, &result, 3, p, NULL);
					return (int)result != 0;
				}
				break;
			case tvtString:
				{
					tTJSVariant result;
					tTJSVariant wparam = (tjs_int)message->WParam;
					tTJSVariant lparam = (tjs_int)message->LParam;
					tTJSVariant *p[] = {&userData, &wparam, &lparam};
					obj->FuncCall(0, receiver.GetString(), NULL, &result, 3, p, obj);
					return (int)result != 0;
				}
				break;
			case tvtInteger:
				{
					NativeReceiver receiverNative = (NativeReceiver)(tjs_int)receiver;
					return receiverNative(obj, (void*)(tjs_int)userData, message);
				}
				break;
			}
			return false;
		}
	};
	
	map<unsigned int, ReceiverInfo> receiverMap;

	// ユーザ規定レシーバの削除
	void removeUserReceiver(unsigned int Msg) {
		map<unsigned int, ReceiverInfo>::const_iterator n = receiverMap.find(Msg);
		if (n != receiverMap.end()) {
			receiverMap.erase(Msg);
		}
	}
	
	// ユーザ規定レシーバの登録
	void addUserReceiver(unsigned int Msg, tTJSVariant &receiver, tTJSVariant &userdata) {
		removeUserReceiver(Msg);
		receiverMap[Msg] = ReceiverInfo(receiver, userdata);
	}

	// --------------------------------------------------------

	ttstr storeKey;         //< HWND 保存指定キー
	
	/**
	 * 実行ファイルがある場所に HWND 情報を保存する
	 */
	void storeHWND(HWND hwnd) {
		if (storeKey != "") {
			tTJSVariant varScripts;
			TVPExecuteExpression(TJS_W("System.exeName"), &varScripts);
			ttstr path = varScripts;
			path += ".";
			path += storeKey;
			IStream *stream = TVPCreateIStream(path, TJS_BS_WRITE);
			if (stream != NULL) {
				char buf[100];
				DWORD len;
				_snprintf(buf, sizeof buf, "%d", (int)hwnd);
				stream->Write(buf, strlen(buf), &len);
				stream->Release();
			}
		}
	}
	
	/**
	 * メッセージ受信関数本体
	 * @param userdata ユーザデータ(この場合ネイティブオブジェクト情報)
	 * @param Message ウインドウメッセージ情報
	 */
	static bool __stdcall MyReceiver(void *userdata, tTVPWindowMessage *Message) {

		iTJSDispatch2 *obj = (iTJSDispatch2*)userdata; // Window のオブジェクト
		// 吉里吉里の内部処理の関係でイベント処理中は登録破棄後でも呼ばれることがあるので
		// Window の本体オブジェクトからネイティブオブジェクトを取り直す
		WindowMsg *self = ncbInstanceAdaptor<WindowMsg>::GetNativeInstance(obj);
		if (self == NULL) {
			return false;
		}
		switch (Message->Msg) {
		case TVP_WM_DETACH: // ウインドウが切り離された
			break; 
		case TVP_WM_ATTACH: // ウインドウが設定された
			self->storeHWND((HWND)Message->LParam);
			break;
		case WM_COPYDATA: // 外部からの通信
			{
				COPYDATASTRUCT *copyData = (COPYDATASTRUCT*)Message->LParam;
				tTJSVariant key;
				getKey(key, (ATOM)copyData->dwData);
				tTJSVariant msg((const tjs_char *)copyData->lpData);
				tTJSVariant *p[] = {&key, &msg};
				obj->FuncCall(0, L"onMessageReceived", NULL, NULL, 2, p, obj);
			}
			return true;
		default:
			{
				map<unsigned int, ReceiverInfo>::iterator n = self->receiverMap.find(Message->Msg);
				if (n != self->receiverMap.end()) {
					return n->second.exec(obj, Message);
				}
			}
			break;
		}
		return false;
	}

	void doStoreKey() {
		if (storeKey != "") {
			tTJSVariant val;
			objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
			storeHWND(reinterpret_cast<HWND>((tjs_int)(val)));
		}
	}
	
	/**
	 * レシーバの登録
	 */
	void registReceiver(bool enable) {
		// レシーバ更新
		tTJSVariant mode    = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)MyReceiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)objthis;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		int ret = objthis->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 3, p, objthis);
	}

public:
	// コンストラクタ
	WindowMsg(iTJSDispatch2 *objthis) : objthis(objthis), messageEnable(false) {}

	// デストラクタ
	~WindowMsg() {
		receiverMap.clear();
		// レシーバを解放
		registReceiver(false);
	}

	/**
	 * メッセージ受信が有効かどうかを設定
	 * @param enable true なら有効
	 */
	void setMessageEnable(bool enable) {
		if (messageEnable != enable) {
			messageEnable = enable;
			registReceiver(messageEnable);
			if (messageEnable) {
				doStoreKey();
			}
		}
	}
	
	/**
	 * @return メッセージ受信が有効かどうかを取得
	 */
	bool getMessageEnable() {
		return messageEnable;
	}

	/**
	 * storeKey を指定
	 * この値を指定すると、HWND の値が 実行ファイル名.key名 として保存されるようになります
	 * @param keyName HWND保存用キー
	 */
	void setStoreKey(const tjs_char *keyName) {
		if (storeKey != keyName) {
			storeKey = keyName;
			if (messageEnable) {
				doStoreKey();
			}
		}
	}

	/**
	 * @return storeKey を取得
	 */
	const tjs_char *getStoreKey() {
		return storeKey.c_str();
	}

	/**
	 * 外部プラグインからのメッセージ処理ロジックの登録
	 * @param mode 登録モード
	 * @param msg
	 * @param func
	 */
	static tjs_error TJS_INTF_METHOD registerUserMessageReceiver(tTJSVariant *result,
																 tjs_int numparams,
																 tTJSVariant **param,
																 WindowMsg *self) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		int mode         = (tjs_int)*param[0];
		unsigned int msg;
		if (param[1]->Type() == tvtString) {
			msg = RegisterWindowMessage(param[1]->GetString());
		} else {
			msg = (unsigned int)(tTVInteger)*param[1];
		}
		if (mode == wrmRegister) {
			if (numparams < 4) return TJS_E_BADPARAMCOUNT;
			self->addUserReceiver(msg, *param[2], *param[3]);
		} else if (mode == wrmUnregister) {
			self->removeUserReceiver(msg);
		}
		if (result) {
			*result = (tjs_int)msg;
		}
		return TJS_S_OK;
	}

	// 送信メッセージ情報
	struct UserMsgInfo {
		HWND hWnd;
		unsigned int msg;
		WPARAM wparam;
		LPARAM lparam;
		UserMsgInfo(HWND hWnd, unsigned int msg, WPARAM wparam, LPARAM lparam) : hWnd(hWnd), msg(msg), wparam(wparam), lparam(lparam) {}
	};

	/**
	 * 個別窓へのメッセージ送信処理
	 * @param hWnd 送信先ウインドウハンドラ
	 * @param parent 送信情報
	 */
	static BOOL CALLBACK enumWindowsProcUser(HWND hWnd, LPARAM parent) {
		UserMsgInfo *info = (UserMsgInfo*)parent;
		TCHAR buf[100];
		GetClassName(hWnd, buf, sizeof buf);
		if (info->hWnd != hWnd && _tcscmp(buf, KRWINDOWCLASS) == 0) {
			SendMessage(hWnd, info->msg, info->wparam, info->lparam);
		}
		return TRUE;
	}
	
	/**
	 * ユーザ定義メッセージ送信処理
	 * 起動している吉里吉里すべてにメッセージを送信します
	 * @param msg メッセージID
	 * @param wparam WPARAM値
	 * @param lparam LPARAM値
	 */
	void sendUserMessage(unsigned int msg, tjs_int wparam, tjs_int lparam) {
		tTJSVariant val;
		objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
		UserMsgInfo info(reinterpret_cast<HWND>((tjs_int)(val)), msg, (WPARAM)wparam, (LPARAM)lparam);
		EnumWindows(enumWindowsProcUser, (LPARAM)&info);
	}
	
	// --------------------------------------------------------
	
	// 送信メッセージ情報
	struct MsgInfo {
		HWND hWnd;
		COPYDATASTRUCT copyData;
		MsgInfo(HWND hWnd, const TCHAR *key, const tjs_char *msg) : hWnd(hWnd) {
			copyData.dwData = getAtom(key);
			copyData.cbData = (TJS_strlen(msg) + 1) * sizeof(tjs_char);
			copyData.lpData = (PVOID)msg;
		}
	};

	/**
	 * 個別窓へのメッセージ送信処理
	 * @param hWnd 送信先ウインドウハンドラ
	 * @param parent 送信情報
	 */
	static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM parent) {
		MsgInfo *info = (MsgInfo*)parent;
		TCHAR buf[100];
		GetClassName(hWnd, buf, sizeof buf);
		if (info->hWnd != hWnd && _tcscmp(buf, KRWINDOWCLASS) == 0) {
			SendMessage(hWnd, WM_COPYDATA, (WPARAM)info->hWnd, (LPARAM)&info->copyData);
		}
		return TRUE;
	}

	/**
	 * メッセージ送信処理
	 * 起動している吉里吉里すべてにメッセージを送信します
	 * @param key 識別キー
	 * @param msg メッセージ
	 */
	void sendMessage(const TCHAR *key, const tjs_char *msg) {
		tTJSVariant val;
		objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
		MsgInfo info(reinterpret_cast<HWND>((tjs_int)(val)), key, msg);
		EnumWindows(enumWindowsProc, (LPARAM)&info);
	}

};

// インスタンスゲッタ
NCB_GET_INSTANCE_HOOK(WindowMsg)
{
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		return obj;
	}
};

// フックつきアタッチ
NCB_ATTACH_CLASS_WITH_HOOK(WindowMsg, Window) {
	Property(L"messageEnable", &WindowMsg::getMessageEnable, &WindowMsg::setMessageEnable);
	Property(L"storeHWND", &WindowMsg::getStoreKey, &WindowMsg::setStoreKey);
	RawCallback("registerUserMessageReceiver", &WindowMsg::registerUserMessageReceiver, 0);
	Method(L"sendUserMessage", &WindowMsg::sendUserMessage);
	Method(L"sendMessage", &WindowMsg::sendMessage);
}

/**
 * 登録処理前
 */
void PreRegistCallback()
{
	atoms = new map<ttstr,ATOM>;
}

/**
 * 開放処理後
 */
void PostUnregistCallback()
{
	map<ttstr,ATOM>::const_iterator i = atoms->begin();
	while (i != atoms->end()) {
		GlobalDeleteAtom(i->second);
		i++;
	}
	delete atoms;
	atoms = NULL;
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
