// Copyright (C) 2006-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_Q3_LEVEL_SHADER_H_INCLUDED__
#define __I_Q3_LEVEL_SHADER_H_INCLUDED__

#include "irrArray.h"
#include "fast_atof.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "coreutil.h"

namespace irr
{
namespace scene
{
namespace quake3
{

	static const core::stringc irrEmptyStringc("");

	//! Hold the different Mesh Types used for getMesh
	enum eQ3MeshIndex
	{
		E_Q3_MESH_GEOMETRY = 0,
		E_Q3_MESH_ITEMS,
		E_Q3_MESH_BILLBOARD,
		E_Q3_MESH_SIZE
	};

	// we are not using gamma, so quake3 is very dark.
	// define the standard multiplication for lightmaps and vertex colors
	const video::E_MATERIAL_TYPE defaultLightMap = video::EMT_LIGHTMAP_M2;
	const video::E_MODULATE_FUNC defaultModulate = video::EMFN_MODULATE_2X;

	// some useful typedefs
	typedef core::array< core::stringc > tStringList;
	typedef core::array< video::ITexture* > tTexArray;

	// name = "a b c .."
	struct SVariable
	{
		core::stringc name;
		core::stringc content;

		void clear ()
		{
			name = "";
			content = "";
		}

		s32 isValid () const
		{
			return name.size();
		}

		bool operator == ( const SVariable &other ) const
		{
			return name == other.name;
		}
	};

	// string helper.. TODO: move to generic files
	inline s32 isEqual ( const core::stringc &string, u32 &pos, const c8 *list[], u32 listSize )
	{
		const char * in = string.c_str () + pos;

		for ( u32 i = 0; i != listSize; ++i )
		{
			if (string.size() < pos)
				return -2;
			u32 len = (u32) strlen ( list[i] );
			if (string.size() < pos+len)
				continue;
			if ( in [len] != 0 && in [len] != ' ' )
				continue;
			if ( strncmp ( in, list[i], len ) )
				continue;

			pos += len + 1;
			return (s32) i;
		}
		return -2;
	}

	inline f32 getAsFloat ( const core::stringc &string, u32 &pos )
	{
		const char * in = string.c_str () + pos;

		f32 value = 0.f;
		pos += (u32) ( core::fast_atof_move ( in, value ) - in ) + 1;
		return value;
	}

	inline core::vector3df getAsVector3df ( const core::stringc &string, u32 &pos )
	{
		core::vector3df v;

		v.X = getAsFloat ( string, pos );
		v.Z = getAsFloat ( string, pos );
		v.Y = getAsFloat ( string, pos );

		return v;
	}

	/*
		extract substrings
	*/
	inline void getAsStringList ( tStringList &list, s32 max, const core::stringc &string, u32 &startPos )
	{
		list.clear ();

		s32 finish = 0;
		s32 endPos;
		do
		{
			endPos = string.findNext ( ' ', startPos );
			if ( endPos == -1 )
			{
				finish = 1;
				endPos = string.size();
			}

			list.push_back ( string.subString ( startPos, endPos - startPos ) );
			startPos = endPos + 1;

			if ( list.size() >= (u32) max )
				finish = 1;

		} while ( !finish );

	}

	//! A blend function for a q3 shader.
	struct SBlendFunc
	{
		SBlendFunc () : type ( video::EMT_SOLID ), param ( 0.f ) {}

		video::E_MATERIAL_TYPE type;
		f32 param;
	};

	// parses the content of Variable cull
	inline bool getBackfaceCulling ( const core::stringc &string )
	{
		if ( string.size() == 0 )
			return true;

		bool ret = true;
		static const c8 * funclist[] = { "none", "disable" };

		u32 pos = 0;
		switch ( isEqual ( string, pos, funclist, 2 ) )
		{
			case 0:
			case 1:
				ret = false;
				break;
		}
		return ret;
	}

	// parses the content of Variable depthfunc
	// return a z-test
	inline u32 getDepthFunction ( const core::stringc &string )
	{
		if ( string.size() == 0 )
			return 1;

		u32 ret = 1;
		static const c8 * funclist[] = { "lequal","equal" };

		u32 pos = 0;
		switch ( isEqual ( string, pos, funclist, 2 ) )
		{
			case 0:
				ret = 1;
			case 1:
				ret = 2;
				break;
		}
		return ret;
	}



