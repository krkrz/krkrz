// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//
// This file was originally written by ZDimitor.

//----------------------------------------------------------------------
//  my3d-stuff.h -  part of the My3D Tools
//
//  This tool was created by Zhuck Dmitry (ZDimitor).
//  Everyone can use it as wants ( i'll be happy if it helps to someone :) ).
//----------------------------------------------------------------------

#ifndef __C_MY_3D_STUFF_H_INCLUDED__
#define __C_MY_3D_STUFF_H_INCLUDED__

#include <irrTypes.h>

namespace irr
{
namespace scene
{

//**********************************************************************
//                      MY3D stuff
//**********************************************************************

const unsigned long MY_ID = 0x4d593344; // was: #define MY_ID 'MY3D'
#define MY_VER             0x0003

#define MY_SCENE_HEADER_ID 0x1000

#define MY_MAT_LIST_ID       0x2000
#define MY_MAT_HEADER_ID     0x2100
#define MY_TEX_FNAME_ID      0x2101
#define MY_TEXDATA_ID        0x2500
#define MY_TEXDATA_HEADER_ID 0x2501
#define MY_TEXDATA_RLE_HEADER_ID 0x2502

#define MY_MESH_LIST_ID    0x3000
#define MY_MESH_HEADER_ID  0x3100
#define MY_VERTS_ID        0x3101
#define MY_FACES_ID        0x3102
#define MY_TVERTS_ID       0x3103
#define MY_TFACES_ID       0x3104

#define MY_FILE_END_ID     0xFFFF

const unsigned long MY_TEXDATA_COMPR_NONE_ID = 0x4e4f4e45; // was: MY_TEXDATA_COMPR_NONE_ID   'NONE'
const unsigned long MY_TEXDATA_COMPR_SIMPLE_ID = 0x53494d50; // was: #define MY_TEXDATA_COMPR_SIMPLE_ID 'SIMP'
const unsigned long MY_TEXDATA_COMPR_RLE_ID = 0x20524c45; // was: #define MY_TEXDATA_COMPR_RLE_ID ' RLE'

const unsigned long MY_PIXEL_FORMAT_24 = 0x5f32345f; // was: #define MY_PIXEL_FORMAT_24 '_24_'
const unsigned long MY_PIXEL_FORMAT_16 = 0x5f31365f; // was: #define MY_PIXEL_FORMAT_16 '_16_'
//--------------------------------------------------------------------
// byte-align structures
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#   pragma pack( push, packing )
#   pragma pack( 1 )
#   define PACK_STRUCT
#elif defined( __GNUC__ )
#   define PACK_STRUCT  __attribute__((packed))
#else
#   error compiler not supported
#endif
//----------------------------------------------------------------------
struct SMyColor
{   SMyColor () {;}
    SMyColor (s32 __R, s32 __G, s32 __B, s32 __A)
        : R(__R), G(__G), B(__B), A(__A) {}
    s32 R, G, B, A;
} PACK_STRUCT;

struct SMyVector3
{   SMyVector3 () {;}
    SMyVector3 (f32 __X, f32 __Y, f32 __Z)
        : X(__X), Y(__Y), Z(__Z) {}
    f32 X, Y, Z;
} PACK_STRUCT;

struct SMyVector2
{   SMyVector2 () {;}
    SMyVector2(f32 __X, f32 __Y)
        : X(__X), Y(__Y) {}
    f32 X, Y;
} PACK_STRUCT;

struct SMyVertex
{   SMyVertex () {;}
    SMyVertex (SMyVector3 _Coord, SMyColor _Color, SMyVector3 _Normal)
        :Coord(_Coord), Color(_Color), Normal(_Normal) {;}
    SMyVector3 Coord;
    SMyColor   Color;
    SMyVector3 Normal;
} PACK_STRUCT;

struct SMyTVertex
{   SMyTVertex () {;}
    SMyTVertex (SMyVector2 _TCoord)
        : TCoord(_TCoord) {;}
    SMyVector2 TCoord;
} PACK_STRUCT;

struct SMyFace
{   SMyFace() {;}
    SMyFace(u32 __A, u32 __B, u32 __C)
        : A(__A), B(__B), C(__C) {}
    u32 A, B, C;
} PACK_STRUCT;

// file header (6 bytes)
struct SMyFileHeader
{   u32 MyId; // MY3D
    u16 Ver;  // Version
} PACK_STRUCT;

// scene header
struct SMySceneHeader
{   SMyColor BackgrColor;  // background color
    SMyColor AmbientColor; // ambient color
    s32 MaterialCount;     // material count
    s32 MeshCount;         // mesh count
} PACK_STRUCT;

// material header
struct SMyMaterialHeader
{   c8  Name[256];           // material name
    u32 Index;
    SMyColor AmbientColor;
    SMyColor DiffuseColor;
    SMyColor EmissiveColor;
    SMyColor SpecularColor;
    f32 Shininess;
    f32 Transparency;
    s32 TextureCount;        // texture count
} PACK_STRUCT;

// mesh header
struct SMyMeshHeader
{   c8  Name[256];   // material name
    u32 MatIndex;    // index of the mesh material
    u32 TChannelCnt; // mesh mapping channels count
} PACK_STRUCT;

// texture data header
struct SMyTexDataHeader
{   c8  Name[256]; // texture name
    u32 ComprMode; //compression mode
    u32 PixelFormat;
    u32 Width;   // image width
    u32 Height;  // image height
} PACK_STRUCT;

// pixel color 24bit (R8G8B8)
struct SMyPixelColor24
{   SMyPixelColor24() {;}
    SMyPixelColor24(u8 __r, u8 __g, u8 __b)
        : r(__r), g(__g), b(__b) {}
    u8 r, g, b;
} PACK_STRUCT;

// pixel color 16bit (A1R5G5B5)
struct SMyPixelColor16
{   SMyPixelColor16() {;}
    SMyPixelColor16(s16 _argb): argb(_argb) {;}
    SMyPixelColor16(u8 r, u8 g, u8 b)
    {   argb = ((r&0x1F)<<10) | ((g&0x1F)<<5) | (b&0x1F);
    }
    s16 argb;
} PACK_STRUCT;

// RLE Header
struct SMyRLEHeader
{   SMyRLEHeader() {}
    u32 nEncodedBytes;
    u32 nDecodedBytes;
} PACK_STRUCT;

// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#   pragma pack( pop, packing )
#endif

} // end namespace
} // end namespace

#endif

