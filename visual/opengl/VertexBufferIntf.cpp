
#include "tjsCommHead.h"

#include "VertexBufferIntf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "DebugIntf.h"
#include "tjsArray.h"
#include <memory>


//---------------------------------------------------------------------------
tTJSNI_VertexBuffer::tTJSNI_VertexBuffer() {
}
//---------------------------------------------------------------------------
tTJSNI_VertexBuffer::~tTJSNI_VertexBuffer() {
}
//---------------------------------------------------------------------------
template<typename TType>
static void TVPCreateToVertexBuffer( GLVertexBufferObject& vtxBuff, const tTJSVariant *param, GLenum updateType, bool isIndex ) {
	tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		tjs_int count = TJSGetArrayElementCount( clo.Object );
		if( count == 0 ) TVPThrowExceptionMessage( TJS_W( "Array is zero." ) );
		tjs_int size = (tjs_int)(sizeof(TType)*count);
		std::unique_ptr<TType[]> buffer(new TType[count]);
		tTJSVariant tmp;
		for( tjs_int i = 0; i < count; i++ ) {
			if( TJS_FAILED( clo.Object->PropGetByNum( TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis ) ) )
				TVPThrowExceptionMessage( TJS_W( "Insufficient number of arrays." ) );
			buffer[i] = (TType)(tjs_int)tmp;
		}
		vtxBuff.createVertexBuffer( size, updateType, isIndex, (const void*)buffer.get() );
	}
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_VertexBuffer::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams < 3 ) return TJS_E_BADPARAMCOUNT;
	if( param[0]->Type() == tvtObject ) {
		DataType = (GLenum)(tjs_int)*param[1];
		tjs_int updateType = *param[2];
		bool isIndex = false;
		if( numparams >= 4 ) isIndex = (tjs_int)*param[3] ? true : false;
		switch( DataType ) {
		case GL_BYTE:
			TVPCreateToVertexBuffer<GLbyte>( VertexBufferObject, param[0], updateType, isIndex );
			break;
		case GL_UNSIGNED_BYTE:
			TVPCreateToVertexBuffer<GLubyte>( VertexBufferObject, param[0], updateType, isIndex );
			break;
		case GL_SHORT:
			TVPCreateToVertexBuffer<GLshort>( VertexBufferObject, param[0], updateType, isIndex );
			break;
		case GL_UNSIGNED_SHORT:
			TVPCreateToVertexBuffer<GLushort>( VertexBufferObject, param[0], updateType, isIndex );
			break;
		case GL_FIXED:
			TVPCreateToVertexBuffer<GLint>( VertexBufferObject, param[0], updateType, isIndex );
			break;
		case GL_FLOAT: {
			tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
			if( clo.Object ) {
				tjs_int count = TJSGetArrayElementCount( clo.Object );
				if( count == 0 ) TVPThrowExceptionMessage( TJS_W( "Array is zero." ) );
				tjs_int size = (tjs_int)(sizeof(GLfloat)*count);
				std::unique_ptr<GLfloat[]> buffer(new GLfloat[count]);
				tTJSVariant tmp;
				for( tjs_int i = 0; i < count; i++ ) {
					if( TJS_FAILED( clo.Object->PropGetByNum( TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis ) ) )
						TVPThrowExceptionMessage( TJS_W( "Insufficient number of arrays." ) );
					buffer[i] = (GLfloat)(tjs_real)tmp;
				}
				VertexBufferObject.createVertexBuffer( size, updateType, isIndex, (const void*)buffer.get() );
			}
			break;
		}
		default:
			TVPThrowExceptionMessage( TJS_W( "Unknown data type." ) );
			break;
		}
	} else {
		tjs_int size = *param[0];
		DataType = (GLenum)(tjs_int)*param[1];
		tjs_int updateType = *param[2];
		bool isIndex = false;
		if( numparams >= 4 ) isIndex = (tjs_int)*param[3] ? true : false;
		VertexBufferObject.createVertexBuffer( size, (GLenum)updateType, isIndex );
	}
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_VertexBuffer::Invalidate() {
	VertexBufferObject.destory();
}
//---------------------------------------------------------------------------
template<typename TType>
static void TVPCopyToVertexBuffer( GLVertexBufferObject& vtxBuff, const tTJSVariant *param, tjs_int offset, tjs_int count ) {
	if( (sizeof(TType)*offset + sizeof(TType)*count) > (tjs_size)vtxBuff.size() ) {
		TVPThrowExceptionMessage( TJSRangeError );
	}
	std::unique_ptr<TType[]> buffer(new TType[count]);
	tTJSVariant tmp;
	tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
	for( tjs_int i = 0; i < count; i++ ) {
		if( TJS_FAILED( clo.Object->PropGetByNum( TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis ) ) )
			TVPThrowExceptionMessage( TJS_W( "Insufficient number of arrays." ) );
		buffer[i] = (TType)(tjs_int)tmp;
	}
	vtxBuff.copyBuffer( sizeof(TType)*offset, sizeof(TType)*count, (const void*)buffer.get() );
}
//---------------------------------------------------------------------------
void tTJSNI_VertexBuffer::SetVertex( const tTJSVariant *param, tjs_int offset ) {
	if( VertexBufferObject.usage() == GL_STATIC_DRAW ) {
		TVPThrowExceptionMessage( TJS_W( "This vertex buffer is constant." ) );
	}
	tTJSVariantClosure clo = param->AsObjectClosureNoAddRef();
	if( clo.Object ) {
		tjs_int count = TJSGetArrayElementCount( clo.Object );
		if( count > 0 ) {
			switch( DataType ) {
			case GL_BYTE:
				TVPCopyToVertexBuffer<GLbyte>( VertexBufferObject, param, offset, count );
				return;
			case GL_UNSIGNED_BYTE:
				TVPCopyToVertexBuffer<GLubyte>( VertexBufferObject, param, offset, count );
				return;
			case GL_SHORT:
				TVPCopyToVertexBuffer<GLshort>( VertexBufferObject, param, offset, count );
				return;
			case GL_UNSIGNED_SHORT:
				TVPCopyToVertexBuffer<GLushort>( VertexBufferObject, param, offset, count );
				return;
			case GL_FIXED:
				TVPCopyToVertexBuffer<GLint>( VertexBufferObject, param, offset, count );
				return;
			case GL_FLOAT: {
				if( (sizeof(GLfloat)*offset + sizeof(GLfloat)*count) > (tjs_size)VertexBufferObject.size() ) {
					TVPThrowExceptionMessage( TJSRangeError );
				}
				std::unique_ptr<GLfloat[]> buffer(new GLfloat[count]);
				tTJSVariant tmp;
				for( tjs_int i = 0; i < count; i++ ) {
					if( TJS_FAILED( clo.Object->PropGetByNum( TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis ) ) )
						TVPThrowExceptionMessage( TJS_W( "Insufficient number of arrays." ) );
					buffer[i] = (GLfloat)(tjs_real)tmp;
				}
				VertexBufferObject.copyBuffer( sizeof(GLfloat)*offset, sizeof(GLfloat)*count, buffer.get() );
				return;
			}
			}
		}
	}
	TJS_eTJSError( TJSSpecifyArray );
}
//---------------------------------------------------------------------------
void tTJSNI_VertexBuffer::BindFrameBuffer() {
	VertexBufferObject.bindBuffer();
}
//---------------------------------------------------------------------------
// tTJSNC_VertexBuffer : TJS VertexBuffer class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_VertexBuffer::ClassID = -1;
tTJSNC_VertexBuffer::tTJSNC_VertexBuffer() : tTJSNativeClass(TJS_W("VertexBuffer"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(VertexBuffer) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_VertexBuffer, /*TJS class name*/VertexBuffer)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/VertexBuffer)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setVertex)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer);
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	tjs_int offset = 0;
	if( numparams >= 2 ) offset = *param[1];
	_this->SetVertex( param[0], offset );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setVertex )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/lock )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer );
	if( result ) {
		*result = (tjs_int64)_this->Lock();
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/lock )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/unlock )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer );
	_this->Unlock();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/unlock )
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(size)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer);
		*result = _this->GetBufferSize();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(size)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dataType)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer);
		*result = _this->GetDataType();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(dataType)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(updateType)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer);
		*result = _this->GetUpdateType();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(updateType)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(isIndex)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer);
		*result = _this->IsIndex() ? (tjs_int)1 : (tjs_int)0;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(isIndex)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(nativeHandle)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_VertexBuffer);
		*result = _this->GetNativeHandle();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(nativeHandle)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtByte) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_BYTE;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtByte)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtUByte) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_UNSIGNED_BYTE;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtUByte)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtShort) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_SHORT;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtShort)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtUShort) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_UNSIGNED_SHORT;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtUShort)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtInt) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_INT;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtInt)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtUInt) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_UNSIGNED_INT;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtUInt)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtFixed) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int) GL_FIXED;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtFixed)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dtFloat) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_FLOAT;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(dtFloat)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(utStream) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_STREAM_DRAW;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(utStream)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(utStatic) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_STATIC_DRAW;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(utStatic)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(utDynamic) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_DYNAMIC_DRAW;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(utDynamic)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(ptPoints) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_POINTS;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(ptPoints)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(ptLineStrip) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_LINE_STRIP;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(ptLineStrip)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(ptLineLoop) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_LINE_LOOP;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(ptLineLoop)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(ptLines) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_LINES;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(ptLines)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(ptTriangleStrip) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_TRIANGLE_STRIP;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(ptTriangleStrip)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(ptTriangleFan) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_TRIANGLE_FAN;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(ptTriangleFan)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(ptTriangles) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)GL_TRIANGLES;
		return TJS_S_OK;
	} TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
} TJS_END_NATIVE_STATIC_PROP_DECL(ptTriangles)
//----------------------------------------------------------------------

//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_VertexBuffer()
{
	tTJSNativeClass *cls = new tTJSNC_VertexBuffer();
	return cls;
}
//---------------------------------------------------------------------------