	// parses the content of Variable blendfunc,alphafunc
	inline static void getBlendFunc ( const core::stringc &string, SBlendFunc &blendfunc )
	{
		if ( string.size() == 0 )
			return;

		// maps to E_BLEND_FACTOR
		static const c8 * funclist[] =
		{
			"gl_zero",
			"gl_one",
			"gl_dst_color",
			"gl_one_minus_dst_color",
			"gl_src_color",
			"gl_one_minus_src_color",
			"gl_src_alpha",
			"gl_one_minus_src_alpha",
			"gl_dst_alpha",
			"gl_one_minus_dst_alpha",
			"gl_src_alpha_sat",

			"add",
			"filter",
			"blend",

			"ge128",
			"gt0"
		};


		u32 pos = 0;
		s32 srcFact = isEqual ( string, pos, funclist, 16 );

		if ( srcFact < 0 )
			return;

		u32 resolved = 0;
		s32 dstFact = isEqual ( string, pos, funclist, 16 );

		switch ( srcFact )
		{
			case video::EBF_ONE:
				switch ( dstFact )
				{
					// gl_one gl_zero
					case video::EBF_ZERO:
						blendfunc.type = video::EMT_SOLID;
						resolved = 1;
						break;

					// gl_one gl_one
					case video::EBF_ONE:
						blendfunc.type = video::EMT_TRANSPARENT_ADD_COLOR;
						resolved = 1;
						break;
				} break;

			case video::EBF_SRC_ALPHA:
				switch ( dstFact )
				{
					// gl_src_alpha gl_one_minus_src_alpha
					case video::EBF_ONE_MINUS_SRC_ALPHA:
						blendfunc.type = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
						blendfunc.param = 1.f / 255.f;
						resolved = 1;
						break;
				} break;

			case 11:
				// add
				blendfunc.type = video::EMT_TRANSPARENT_ADD_COLOR;
				resolved = 1;
				break;
			case 12:
				// filter = gl_dst_color gl_zero
				blendfunc.type = video::EMT_ONETEXTURE_BLEND;
				blendfunc.param = video::pack_texureBlendFunc ( video::EBF_DST_COLOR, video::EBF_ZERO, defaultModulate );
				resolved = 1;
				break;
			case 13:
				// blend
				blendfunc.type = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
				blendfunc.param = 1.f / 255.f;
				resolved = 1;
				break;
			case 14:
				// alphafunc ge128
				blendfunc.type = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
				blendfunc.param = 0.5f;
				resolved = 1;
				break;
			case 15:
				// alphafunc gt0
				blendfunc.type = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
				blendfunc.param = 1.f / 255.f;
				resolved = 1;
				break;
		}

		// use the generic blender
		if ( 0 == resolved )
		{
			blendfunc.type = video::EMT_ONETEXTURE_BLEND;
			blendfunc.param = video::pack_texureBlendFunc (
					(video::E_BLEND_FACTOR) srcFact,
					(video::E_BLEND_FACTOR) dstFact,
					defaultModulate);
		}
	}

	struct SModifierFunction
	{
		SModifierFunction ()
			: masterfunc0 ( 0 ), masterfunc1(0), func ( 0 ),
			tcgen( 8 ), base ( 0 ), amp ( 1 ), phase ( 0 ), freq ( 1 ), wave(1) {}

		// "tcmod","deformvertexes","rgbgen", "tcgen"
		s32 masterfunc0;
		// depends
		s32 masterfunc1;
		// depends
		s32 func;

		s32 tcgen;

		union
		{
			f32 base;
			f32 bulgewidth;
		};

		union
		{
			f32 amp;
			f32 bulgeheight;
		};

		f32 phase;

		union
		{
			f32 freq;
			f32 bulgespeed;
		};

		f32 wave;

		f32 evaluate ( f32 dt ) const
		{
			// phase in 0 and 1..
			f32 x = core::fract( (dt + phase ) * freq );
			f32 y = 0.f;

			switch ( func )
			{
				// sin
				case 0:
					y = (f32) sin ( x * core::PI64 * 2.0 );
					break;
				// cos
				case 1:
					y = (f32) cos ( x * core::PI64 * 2.0 );
					break;
				// square
				case 2:
					y = x < 0.5f ? 1.f : -1.f;
					break;
				// triangle
				case 3:
					y = x < 0.5f ? ( 2.f * x ) - 1.f : ( -2.f * x ) + 2.f;
					break;
				// sawtooth:
				case 4:
					y = x;
					break;
				// inverse sawtooth:
				case 5:
					y = 1.f - x;
					break;
			}

			return base + ( y * amp );
		}


	};

	//
	inline void getModifierFunc ( SModifierFunction& fill, const core::stringc &string, u32 &pos )
	{
		if ( string.size() == 0 )
			return;

		static const c8 * funclist[] =
		{
			"sin","cos","square", "triangle", "sawtooth","inversesawtooth"
		};

		fill.func = quake3::isEqual ( string,pos, funclist,6 );
		if ( fill.func == -2 )
			fill.func = 0;

		fill.base = quake3::getAsFloat ( string, pos );
		fill.amp = quake3::getAsFloat ( string, pos );
		fill.phase = quake3::getAsFloat ( string, pos );
		fill.freq = quake3::getAsFloat ( string, pos );
	}



