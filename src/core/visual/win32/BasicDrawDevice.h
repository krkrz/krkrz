
#ifndef BASIC_DRAW_DEVICE_H
#define BASIC_DRAW_DEVICE_H

#include "DrawDevice.h"
#include <d3d9.h>

//---------------------------------------------------------------------------
//! @brief		�uBasic�v�f�o�C�X(�����Ƃ���{�I�ȕ`����s���݂̂̃f�o�C�X)
//---------------------------------------------------------------------------
class tTVPBasicDrawDevice : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

	HWND TargetWindow;
	bool IsMainWindow;
	bool DrawUpdateRectangle;
	bool BackBufferDirty;
	bool IsFullScreened;

	IDirect3D9*				Direct3D;
	IDirect3DDevice9*		Direct3DDevice;
	IDirect3DTexture9*		Texture;
	D3DPRESENT_PARAMETERS	D3dPP;
	D3DDISPLAYMODE			DispMode;

	UINT	CurrentMonitor;
	void*	TextureBuffer; //!< �e�N�X�`���̃T�[�t�F�[�X�ւ̃������|�C���^
	long	TexturePitch; //!< �e�N�X�`���̃s�b�`

	tjs_uint TextureWidth; //!< �e�N�X�`���̉���
	tjs_uint TextureHeight; //!< �e�N�X�`���̏c��

	bool ShouldShow; //!< show �Ŏ��ۂɉ�ʂɉ摜��]�����ׂ���

	tjs_uint VsyncInterval;

public:
	tTVPBasicDrawDevice(); //!< �R���X�g���N�^

private:
	~tTVPBasicDrawDevice(); //!< �f�X�g���N�^

	void InvalidateAll();

	UINT GetMonitorNumber( HWND window );

	bool IsTargetWindowActive() const;

	bool GetDirect3D9Device();
	HRESULT InitializeDirect3DState();
	HRESULT DecideD3DPresentParameters();

	bool CreateD3DDevice();
	void DestroyD3DDevice();

	bool CreateTexture();
	void DestroyTexture();

	void TryRecreateWhenDeviceLost();
	void ErrorToLog( HRESULT hr );
	void CheckMonitorMoved();

public:
	void SetToRecreateDrawer() { DestroyD3DDevice(); }

public:
	void EnsureDevice();

//---- LayerManager �̊Ǘ��֘A
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);

//---- �`��ʒu�E�T�C�Y�֘A
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);

//---- �ĕ`��֘A
	virtual void TJS_INTF_METHOD Show();
	virtual bool TJS_INTF_METHOD WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed );

//---- LayerManager ����̉摜�󂯓n���֘A
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

//---- �f�o�b�O�x��
	virtual void TJS_INTF_METHOD SetShowUpdateRect(bool b);

//---- �t���X�N���[��
	virtual bool TJS_INTF_METHOD SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution );
	virtual void TJS_INTF_METHOD RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color );

};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTJSNI_BasicDrawDevice
//---------------------------------------------------------------------------
class tTJSNI_BasicDrawDevice :
	public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

	tTVPBasicDrawDevice * Device;

public:
	tTJSNI_BasicDrawDevice();
	~tTJSNI_BasicDrawDevice();
	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param,
			iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

public:
	tTVPBasicDrawDevice * GetDevice() const { return Device; }

};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTJSNC_BasicDrawDevice
//---------------------------------------------------------------------------
class tTJSNC_BasicDrawDevice : public tTJSNativeClass
{
public:
	tTJSNC_BasicDrawDevice();

	static tjs_uint32 ClassID;

private:
	iTJSNativeInstance *CreateNativeInstance();
};
//---------------------------------------------------------------------------


#endif // BASIC_DRAW_DEVICE_H