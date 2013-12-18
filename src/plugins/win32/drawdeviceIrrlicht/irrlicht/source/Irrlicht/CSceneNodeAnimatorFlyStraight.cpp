// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSceneNodeAnimatorFlyStraight.h"

namespace irr
{
namespace scene
{


//! constructor
CSceneNodeAnimatorFlyStraight::CSceneNodeAnimatorFlyStraight(const core::vector3df& startPoint,
				const core::vector3df& endPoint, u32 timeForWay,
				bool loop, u32 now)
: Start(startPoint), End(endPoint), WayLength(0.0f), TimeFactor(0.0f), StartTime(now), TimeForWay(timeForWay), Loop(loop)
{
	#ifdef _DEBUG
	setDebugName("CSceneNodeAnimatorFlyStraight");
	#endif

	recalculateImidiateValues();
}


void CSceneNodeAnimatorFlyStraight::recalculateImidiateValues()
{
	Vector = End - Start;
	WayLength = (f32)Vector.getLength();
	Vector.normalize();

	TimeFactor = WayLength / TimeForWay;
}



//! destructor
CSceneNodeAnimatorFlyStraight::~CSceneNodeAnimatorFlyStraight()
{
}



//! animates a scene node
void CSceneNodeAnimatorFlyStraight::animateNode(ISceneNode* node, u32 timeMs)
{
	if (!node)
		return;

	u32 t = (timeMs-StartTime);

	core::vector3df pos = Start;

	if (!Loop && t >= TimeForWay)
		pos = End;
	else
		pos += Vector * (f32)fmod((f32)t, (f32)TimeForWay) * TimeFactor;
	node->setPosition(pos);
}


//! Writes attributes of the scene node animator.
void CSceneNodeAnimatorFlyStraight::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	out->addVector3d("Start", Start);
	out->addVector3d("End", End);
	out->addInt("TimeForWay", TimeForWay);
	out->addBool("Loop", Loop);
}


//! Reads attributes of the scene node animator.
void CSceneNodeAnimatorFlyStraight::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	Start = in->getAttributeAsVector3d("Start");
	End = in->getAttributeAsVector3d("End");
	TimeForWay = in->getAttributeAsInt("TimeForWay");
	Loop = in->getAttributeAsBool("Loop");

	recalculateImidiateValues();
}


} // end namespace scene
} // end namespace irr

