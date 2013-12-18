#include "ncbind/ncbind.hpp"
#include "QR_Encode.h" // http://www.psytec.co.jp/freesoft/02/

#ifdef _DEBUG
#define dm(msg) TVPAddLog(msg)
#else
#define dm(msg) {}
#endif

class LayerQRCode
{
public:
	LayerQRCode(){}

	static tjs_error TJS_INTF_METHOD drawQRCode(
		tTJSVariant	*result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis)
	{
		tTJSVariant vals[4];
		tTJSVariant *pvals[4] = { vals, vals +1, vals +2, vals +3 };
		tTJSVariant	val;

		//	QRコードにするデータ
		if(numparams < 1)
			return TJS_E_BADPARAMCOUNT;
		ttstr	str	= param[0]->AsStringNoAddRef();
		size_t	len	= str.length() + 1, cvlen;
		char*	buf	= new char[len];
		wcstombs_s(&cvlen, buf, len, str.c_str(), _TRUNCATE);
		dm(L"original text= "+str);
		dm(L"convert text = "+ttstr(buf));
		dm(L"original len = "+ttstr((tjs_int)len)+" / convert len = "+ttstr((tjs_int)cvlen));
		buf[len-1]	= 0x0;

		//	その他仕様
		tjs_int	ecLevel		= numparams > 1 && param[1]->Type() != tvtVoid ? param[1]->AsInteger() : QR_LEVEL_L;
		tjs_int	qrVersion	= numparams > 2 && param[2]->Type() != tvtVoid ? param[2]->AsInteger() : QR_VRESION_S;
		tjs_int	autoExtent	= numparams > 3 && param[3]->Type() != tvtVoid ? param[3]->AsInteger() : true;
		tjs_int	maskPattern	= numparams > 4 && param[4]->Type() != tvtVoid ? param[4]->AsInteger() : -1;
		dm(L"error correct level = "+ttstr(ecLevel)+" / version = "+ttstr(qrVersion)+" / auto extent = "+ttstr(autoExtent)+" / masking patter = "+ttstr(maskPattern));

		//	QRエンコード
		CQR_Encode	*pQR_Encode	= new CQR_Encode;
		BOOL	res = pQR_Encode->EncodeData(ecLevel, qrVersion, autoExtent, maskPattern, buf, len-1);
		dm(L"Version = "+ttstr((tjs_int)pQR_Encode->m_nVersion)+" / Masking Patter = "+ttstr((tjs_int)pQR_Encode->m_nMaskingNo));
		if(!res)
		{
			if(result)
				*result	= L"データが存在しないか、容量をオーバーしています";
			return TJS_S_OK;
		}

		//	サイズを調整
		tjs_int	symbolsize	= pQR_Encode->m_nSymbleSize;
		tjs_int	whsize		= symbolsize + QR_MARGIN * 2;
		vals[0]	= whsize;
		vals[1] = whsize;
		objthis->FuncCall(0, L"setImageSize", NULL, &val, 2, pvals, objthis);

		//	レイヤー情報取得
/*		objthis->PropGet(0, L"imageWidth", NULL, &val, objthis);
		tjs_int	imageWidth	= (tjs_int)val;
		objthis->PropGet(0, L"imageHeight", NULL, &val, objthis);
		tjs_int	imageHeight	= (tjs_int)val;
*/		objthis->PropGet(0, L"mainImageBufferPitch", NULL, &val, objthis);
		tjs_int	bufferpitch	= (tjs_int)val;
		objthis->PropGet(0, L"mainImageBufferForWrite", NULL, &val, objthis);
		tjs_uint8*	bufferptr	= (tjs_uint8*)(tjs_int)val;

		//	書き込み
		tjs_uint32*	lineptr		= (tjs_uint32*)bufferptr;
		tjs_int		linepitch	= bufferpitch * sizeof(tjs_uint8) / sizeof(tjs_uint32);
		for(tjs_int	y=0; y<QR_MARGIN; y++)
		{
			tjs_uint32	*bptr	= lineptr;
			for(tjs_int x=0; x<whsize; x++)
				*bptr++	= 0xffffffff;
			lineptr	+= linepitch;
		}
		for(tjs_int y=0; y<symbolsize; y++)
		{
			tjs_uint32	*bptr	= lineptr;
			for(tjs_int x=0; x<QR_MARGIN; x++)
				*bptr++	= 0xffffffff;
			for(tjs_int x=0; x<symbolsize; x++)
				*bptr++	= 0xffffffff - pQR_Encode->m_byModuleData[x][y] * 0xffffff;
			for(tjs_int x=0; x<QR_MARGIN; x++)
				*bptr++	= 0xffffffff;
			lineptr	+= linepitch;
		}
		for(tjs_int	y=0; y<QR_MARGIN; y++)
		{
			tjs_uint32	*bptr	= lineptr;
			for(tjs_int x=0; x<whsize; x++)
				*bptr++	= 0xffffffff;
			lineptr	+= linepitch;
		}

/*		//	領域を更新させる
		vals[0] = (tjs_int64)0;
		vals[1] = (tjs_int64)0;
		vals[2] = (tjs_int64)imageWidth;
		vals[3] = (tjs_int64)imageHeight;
		static tjs_uint32 update_hint = 0;
		objthis->FuncCall(0, L"update", &update_hint, NULL, 4, pvals, objthis);
*/
		delete pQR_Encode;
		delete [] buf;

		return TJS_S_OK;
	}
};

NCB_ATTACH_CLASS(LayerQRCode, Layer)
{
	RawCallback("drawQRCode", &Class::drawQRCode, 0);
};
