
#include "tjsCommHead.h"

#include "BitmapIntf.h"
#include "GraphicsLoadThread.h"
#include "ThreadIntf.h"
#include "NativeEventQueue.h"
#include "UserEvent.h"
#include "EventIntf.h"
#include "StorageIntf.h"
#include "LayerBitmapIntf.h"
#include "MsgIntf.h"
#include "UtilStreams.h"

tTVPTmpBitmapImage::tTVPTmpBitmapImage()
	: w(0), h(0), imglen(0), img(NULL), buf(NULL), MetaInfo(NULL)
{}
tTVPTmpBitmapImage::~tTVPTmpBitmapImage() {
	if(img) {
		delete[] img;
		img = NULL;
	}
	if(buf) {
		delete[] buf;
		buf = NULL;
	}
	if( MetaInfo ) {
		delete MetaInfo;
		MetaInfo = NULL;
	}
}
tTVPImageLoadCommand::tTVPImageLoadCommand() : owner_(NULL), bmp_(NULL), dest_(NULL) {}
tTVPImageLoadCommand::~tTVPImageLoadCommand() {
	if( owner_ ) {
		owner_->Release();
		owner_ = NULL;
	}
	if( dest_ ) {
		delete dest_;
		dest_ = NULL;
	}
	bmp_ = NULL;
}

static void TVPLoadGraphicAsync_SizeCallback(void *callbackdata, tjs_uint w, tjs_uint h)
{
	tTVPTmpBitmapImage* img = (tTVPTmpBitmapImage*)callbackdata;
	img->h = h;
	img->w = w;
	img->imglen = h * w;
	img->buf = new tjs_uint32[img->imglen];
	img->img = new tjs_uint32*[h];
	tjs_uint32* pos = img->buf;
	for( tjs_uint i = 0; i < h; i++ ) {
		img->img[i] = pos;
		pos += w;
	}
}
//---------------------------------------------------------------------------
static void* TVPLoadGraphicAsync_ScanLineCallback(void *callbackdata, tjs_int y)
{
	tTVPTmpBitmapImage* img = (tTVPTmpBitmapImage*)callbackdata;
	if( y >= 0 ) {
		if( y < (tjs_int)img->h ) {
			return img->img[y];
		} else {
			return NULL;
		}
	}
	return NULL; // -1 の時のフラッシュ処理は何もしない
}
//---------------------------------------------------------------------------
static void TVPLoadGraphicAsync_MetaInfoPushCallback(void *callbackdata, const ttstr & name, const ttstr & value)
{
	tTVPTmpBitmapImage * img = (tTVPTmpBitmapImage *)callbackdata;

	if(!img->MetaInfo) img->MetaInfo = new std::vector<tTVPGraphicMetaInfoPair>();
	img->MetaInfo->push_back(tTVPGraphicMetaInfoPair(name, value));
}
//---------------------------------------------------------------------------

