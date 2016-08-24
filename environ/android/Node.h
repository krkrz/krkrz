

#ifndef __NODE_H__
#define __NODE_H__

class tTJSNI_Node : public tTJSNativeInstance {
protected:
	iTJSDispatch2 *Owner;
	tTVPScreen* screen_;

	tjs_real x_;
	tjs_real y_;
	tjs_real width_;
	tjs_real height_;

	// 毎回計算するか、保持したままにするか……
	tjs_real real_x_;	// 実際の位置X
	tjs_real real_y_;	// 実際の位置Y
	tjs_real real_width_;	// 実際の幅
	tjs_real real_height_;	// 実際の高さ

	tTJSNI_Node *Parent;
	tObjectList<tTJSNI_Node> Children;

protected:
	void updateRealSize() {
		tjs_int original = screen_->getOriginalSize();
		tjs_int view = screen_->getViewSize();

		real_x_ = x_ * view / original;
		real_y_ = y_ * view / original;
		real_width_ = width_ * view / original;
		real_height_ = height_ * view / original;
	}

public:
	tTJSNI_Node();
	virtual ~tTJSNI_Node();

	virtual tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	virtual void TJS_INTF_METHOD Invalidate();

	/**
	 * 自身をparent に追加する
	 * @param parent 親ノード
	 */
	void Join(tTJSNI_Node *parent);
	/**
	 * 現在の親から分離する
	 */
	void Part();
	/**
	 * child を自身の子として追加する
	 * @param child : 追加するノード
	 */
	void AddChild(tTJSNI_Node *child);
	/**
	 * child を自身の子から削除する
	 * @param child : 削除するノード
	 */
	void RemoveChild(tTJSNI_Node *child);

	/**
	 * @param original : 基準サイズ。このサイズを想定して作られている
	 * @param view : 表示サイズ。フルスクリーン化等で大きくなる。
	 * view / original が拡大率
	 * 変更時、すべてのノードに通知が行く
	 */
	virtual void OnChangeRealSize( tjs_int original, tjs_int view ) {
		updateRealSize();
	}
	/*
	virtual void OnPreDraw();
	virtual void OnPostDraw();
	*/

	virtual void OnDraw( NodeDrawDevice* drawdevice );

	virtual tTJSNI_Node* OnHitTest( tjs_int x, tjs_int y ) {
		return this;
	}
	virtual tTJSNI_Node* HitTest( tjs_int x, tjs_int y ) {
		if( ( x >= real_x_ && x < (real_x_+real_width_) ) &&
			( y >= real_y_ && y < (real_y_+real_height_) ) ) {
			return OnHitTest( x, y );
		}
		return NULL;
	}
};


#endif // __NODE_H__
