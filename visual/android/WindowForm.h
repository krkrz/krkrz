// Form = new TTVPWindowForm

// 仮想ウィンドウとして実装してしまうのがいいか
// シングルウィンドウ環境では、1画面1ウィンドウのみとして、1つのウィンドウしかアクティブにならない。
// ウィンドウの切り替え方法が提供されていない場合は、進行不能になってしまうが諦めてもらう？
// DrawDevice はテクスチャに描画して、それを返すだけの実装がいいかな？ そのテクスチャが実質Windowとして。
// 理屈上はマルチウィンドウも可能になるように
/*
 * シングルウィンドウ系システムのために、マルチウィンドウは使えない方がやっぱりいいかな。
 * Window がほぼそのままディスプレイと紐づいているような
 */
class tTVPWindowForm {
	// Windowが有効かどうか、無効だとイベントが配信されない
	bool GetFormEnabled();

	// 閉じる
	void InvalidateClose();
	void SendCloseMessage()
	void Close();
	void OnCloseQueryCalled(bool b);

	// 定期的に呼び出されるので、定期処理があれば実行する
	void TickBeat()

	// アクティブ/デアクティブ化された時に、Windowがアクティブかどうかチェックされる
	bool GetWindowActive()

	// DrawDevice
	void ResetDrawDevice()

	// キー入力
	void InternalKeyDown(WORD key, tjs_uint32 shift);
	void OnKeyUp( WORD vk, int shift );
	void OnKeyPress( WORD vk, int repeat, bool prevkeystate, bool convertkey );

	// プライマリーレイヤーのサイズに合うように呼び出される。w/hはLayerWidth/LayerHeightに当たり、ズームを考慮して表示サイズを設定する
	void SetPaintBoxSize(tjs_int w, tjs_int h);

	// マウスカーソル
	void SetDefaultMouseCursor();
	void SetMouseCursor(tjs_int handle);
	void HideMouseCursor();
	void SetMouseCursorState(tTVPMouseCursorState mcs);
	tTVPMouseCursorState GetMouseCursorState() const
	// void ReleaseCapture();
	// マウスカーソル座標はLayer.cursorX/cursorYで得られるようになっているが、タッチスクリーンだと無意味なので使えなくなる
	void GetCursorPos(tjs_int &x, tjs_int &y);
	void SetCursorPos(tjs_int x, tjs_int y);

	// ヒント表示(Androidでは無効か)
	void SetHintText(iTJSDispatch2* sender, const ttstr &text);
	void SetHintDelay( tjs_int delay );
	tjs_int GetHintDelay() const;

	// IME 入力関係、EditViewを最前面に貼り付けて、そこで入力させるのが現実的かな、好きな位置に表示は画面狭いとあまり現実的じゃないかも
	// 入力タイトルを指定して、入力受付、確定文字が返ってくるスタイルの方がいいか、モーダルにはならないから、確定後イベント通知かな
	void SetAttentionPoint(tjs_int left, tjs_int top, const struct tTVPFont * font );
	void DisableAttentionPoint();
	void SetImeMode(tTVPImeMode mode);
	tTVPImeMode GetDefaultImeMode() const;
	void ResetImeMode();

	// Windowハンドル系メソッドはすべて不要
	// VideoOverlay で SetMessageDrainWindow に渡す Window ハンドル
	HWND GetSurfaceWindowHandle();
	// VideoOverlay の OwnerWindow として設定される Window ハンドル
	HWND GetWindowHandle();
	// Window::HWND プロパティ値として使用される。非Windows環境ではプラグインに渡すこと出来ないから不要か。ダミーでNULL返せばいい
	HWND GetWindowHandleForPlugin();

	// VideoOverlayで表示サイズを決めるためにズーム値を用いて引数値を拡大縮小する
	void ZoomRectangle( tjs_int & left, tjs_int & top, tjs_int & right, tjs_int & bottom);
	// VideoOverlayで表示位置を決めるため、フルスクリーン時の黒ふちを考慮したleft/top位置を得る
	void GetVideoOffset(tjs_int &ofsx, tjs_int &ofsy);

