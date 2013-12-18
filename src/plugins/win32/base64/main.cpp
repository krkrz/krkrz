#include "ncbind.hpp"

class Base64
{
	//	inbuf の内容を base64 エンコードして、outbuf に文字列として出力
	//	outbuf のサイズは、insize / 4 * 3 + 1 必要
	static void encodeBase64(tjs_uint8* inbuf, tjs_int insize, char* outbuf)
	{
		char	*base64str	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		tjs_int	insize_3	= insize - 3;
		tjs_int outptr		= 0;
		tjs_int i;
		for(i=0; i<=insize_3; i+=3)
		{
			outbuf[outptr++] = base64str[ (inbuf[i  ] >> 2) & 0x3F ];
			outbuf[outptr++] = base64str[((inbuf[i  ] << 4) & 0x30) | ((inbuf[i+1] >> 4) & 0x0F)];
			outbuf[outptr++] = base64str[((inbuf[i+1] << 2) & 0x3C) | ((inbuf[i+2] >> 6) & 0x03)];
			outbuf[outptr++] = base64str[ (inbuf[i+2]     ) & 0x3F ];
		}
		switch(insize % 3)
		{
		case 2:
			outbuf[outptr+4] = 0;
			outbuf[outptr+3] = '=';
			outbuf[outptr+2] = base64str[ (inbuf[i+1] << 2) & 0x3C ];
			outbuf[outptr+1] = base64str[((inbuf[i  ] << 4) & 0x30) | ((inbuf[i+1] >> 4) & 0x0F)];
			outbuf[outptr  ] = base64str[ (inbuf[i  ] >> 2) & 0x3F ];
			break;
		case 1:
			outbuf[outptr+4] = 0;
			outbuf[outptr+3] = '=';
			outbuf[outptr+2] = '=';
			outbuf[outptr+1] = base64str[ (inbuf[i  ] << 4) & 0x30 ];
			outbuf[outptr  ] = base64str[ (inbuf[i  ] >> 2) & 0x3F ];
			break;
		case 0:
			outbuf[outptr  ] = 0;
		}
	}

public:
	//	指定のファイルをbase64エンコードする
	static tjs_error TJS_INTF_METHOD encode(
		tTJSVariant	*result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis)
	{
		if(numparams < 1)
			return TJS_E_BADPARAMCOUNT;

		if(!result)
			return TJS_S_OK;

//		TVPAddLog(L"Base64.encode("+ttstr(param[0]->AsStringNoAddRef())+")");

		ttstr fn = TVPGetPlacedPath(*param[0]);
		if(fn.length() > 0)
		{//	アーカイブ内
			IStream *in = TVPCreateIStream(fn, TJS_BS_READ);
			if(in)
			{
				tjs_uint8	buffer[1024*16/4*3];
				char		cbuf[1024*16+4];
				DWORD		size;
				ttstr		encoded		= "";
				while(in->Read(buffer, sizeof buffer, &size) == S_OK && size > 0)
				{
					encodeBase64(buffer, size, cbuf);
					encoded	+= cbuf;
				}
				in->Release();
				*result	= encoded;
			}
//			else
//				TVPThrowExceptionMessage((ttstr(L"can't open file: "+fn)).c_str());
		}
		return TJS_S_OK;
	}

	//	指定の内容をbase64でコードして、指定のファイルへ出力する
	//	ついでにmd5を計算する
	static tjs_error TJS_INTF_METHOD decode(
		tTJSVariant	*result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis)
	{
		if(numparams < 2)
			return TJS_E_BADPARAMCOUNT;

//		TVPAddLog(L"Base64.decode("+ttstr(param[0]->AsStringNoAddRef())+", "+ttstr(param[1]->AsStringNoAddRef())+")");

		tjs_char*	data= (tjs_char*)(ttstr(param[0]->AsStringNoAddRef())).c_str();
		tjs_int		len	= param[0]->AsStringNoAddRef()->GetLength();
		ttstr		fn	= param[1]->AsStringNoAddRef();
		tjs_int		base64tonum[] = {
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  62,   0,   0,   0,  63, 
			  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,   0,   0,   0,   0,   0,   0, 
			   0,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, 
			  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,   0,   0,   0,   0,   0, 
			   0,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, 
			  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
		};
		if(fn.length() > 0)
		{
			IStream * out = TVPCreateIStream(fn, TJS_BS_WRITE);
			if(out)
			{
				tjs_int			dptr = 0;
				tjs_uint8		buf[1024*16*3/4];
				tjs_int			bsize= sizeof(buf);
				tjs_int			len_4	= len - 4;
				ULONG			wsize;
				tjs_int			bptr;
				TVP_md5_state_t	md5state;
				if(result)
					TVP_md5_init(&md5state);
				do
				{
					bptr	= 0;
					while(bptr < bsize && dptr < len_4)
					{
						buf[bptr]	=  base64tonum[data[dptr++]] << 2;
						buf[bptr++]	|= base64tonum[data[dptr]] >> 4;
						buf[bptr]	=  base64tonum[data[dptr++]] << 4;
						buf[bptr++]	|= base64tonum[data[dptr]] >> 2;
						buf[bptr]	=  base64tonum[data[dptr++]] << 6;
						buf[bptr++]	|= base64tonum[data[dptr]];
						dptr++;
/*						TVPAddLog(L"data["+ttstr(dptr-4)+"]: "+
							ttstr((tjs_char)data[dptr-4])+", "+ttstr((tjs_int)data[dptr-3])+", "+
							ttstr((tjs_char)data[dptr-2])+", "+ttstr((tjs_int)data[dptr-1])+" = "+
							ttstr(buf[bptr-3])+", "+ttstr(buf[bptr-2])+", "+ttstr(buf[bptr-1])
						);
*/					}
					out->Write(buf, bptr, &wsize);
					if(result)
						TVP_md5_append(&md5state, buf, bptr);
				}
				while(dptr < len_4);
				bptr	= 0;
				buf[bptr]	=  base64tonum[data[dptr++]] << 2;
				buf[bptr++]	|= base64tonum[data[dptr]] >> 4;
				buf[bptr]	=  base64tonum[data[dptr++]] << 4;
				if(data[dptr] != (tjs_char)'=')
				{
					buf[bptr++]	|= base64tonum[data[dptr]] >> 2;
					buf[bptr]	=  base64tonum[data[dptr++]] << 6;
					if(data[dptr] != (tjs_char)'=')
						buf[bptr++]	|= base64tonum[data[dptr]];
				}
				out->Write(buf, bptr, &wsize);
				out->Release();
				if(result)
				{
					TVP_md5_append(&md5state, buf, bptr);

					tjs_uint8	digest[16];
					TVP_md5_finish(&md5state, digest);
					char*	n2h	= "0123456789abcdef";
					char		digest_base64[32+1];
					for(tjs_int i=0; i<16; i++)
					{
						digest_base64[(i<<1)  ]	= n2h[digest[i] >> 4];
						digest_base64[(i<<1)+1]	= n2h[digest[i] & 0x0f];
					}
					digest_base64[32]	= 0;
					*result	= digest_base64;
//					*result	= (char*)digest;
				}
			}
			else
				TVPThrowExceptionMessage((ttstr(L"can't open writefile: "+fn)).c_str());
		}
		else
			TVPThrowExceptionMessage(L"no filename");

		return TJS_S_OK;
	}
};

NCB_REGISTER_CLASS(Base64)
{
	RawCallback("encode", &Class::encode, 0);
	RawCallback("decode", &Class::decode, 0);
};