tTVPAsyncImageLoader::tTVPAsyncImageLoader()
: EventQueue(this,&tTVPAsyncImageLoader::Proc), tTVPThread(true)
{
	EventQueue.Allocate();
}
tTVPAsyncImageLoader::~tTVPAsyncImageLoader() {
	ExitRequest();
	WaitFor();
	EventQueue.Deallocate();
	while( CommandQueue.size() > 0 ) {
		tTVPImageLoadCommand* cmd = CommandQueue.front();
		CommandQueue.pop();
		delete cmd;
	}
	while( LoadedQueue.size() > 0 ) {
		tTVPImageLoadCommand* cmd = LoadedQueue.front();
		LoadedQueue.pop();
		delete cmd;
	}
}
void tTVPAsyncImageLoader::ExitRequest() {
	Terminate();
	PushCommandQueueEvent.Set();
}
void tTVPAsyncImageLoader::Execute() {
	// プライオリティは最低にする
	SetPriority(ttpIdle);
	LoadingThread();
}
void tTVPAsyncImageLoader::SendToLoadFinish() {
	NativeEvent ev(TVP_EV_IMAGE_LOAD_THREAD);
	EventQueue.PostEvent(ev);
}
void tTVPAsyncImageLoader::Proc( NativeEvent& ev )
{
	if(ev.Message != TVP_EV_IMAGE_LOAD_THREAD) {
		EventQueue.HandlerDefault(ev);
		return;
	}
	HandleLoadedImage();
}
void tTVPAsyncImageLoader::HandleLoadedImage() {
	bool loading;
	do {
		loading = false;
		tTVPImageLoadCommand* cmd = NULL;
		{
			tTJSCriticalSectionHolder cs(ImageQueueCS);
			if( LoadedQueue.size() > 0 ) {
				cmd = LoadedQueue.front();
				LoadedQueue.pop();
				loading = true;
			}
		}
		if( cmd != NULL ) {
			cmd->bmp_->SetLoading( false );
			if( cmd->result_.length() > 0 ) {
				// error
				tTJSVariant param[4];
				param[0] = tTJSVariant((iTJSDispatch2*)NULL,(iTJSDispatch2*)NULL);
				param[1] = 1; // true async
				param[2] = 1; // true error
				param[3] = cmd->result_; // error_mes
				static ttstr eventname(TJS_W("onLoaded"));
				if( cmd->owner_->IsValid(0,NULL,NULL,cmd->owner_) == TJS_S_TRUE ) {
					TVPPostEvent(cmd->owner_, cmd->owner_, eventname, 0, TVP_EPT_IMMEDIATE, 4, param);
				}

				if( cmd->dest_->MetaInfo ) {
					delete cmd->dest_->MetaInfo;
					cmd->dest_->MetaInfo = NULL;
				}
			} else {
				iTJSDispatch2* metainfo = TVPMetaInfoPairsToDictionary(cmd->dest_->MetaInfo);

				tjs_uint w = cmd->dest_->w;
				tjs_uint h = cmd->dest_->h;
				cmd->bmp_->SetSize(w,h,false);
				tjs_int stride = cmd->bmp_->GetPixelBufferPitch();
				tjs_uint8* dest = reinterpret_cast<tjs_uint8*>( cmd->bmp_->GetPixelBufferForWrite() );
				tjs_uint pitch = w*4;
				for( tjs_uint i = 0; i < h; i++ ) {
					memcpy( dest, cmd->dest_->img[i], pitch );
					dest += stride;
				}

				TVPPushGraphicCache( cmd->path_, cmd->bmp_->GetBitmap(), cmd->dest_->MetaInfo );
				cmd->dest_->MetaInfo = NULL;

				tTJSVariant param[4];
				param[0] = tTJSVariant(metainfo,metainfo);
				metainfo->Release();
				param[1] = 1; // true async
				param[2] = 0; // false error
				param[3] = TJS_W(""); // error_mes
				static ttstr eventname(TJS_W("onLoaded"));
				if( cmd->owner_->IsValid(0,NULL,NULL,cmd->owner_) == TJS_S_TRUE ) {
					TVPPostEvent(cmd->owner_, cmd->owner_, eventname, 0, TVP_EPT_IMMEDIATE, 4, param);
				}
			}
			delete cmd;
		}
	} while(loading);
}
//---------------------------------------------------------------------------