	// プラグインでメッセージレシーバを登録するに使われる、Androidでは無効
	void RegisterWindowMessageReceiver(tTVPWMRRegMode mode, void * proc, const void *userdata);


	// 内容更新
	void UpdateWindow(tTVPUpdateType type = utNormal);

	// 表示/非表示
	bool GetVisible() const;
	void SetVisibleFromScript(bool b);
	void ShowWindowAsModal();

	// タイトル、Activityのタイトルに設定できるが、無意味かな
	std::wstring GetCaption();
	void GetCaption( std::wstring& v ) const;
	void SetCaption( const std::wstring& v );

	// サイズや位置など
	// 位置はAndroidでは無効か、常に0を返し、設定もスルーなど
	void SetLeft( int l );
	int GetLeft() const;
	void SetTop( int t );
	int GetTop() const;
	void SetPosition( int l, int t );
	// サイズ
	void SetWidth( int w );
	int GetWidth() const;
	void SetHeight( int h );
	int GetHeight() const;
	void SetSize( int w, int h );
	// 最小、最大サイズ関係、Androidなどリサイズがないとしたら無効か
	void SetMinWidth( int v )
	int GetMinWidth() const
	void SetMinHeight( int v )
	int GetMinHeight()
	void SetMinSize( int w, int h )
	void SetMaxWidth( int v )
	int GetMaxWidth()
	void SetMaxHeight( int v )
	int GetMaxHeight()
	void SetMaxSize( int w, int h )

	// 内部のサイズ、実質的にこれが表示領域サイズ
	void SetInnerWidth( int w );
	int GetInnerWidth() const;
	void SetInnerHeight( int h );
	int GetInnerHeight() const;
	void SetInnerSize( int w, int h );

	// 境界サイズ、無効
	void SetBorderStyle( enum tTVPBorderStyle st);
	enum tTVPBorderStyle GetBorderStyle() const;

	// 常に最前面表示、無効
	void SetStayOnTop( bool b );
	bool GetStayOnTop() const;
	// 最前面へ移動
	void BringToFront();

	// フルスクリーン、無効と言うか常に真
	void SetFullScreenMode(bool b);
	bool GetFullScreenMode() const;

	//マウスキー(キーボードでのマウスカーソル操作)は無効
	void SetUseMouseKey(bool b);
	bool GetUseMouseKey() const;

	// 他ウィンドウのキー入力をトラップするか、無効
	void SetTrapKey(bool b);
	bool GetTrapKey() const;

	// ウィンドウマスクリージョンは無効
	void SetMaskRegion(HRGN threshold);
	void RemoveMaskRegion();

	// フォースは常に真
	void SetFocusable(bool b);
	bool GetFocusable() const;

	// 表示ズーム関係
	void SetZoom(tjs_int numer, tjs_int denom, bool set_logical = true);
	void SetZoomNumer( tjs_int n );
	tjs_int GetZoomNumer() const;
	void SetZoomDenom(tjs_int d);
	tjs_int GetZoomDenom() const;

	// タッチ入力関係
	void SetTouchScaleThreshold( double threshold )
	double GetTouchScaleThreshold() const
	void SetTouchRotateThreshold( double threshold )
	double GetTouchRotateThreshold() const
	tjs_real GetTouchPointStartX( tjs_int index ) const;
	tjs_real GetTouchPointStartY( tjs_int index ) const;
	tjs_real GetTouchPointX( tjs_int index ) const;
	tjs_real GetTouchPointY( tjs_int index ) const;
	tjs_int GetTouchPointID( tjs_int index ) const;
	tjs_int GetTouchPointCount() const;

	// タッチ入力のマウスエミュレートON/OFF
	void SetEnableTouch( bool b );
	bool GetEnableTouch() const;

	// タッチ、マウス加速度
	bool GetTouchVelocity( tjs_int id, float& x, float& y, float& speed ) const
	bool GetMouseVelocity( float& x, float& y, float& speed ) const
	void ResetMouseVelocity()

	// 画面表示向き取得
	int GetDisplayOrientation();
	int GetDisplayRotate();
};

