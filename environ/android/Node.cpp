

#include "tjsCommHead.h"
#include "Node.h"


tTJSNI_Node::tTJSNI_Node()
: Owner(NULL), drawdevice_(NULL), Parent(NULL)
, x_(0), y_(0), width_(0), height_(0)
, real_x_(0), real_y_(0), real_width_(0), real_height_(0)
{
}
tTJSNI_Node::~tTJSNI_Node() {
}

// Node( nodedrawdevice, parent )
tjs_error TJS_INTF_METHOD tTJSNI_Node::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;

	Owner = tjs_obj; // no addref

	// get the drawdevice native instance
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	if(clo.Object == NULL) TVPThrowExceptionMessage(TJS_W("NodeDrawDeviceを指定してください"));

	tTJSVariant iface_v;
	if(TJS_FAILED(clo.PropGet(0, TJS_W("interface"), NULL, &iface_v, NULL)))
		TVPThrowExceptionMessage( TJS_W("Cannot Retrive Interface.") );
	drawdevice_ = reinterpret_cast<NodeDrawDevice *>((long)(tjs_int64)iface_v);

	// get the node native instance
	clo = param[1]->AsObjectClosureNoAddRef();
	tTJSNI_Node *node = NULL;
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Node::ClassID, (iTJSNativeInstance**)&node)))
			TVPThrowExceptionMessage(TJS_W("Nodeオブジェクトを指定してください"));
	}

	if( node ) {
		Join( node );
	} else {
		drawdevice_->AttachRoot( this );
		Parent = NULL;
	}
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Node::Invalidate() {
	// 自身がルートなら解除
	drawdevice_->DetachRoot( this );

	// part from the parent
	Part();

	// sever all children
	tObjectListSafeLockHolder<tTJSNI_Node> holder(Children);
	tjs_int count = Children.GetSafeLockedObjectCount(); \
	for( tjs_int i = 0; i < count; i++ ) {
		tTJSNI_Node* child = Children.GetSafeLockedObjectAt(i);
		if( !child ) continue;
		child->Part();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Node::OnDraw( NodeDrawDevice* drawdevice ) {
	tjs_uint count = Children.GetCount();
	for( tjs_uint i = 0; i < count; i++ ) {
		Children[i]->OnDraw( drawdevice );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Node::Join(tTJSNI_Node *parent) {
	if( parent == this )
		TVPThrowExceptionMessage(TVPCannotSetParentSelf);

	if( Parent ) Part();
	Parent = parent;
	if( Parent ) parent->AddChild(this);
}
//---------------------------------------------------------------------------
void tTJSNI_Node::Part() {
	if( Parent != NULL ) {
		Parent->RemoveChild(this);
		Parent = NULL;
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Node::AddChild(tTJSNI_Node *child)
{
	Children.Add(child);
}
//---------------------------------------------------------------------------
void tTJSNI_Node::RemoveChild(tTJSNI_Node *child)
{
	Children.Remove(child);
}
//---------------------------------------------------------------------------