	struct SVarGroup
	{
		// simple assoziative array
		s32 getIndex( const c8 * name ) const
		{
			SVariable search;
			search.name = name;

			return Variable.linear_search ( search );
		}

		// searches for Variable name and returns is content
		// if Variable is not found a reference to an Empty String is returned
		const core::stringc &get( const c8 * name ) const
		{
			s32 index = getIndex ( name );
			if ( index < 0 )
				return irrEmptyStringc;

			return Variable [ index ].content;
		}

		bool isDefined ( const c8 * name, const c8 * content = 0 ) const
		{
			for ( u32 i = 0; i != Variable.size (); ++i )
			{
				if ( 0 == strcmp ( Variable[i].name.c_str(), name ) )
				{
					if ( 0 == content )
						return true;
					if ( 0 == strcmp ( Variable[i].content.c_str(), content ) )
						return true;
				}
			}
			return false;
		}

		core::array < SVariable > Variable;
	};

	struct SVarGroupList: public IReferenceCounted
	{
		SVarGroupList () {}
		virtual ~SVarGroupList () {}

		core::array < SVarGroup > VariableGroup;
	};


	//! A Parsed Shader Holding Variables ordered in Groups
	class SShader
	{
		public:
			bool operator == (const SShader &other ) const
			{
				return name == other.name;
			}

			bool operator < (const SShader &other ) const
			{
				return name < other.name;
			}

			const SVarGroup * getGroup ( u32 stage ) const
			{
				if ( 0 == VarGroup || stage >= VarGroup->VariableGroup.size () )
					return 0;

				return &VarGroup->VariableGroup [ stage ];
			}

			// id
			s32 id;

			// Shader: shader name ( also first variable in first Vargroup )
			// Entity: classname ( variable in Group(1) )
			core::stringc name;
			SVarGroupList *VarGroup; // reference
	};

	typedef SShader SEntity;

	typedef core::array < SEntity > tQ3EntityList;

	/*
		dump shader like original layout, regardless of internal data holding
		no recursive folding..
	*/
	inline void dumpVarGroup ( core::stringc &dest, const SVarGroup * group, s32 stack )
	{
		core::stringc buf;
		s32 i;


		if ( stack > 0 )
		{
			buf = "";
			for ( i = 0; i < stack - 1; ++i )
				buf += '\t';

			buf += "{\n";
			dest.append ( buf );
		}

		for ( u32 g = 0; g != group->Variable.size(); ++g )
		{
			buf = "";
			for ( i = 0; i < stack; ++i )
				buf += '\t';

			buf += group->Variable[g].name;
			buf += " ";
			buf += group->Variable[g].content;
			buf += "\n";
			dest.append ( buf );
		}

		if ( stack > 1 )
		{
			buf = "";
			for ( i = 0; i < stack - 1; ++i )
				buf += '\t';

			buf += "}\n";
			dest.append ( buf );
		}

	}

	inline core::stringc & dumpShader ( core::stringc &dest, const SShader * shader )
	{
		dest = "";
		if ( 0 == shader )
			return dest;

		const SVarGroup * group;

		const u32 size = shader->VarGroup->VariableGroup.size ();

		for ( u32 i = 0; i != size; ++i )
		{
			group = &shader->VarGroup->VariableGroup[ i ];
			dumpVarGroup ( dest, group, core::clamp ( (s32) i, 0, 2 ) );
		}

		if ( size <= 1 )
		{
			dest.append ( "{\n" );
		}

		dest.append ( "}\n" );
		return dest;
	}



	/*
		quake3 doesn't care much about tga & jpg
		load one or multiple files stored in name started at startPos to the texture array textures
		if texture is not loaded 0 will be added ( to find missing textures easier)
	*/
	inline void getTextures(tTexArray &textures,
				const core::stringc &name, u32 &startPos,
				io::IFileSystem *fileSystem,
				video::IVideoDriver* driver)
	{
		static const char * extension[2] =
		{
			".jpg",
			".tga"
		};

		tStringList stringList;
		getAsStringList ( stringList, -1, name, startPos );

		textures.clear();

		core::stringc loadFile;
		for ( u32 i = 0; i!= stringList.size (); ++i )
		{
			video::ITexture* texture = 0;
			for ( u32 g = 0; g != 2 ; ++g )
			{
				core::cutFilenameExtension ( loadFile, stringList[i] ).append ( extension[g] );

				if ( fileSystem->existFile ( loadFile.c_str() ) )
				{
					texture = driver->getTexture( loadFile.c_str () );
					if ( texture )
					{
						break;
					}
				}
			}
			// take 0 Texture
			textures.push_back(texture);
		}
	}


	//! Manages various Quake3 Shader Styles
	class IShaderManager : public IReferenceCounted
	{
	};

} // end namespace quake3
} // end namespace scene
} // end namespace irr

#endif