// onLoaded( dic, is_async, is_error, error_mes ); エラーは
// sync ( main thead )
void tTVPAsyncImageLoader::LoadRequest( iTJSDispatch2 *owner, tTJSNI_Bitmap* bmp, const ttstr &name ) {
	tTVPBaseBitmap* dest = new tTVPBaseBitmap(32,32,32);
	iTJSDispatch2* metainfo = NULL;
	ttstr nname = TVPNormalizeStorageName(name);
	if( TVPCheckImageCache(nname,dest,glmNormal,0,0,-1,&metainfo) ) {
		// キャッシュ内に発見、即座に読込みを完了する
		bmp->CopyFrom( dest );
		if( dest ) delete dest;
		bmp->SetLoading( false );

		tTJSVariant param[4];
		param[0] = tTJSVariant(metainfo,metainfo);
		metainfo->Release();
		param[1] = 0; // false
		param[2] = 0; // false
		param[3] = TJS_W(""); // error_mes
		static ttstr eventname(TJS_W("onLoaded"));
		TVPPostEvent(owner, owner, eventname, 0, TVP_EPT_IMMEDIATE, 4, param);
		return;
	}
	if( dest ) delete dest;
	dest = NULL;
	if( TVPIsExistentStorage(name) == false ) {
		TVPThrowExceptionMessage(TVPCannotFindStorage, name);
	}
	ttstr ext = TVPExtractStorageExt(name);
	if(ext == TJS_W("")) {
		TVPThrowExceptionMessage(TJS_W("Filename extension not found/%1"), name);
	}

	PushLoadQueue( owner, bmp, nname );
}

// tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);
//	tTJSBinaryStream* stream = TVPCreateStream(nname, TJS_BS_READ);
// TVPCreateStream はロックされているので、非同期で実行可能
void tTVPAsyncImageLoader::PushLoadQueue( iTJSDispatch2 *owner, tTJSNI_Bitmap *bmp, const ttstr &nname ) {
	tTVPImageLoadCommand* cmd = new tTVPImageLoadCommand();
	cmd->owner_ = owner;
	owner->AddRef();
	cmd->bmp_ = bmp;
	cmd->path_ = nname;
	cmd->dest_ = new tTVPTmpBitmapImage();
	cmd->result_.Clear();
	{
		// キューをロックしてプッシュ
		tTJSCriticalSectionHolder cs(CommandQueueCS);
		CommandQueue.push(cmd);
	}
	// 追加したことをイベントで通知
	PushCommandQueueEvent.Set();
}
void tTVPAsyncImageLoader::LoadingThread() {
	while( !GetTerminated() ) {
		// キュー追加イベント待ち
		PushCommandQueueEvent.WaitFor(0);
		if( GetTerminated() ) break;
		bool loading;
		do {
			loading = false;
			tTVPImageLoadCommand* cmd = NULL;

			{ // Lock
				tTJSCriticalSectionHolder cs(CommandQueueCS);
				if( CommandQueue.size() ) {
					cmd = CommandQueue.front();
					CommandQueue.pop();
				}
			}
			if( cmd ) {
				loading = true;
				LoadImageFromCommand(cmd);
				{	// Lock
					tTJSCriticalSectionHolder cs(ImageQueueCS);
					LoadedQueue.push(cmd);
				}
				// Send to message
				SendToLoadFinish();
			}
		} while( loading && !GetTerminated() );
	}
}
void tTVPAsyncImageLoader::LoadImageFromCommand( tTVPImageLoadCommand* cmd ) {
	ttstr ext = TVPExtractStorageExt(cmd->path_);
	tTVPGraphicHandlerType* handler = NULL;
	if(ext == TJS_W("")) {
		cmd->result_ = TJS_W("Filename extension not found");
	} else {
		handler = TVPGetGraphicLoadHandler(ext);
	}
	if( handler ) {
		try {
			tTVPStreamHolder holder(cmd->path_);
			(handler->Handler)(handler->FormatData, (void*)cmd->dest_, TVPLoadGraphicAsync_SizeCallback,
				TVPLoadGraphicAsync_ScanLineCallback, TVPLoadGraphicAsync_MetaInfoPushCallback,
				holder.Get(), -1, glmNormal );
		} catch(...) {
			// 例外は全てキャッチ
			cmd->result_ = TVPFormatMessage(TVPImageLoadError, cmd->path_);
		}
	} else {
		// error
		cmd->result_ = TVPFormatMessage(TVPUnknownGraphicFormat, cmd->path_);
	}
}

