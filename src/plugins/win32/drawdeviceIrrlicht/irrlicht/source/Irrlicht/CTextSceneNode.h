// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_TEXT_SCENE_NODE_H_INCLUDED__
#define __C_TEXT_SCENE_NODE_H_INCLUDED__

#include "ITextSceneNode.h"
#include "IGUIFont.h"
#include "IGUIFontBitmap.h"
#include "ISceneCollisionManager.h"
#include "SMesh.h"

namespace irr
{
namespace scene
{


	class CTextSceneNode : public ITextSceneNode
	{
	public:

		//! constructor
		CTextSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,
			gui::IGUIFont* font, scene::ISceneCollisionManager* coll,
			const core::vector3df& position = core::vector3df(0,0,0), const wchar_t* text=0,
			video::SColor color=video::SColor(100,0,0,0));

		//! destructor
		virtual ~CTextSceneNode();

		virtual void OnRegisterSceneNode();

		//! renders the node.
		virtual void render();

		//! returns the axis aligned bounding box of this node
		virtual const core::aabbox3d<f32>& getBoundingBox() const;

		//! sets the text string
		virtual void setText(const wchar_t* text);

		//! sets the color of the text
		virtual void setTextColor(video::SColor color);
		
		//! Returns type of the scene node
		virtual ESCENE_NODE_TYPE getType() const { return ESNT_TEXT; }

	private:

		core::stringw Text;
		video::SColor Color;
		gui::IGUIFont* Font;
		scene::ISceneCollisionManager* Coll;
		core::aabbox3d<f32> Box;
	};

	class CBillboardTextSceneNode : public ITextSceneNode
	{
	public:

		CBillboardTextSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,	
			gui::IGUIFont* font,const wchar_t* text,
			const core::vector3df& position, const core::dimension2d<f32>& size,
			video::SColor shade_top, video::SColor shade_bottom);

		//! destructor
		virtual ~CBillboardTextSceneNode();

		virtual void OnRegisterSceneNode();

		//! renders the node.
		virtual void render();

		//! returns the axis aligned bounding box of this node
		virtual const core::aabbox3d<f32>& getBoundingBox() const;

		//! sets the text string
		virtual void setText(const wchar_t* text);

		//! sets the color of the text
		virtual void setTextColor(video::SColor color);
		
		//! sets the size of the billboard
		virtual void setSize(const core::dimension2d<f32>& size);

		//! gets the size of the billboard
		virtual const core::dimension2d<f32>& getSize();

		virtual video::SMaterial& getMaterial(u32 i);
		
		//! returns amount of materials used by this scene node.
		virtual u32 getMaterialCount() const;

		//! Returns type of the scene node
		virtual ESCENE_NODE_TYPE getType() const { return ESNT_TEXT; }

	private:

		core::stringw Text;
		video::SColor Color;
		gui::IGUIFontBitmap* Font;

		core::dimension2d<f32> Size;
		core::aabbox3d<f32> BBox;
		video::SMaterial Material;

		video::SColor Shade_top;
		video::SColor Shade_bottom;
		struct SSymbolInfo
		{
			u32 bufNo;
			f32 Width;
			f32 Kerning;
			u32 firstInd;
			u32 firstVert;
		};

		core::array < SSymbolInfo > Symbol;

		SMesh *Mesh;
	};

} // end namespace scene
} // end namespace irr

#endif

