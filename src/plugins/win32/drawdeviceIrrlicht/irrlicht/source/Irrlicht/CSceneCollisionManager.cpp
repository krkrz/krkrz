// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSceneCollisionManager.h"
#include "ISceneNode.h"
#include "ICameraSceneNode.h"
#include "ITriangleSelector.h"
#include "SViewFrustum.h"

#include "os.h"
#include "irrMath.h"

namespace irr
{
namespace scene
{

//! constructor
CSceneCollisionManager::CSceneCollisionManager(ISceneManager* smanager, video::IVideoDriver* driver)
: SceneManager(smanager), Driver(driver)
{
	#ifdef _DEBUG
	setDebugName("CSceneCollisionManager");
	#endif

	if (Driver)
		Driver->grab();
}



//! destructor
CSceneCollisionManager::~CSceneCollisionManager()
{
	if (Driver)
		Driver->drop();
}



//! Returns the scene node, which is currently visible under the overgiven
//! screencoordinates, viewed from the currently active camera.
ISceneNode* CSceneCollisionManager::getSceneNodeFromScreenCoordinatesBB(
	core::position2d<s32> pos, s32 idBitMask, bool bNoDebugObjects)
{
	core::line3d<f32> ln = getRayFromScreenCoordinates(pos, 0);

	if ( ln.start == ln.end )
		return 0;

	return getSceneNodeFromRayBB(ln, idBitMask, bNoDebugObjects);
}



//! Returns the nearest scene node which collides with a 3d ray and
//! which id matches a bitmask.
ISceneNode* CSceneCollisionManager::getSceneNodeFromRayBB(core::line3d<f32> ray,
						s32 idBitMask,
						bool bNoDebugObjects)
{
	ISceneNode* best = 0;
	f32 dist = 9999999999.0f;

	getPickedNodeBB(SceneManager->getRootSceneNode(), ray, 
		idBitMask, bNoDebugObjects, dist, best);

	return best;
}


//! recursive method for going through all scene nodes
void CSceneCollisionManager::getPickedNodeBB(ISceneNode* root,
               const core::line3df& ray,
               s32 bits,
               bool bNoDebugObjects,
               f32& outbestdistance,
               ISceneNode*& outbestnode)
{
   core::vector3df edges[8];

   const core::list<ISceneNode*>& children = root->getChildren();

   core::list<ISceneNode*>::ConstIterator it = children.begin();
   for (; it != children.end(); ++it)
   {
      ISceneNode* current = *it;

      if (current->isVisible() &&
          (bNoDebugObjects ? !current->isDebugObject() : true) &&
          (bits==0 || (bits != 0 && (current->getID() & bits))))
      {
         // get world to object space transform
         core::matrix4 mat;
         if (!current->getAbsoluteTransformation().getInverse(mat))
            continue;

         // transform vector from world space to object space
         core::line3df line(ray);
         mat.transformVect(line.start);
         mat.transformVect(line.end);

         const core::aabbox3df& box = current->getBoundingBox();

         // do intersection test in object space
         if (box.intersectsWithLine(line))
         {
            box.getEdges(edges);
            f32 distance = 0.0f;

            for (s32 e=0; e<8; ++e)
            {
               f32 t = edges[e].getDistanceFromSQ(line.start);
               if (t > distance)
                  distance = t;
            }

            if (distance < outbestdistance)
            {
               outbestnode = current;
               outbestdistance = distance;
            }
         }
      }

      getPickedNodeBB(current, ray, bits, bNoDebugObjects, outbestdistance, outbestnode);
   }
} 



//! Returns the scene node, at which the overgiven camera is looking at and
//! which id matches the bitmask.
ISceneNode* CSceneCollisionManager::getSceneNodeFromCameraBB(
	ICameraSceneNode* camera, s32 idBitMask, bool bNoDebugObjects)
{
	if (!camera)
		return 0;

	core::vector3df start = camera->getAbsolutePosition();
	core::vector3df end = camera->getTarget();

	end = start + ((end - start).normalize() * camera->getFarValue());
	core::line3d<f32> line(start, end);

	return getSceneNodeFromRayBB(line, idBitMask, bNoDebugObjects);
}



//! Finds the collision point of a line and lots of triangles, if there is one.
bool CSceneCollisionManager::getCollisionPoint(const core::line3d<f32>& ray,
	ITriangleSelector* selector, core::vector3df& outIntersection,
	core::triangle3df& outTriangle)
{
	if (!selector)
	{
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return false;
	}

	s32 totalcnt = selector->getTriangleCount();
	Triangles.set_used(totalcnt);

	s32 cnt = 0;
	selector->getTriangles(Triangles.pointer(), totalcnt, cnt, ray);

	const core::vector3df linevect = ray.getVector().normalize();
	core::vector3df intersection;
	f32 nearest = 9999999999999.0f;
	bool found = false;
	const f32 raylength = ray.getLengthSQ();

	for (s32 i=0; i<cnt; ++i)
	{
		if (Triangles[i].getIntersectionWithLine(ray.start, linevect, intersection))
		{
			const f32 tmp = intersection.getDistanceFromSQ(ray.start);
			const f32 tmp2 = intersection.getDistanceFromSQ(ray.end);

			if (tmp < raylength && tmp2 < raylength && tmp < nearest)
			{
				nearest = tmp;
				outTriangle = Triangles[i];
				outIntersection = intersection;
				found = true;
			}
		}
	}

	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return found;
}



//! Collides a moving ellipsoid with a 3d world with gravity and returns
//! the resulting new position of the ellipsoid.
core::vector3df CSceneCollisionManager::getCollisionResultPosition(
	ITriangleSelector* selector,
	const core::vector3df &position, const core::vector3df& radius,
	const core::vector3df& direction,
	core::triangle3df& triout,
	bool& outFalling,
	f32 slidingSpeed,
	const core::vector3df& gravity)
{
	if (!selector || radius.X == 0.0f || radius.Y == 0.0f || radius.Z == 0.0f)
		return position;

	return collideEllipsoidWithWorld(selector, position,
		radius, direction, slidingSpeed, gravity, triout, outFalling);
}


void CSceneCollisionManager::testTriangleIntersection(SCollisionData* colData,
			const core::triangle3df& triangle)
{
	const core::plane3d<f32> trianglePlane = triangle.getPlane();

	// only check front facing polygons
	if ( !trianglePlane.isFrontFacing(colData->normalizedVelocity) )
		return;

	// get interval of plane intersection

	f32 t1, t0;
	bool embeddedInPlane = false;

	// calculate signed distance from sphere position to triangle plane
	f32 signedDistToTrianglePlane = trianglePlane.getDistanceTo(
		colData->basePoint);

	f32 normalDotVelocity =
		trianglePlane.Normal.dotProduct(colData->velocity);

	if ( core::iszero ( normalDotVelocity ) )
	{
		// sphere is traveling parallel to plane

		if (fabs(signedDistToTrianglePlane) >= 1.0f)
			return; // no collision possible
		else
		{
			// sphere is embedded in plane
			embeddedInPlane = true;
			t0 = 0.0;
			t1 = 1.0;
		}
	}
	else
	{
		normalDotVelocity = core::reciprocal ( normalDotVelocity );

		// N.D is not 0. Calculate intersection interval
		t0 = (-1.f - signedDistToTrianglePlane) * normalDotVelocity;
		t1 = (1.f - signedDistToTrianglePlane) * normalDotVelocity;

		// Swap so t0 < t1
		if (t0 > t1) { f32 tmp = t1; t1 = t0; t0 = tmp;	}

		// check if at least one value is within the range
		if (t0 > 1.0f || t1 < 0.0f)
			return; // both t values are outside 1 and 0, no collision possible

		// clamp to 0 and 1
		t0 = core::clamp ( t0, 0.f, 1.f );
		t1 = core::clamp ( t1, 0.f, 1.f );
	}

	// at this point we have t0 and t1, if there is any intersection, it
	// is between this interval
	core::vector3df collisionPoint;
	bool foundCollision = false;
	f32 t = 1.0f;

	// first check the easy case: Collision within the triangle;
	// if this happens, it must be at t0 and this is when the sphere
	// rests on the front side of the triangle plane. This can only happen
	// if the sphere is not embedded in the triangle plane.

	if (!embeddedInPlane)
	{
		core::vector3df planeIntersectionPoint =
			(colData->basePoint - trianglePlane.Normal)
			+ (colData->velocity * t0);

		if (triangle.isPointInsideFast(planeIntersectionPoint))
		{
			foundCollision = true;
			t = t0;
			collisionPoint = planeIntersectionPoint;
		}
	}

	// if we havent found a collision already we will have to sweep
	// the sphere against points and edges of the triangle. Note: A
	// collision inside the triangle will always happen before a
	// vertex or edge collision.

	if (!foundCollision)
	{
		core::vector3df velocity = colData->velocity;
		core::vector3df base = colData->basePoint;

		f32 velocitySqaredLength = velocity.getLengthSQ();
		f32 a,b,c;
		f32 newT;

		// for each edge or vertex a quadratic equation has to be solved:
		// a*t^2 + b*t + c = 0. We calculate a,b, and c for each test.

		// check against points
		a = velocitySqaredLength;

		// p1
		b = 2.0f * (velocity.dotProduct(base - triangle.pointA));
		c = (triangle.pointA-base).getLengthSQ() - 1.f;
		if (getLowestRoot(a,b,c,t, &newT))
		{
			t = newT;
			foundCollision = true;
			collisionPoint = triangle.pointA;
		}

		// p2
		if (!foundCollision)
		{
			b = 2.0f * (velocity.dotProduct(base - triangle.pointB));
			c = (triangle.pointB-base).getLengthSQ() - 1.f;
			if (getLowestRoot(a,b,c,t, &newT))
			{
				t = newT;
				foundCollision = true;
				collisionPoint = triangle.pointB;
			}
		}

		// p3
		if (!foundCollision)
		{
			b = 2.0f * (velocity.dotProduct(base - triangle.pointC));
			c = (triangle.pointC-base).getLengthSQ() - 1.f;
			if (getLowestRoot(a,b,c,t, &newT))
			{
				t = newT;
				foundCollision = true;
				collisionPoint = triangle.pointC;
			}
		}

		// check against edges:

		// p1 --- p2
		core::vector3df edge = triangle.pointB - triangle.pointA;
		core::vector3df baseToVertex = triangle.pointA - base;
		f32 edgeSqaredLength = edge.getLengthSQ();
		f32 edgeDotVelocity = edge.dotProduct(velocity);
		f32 edgeDotBaseToVertex = edge.dotProduct(baseToVertex);

		// calculate parameters for equation
		a = edgeSqaredLength* -velocitySqaredLength +
			edgeDotVelocity*edgeDotVelocity;
		b = edgeSqaredLength* (2.f *velocity.dotProduct(baseToVertex)) -
			2.0f*edgeDotVelocity*edgeDotBaseToVertex;
		c = edgeSqaredLength* (1.f -baseToVertex.getLengthSQ()) +
			edgeDotBaseToVertex*edgeDotBaseToVertex;

		// does the swept sphere collide against infinite edge?
		if (getLowestRoot(a,b,c,t,&newT))
		{
			f32 f = (edgeDotVelocity*newT - edgeDotBaseToVertex) / edgeSqaredLength;
			if (f >=0.0f && f <= 1.0f)
			{
				// intersection took place within segment
				t = newT;
				foundCollision = true;
				collisionPoint = triangle.pointA + (edge*f);
			}
		}

		// p2 --- p3
		edge = triangle.pointC-triangle.pointB;
		baseToVertex = triangle.pointB - base;
		edgeSqaredLength = edge.getLengthSQ();
		edgeDotVelocity = edge.dotProduct(velocity);
		edgeDotBaseToVertex = edge.dotProduct(baseToVertex);

		// calculate parameters for equation
		a = edgeSqaredLength* -velocitySqaredLength +
			edgeDotVelocity*edgeDotVelocity;
		b = edgeSqaredLength* (2*velocity.dotProduct(baseToVertex)) -
			2.0f*edgeDotVelocity*edgeDotBaseToVertex;
		c = edgeSqaredLength* (1-baseToVertex.getLengthSQ()) +
			edgeDotBaseToVertex*edgeDotBaseToVertex;

		// does the swept sphere collide against infinite edge?
		if (getLowestRoot(a,b,c,t,&newT))
		{
			f32 f = (edgeDotVelocity*newT-edgeDotBaseToVertex) /
				edgeSqaredLength;
			if (f >=0.0f && f <= 1.0f)
			{
				// intersection took place within segment
				t = newT;
				foundCollision = true;
				collisionPoint = triangle.pointB + (edge*f);
			}
		}


		// p3 --- p1
		edge = triangle.pointA-triangle.pointC;
		baseToVertex = triangle.pointC - base;
		edgeSqaredLength = edge.getLengthSQ();
		edgeDotVelocity = edge.dotProduct(velocity);
		edgeDotBaseToVertex = edge.dotProduct(baseToVertex);

		// calculate parameters for equation
		a = edgeSqaredLength* -velocitySqaredLength +
			edgeDotVelocity*edgeDotVelocity;
		b = edgeSqaredLength* (2*velocity.dotProduct(baseToVertex)) -
			2.0f*edgeDotVelocity*edgeDotBaseToVertex;
		c = edgeSqaredLength* (1-baseToVertex.getLengthSQ()) +
			edgeDotBaseToVertex*edgeDotBaseToVertex;

		// does the swept sphere collide against infinite edge?
		if (getLowestRoot(a,b,c,t,&newT))
		{
			f32 f = (edgeDotVelocity*newT-edgeDotBaseToVertex) /
				edgeSqaredLength;
			if (f >=0.0f && f <= 1.0f)
			{
				// intersection took place within segment
				t = newT;
				foundCollision = true;
				collisionPoint = triangle.pointC + (edge*f);
			}
		}
	}// end no collision found

	// set result:
	if (foundCollision)
	{
		// distance to collision is t
		f32 distToCollision = t*colData->velocity.getLength();

		// does this triangle qualify for closest hit?
		if (!colData->foundCollision ||
			distToCollision	< colData->nearestDistance)
		{
			colData->nearestDistance = distToCollision;
			colData->intersectionPoint = collisionPoint;
			colData->foundCollision = true;
			colData->intersectionTriangle = triangle;
			++colData->triangleHits;
		}

	}// end found collision
}



//! Collides a moving ellipsoid with a 3d world with gravity and returns
//! the resulting new position of the ellipsoid.
core::vector3df CSceneCollisionManager::collideEllipsoidWithWorld(
	ITriangleSelector* selector, const core::vector3df &position,
	const core::vector3df& radius,  const core::vector3df& velocity,
	f32 slidingSpeed,
	const core::vector3df& gravity,
	core::triangle3df& triout, bool& outFalling)
{
	if (!selector || radius.X == 0.0f || radius.Y == 0.0f || radius.Z == 0.0f)
		return position;

	// This code is based on the paper "Improved Collision detection and Response"
	// by Kasper Fauerby, but some parts are modified.

	SCollisionData colData;
	colData.R3Position = position;
	colData.R3Velocity = velocity;
	colData.eRadius = radius;
	colData.nearestDistance = 9999999999999.0f;
	colData.selector = selector;
	colData.slidingSpeed = slidingSpeed;
	colData.triangleHits = 0;

	core::vector3df eSpacePosition = colData.R3Position / colData.eRadius;
	core::vector3df eSpaceVelocity = colData.R3Velocity / colData.eRadius;

	// iterate until we have our final position

	core::vector3df finalPos = collideWithWorld(
		0, colData, eSpacePosition, eSpaceVelocity);

	outFalling = false;

	// add gravity

	if (gravity != core::vector3df(0,0,0))
	{
		colData.R3Position = finalPos * colData.eRadius;
		colData.R3Velocity = gravity;
		colData.triangleHits = 0;

		eSpaceVelocity = gravity/colData.eRadius;

		finalPos = collideWithWorld(0, colData,
			finalPos, eSpaceVelocity);

		outFalling = (colData.triangleHits == 0);
	}

	if (colData.triangleHits)
	{
		triout = colData.intersectionTriangle;
		triout.pointA *= colData.eRadius;
		triout.pointB *= colData.eRadius;
		triout.pointC *= colData.eRadius;
	}

	finalPos *= colData.eRadius;
	return finalPos;
}

core::vector3df CSceneCollisionManager::collideWithWorld(s32 recursionDepth,
	SCollisionData &colData, core::vector3df pos, core::vector3df vel)
{
	f32 veryCloseDistance = colData.slidingSpeed;

	if (recursionDepth > 5)
		return pos;

	colData.velocity = vel;
	colData.normalizedVelocity = vel;
	colData.normalizedVelocity.normalize();
	colData.basePoint = pos;
	colData.foundCollision = false;
	colData.nearestDistance = 9999999999999.0f;

	//------------------ collide with world

	// get all triangles with which we might collide
	core::aabbox3d<f32> box(colData.R3Position);
	box.addInternalPoint(colData.R3Position + colData.R3Velocity);
	box.MinEdge -= colData.eRadius;
	box.MaxEdge += colData.eRadius;

	s32 totalTriangleCnt = colData.selector->getTriangleCount();
	Triangles.set_used(totalTriangleCnt);

	core::matrix4 scaleMatrix;
	scaleMatrix.setScale(
		core::vector3df(1.0f / colData.eRadius.X, 
						1.0f / colData.eRadius.Y,
						1.0f / colData.eRadius.Z)
					);

	s32 triangleCnt = 0;
	colData.selector->getTriangles(Triangles.pointer(), totalTriangleCnt, triangleCnt, box, &scaleMatrix);
	//colData.selector->getTriangles(Triangles.pointer(), totalTriangleCnt, triangleCnt, &scaleMatrix);

	for (s32 i=0; i<triangleCnt; ++i)
		testTriangleIntersection(&colData, Triangles[i]);

	//---------------- end collide with world

	if (!colData.foundCollision)
		return pos + vel;

	// original destination point
	core::vector3df destinationPoint = pos + vel;
	core::vector3df newBasePoint = pos;

	// only update if we are not already very close
	// and if so only move very close to intersection, not to the
	// exact point
	if (colData.nearestDistance >= veryCloseDistance)
	{
		core::vector3df v = vel;
		v.setLength( colData.nearestDistance - veryCloseDistance );
		newBasePoint = colData.basePoint + v;

		v.normalize();
		colData.intersectionPoint -= (v * veryCloseDistance);
	}

	// calculate sliding plane

	core::vector3df slidePlaneOrigin = colData.intersectionPoint;
	core::vector3df slidePlaneNormal = newBasePoint - colData.intersectionPoint;
	slidePlaneNormal.normalize();
	core::plane3d<f32> slidingPlane(slidePlaneOrigin, slidePlaneNormal);

	core::vector3df newDestinationPoint =
		destinationPoint -
		(slidePlaneNormal * slidingPlane.getDistanceTo(destinationPoint));

	// generate slide vector

	core::vector3df newVelocityVector = newDestinationPoint -
		colData.intersectionPoint;

	if (newVelocityVector.getLength() < veryCloseDistance)
		return newBasePoint;

	return collideWithWorld(recursionDepth+1, colData,
		newBasePoint, newVelocityVector);
}


//! Returns a 3d ray which would go through the 2d screen coodinates.
core::line3d<f32> CSceneCollisionManager::getRayFromScreenCoordinates(
	core::position2d<s32> pos, ICameraSceneNode* camera)
{
	core::line3d<f32> ln(0,0,0,0,0,0);

	if (!SceneManager)
		return ln;

	if (!camera)
		camera = SceneManager->getActiveCamera();

	if (!camera)
		return ln;

	const scene::SViewFrustum* f = camera->getViewFrustum();

	core::vector3df farLeftUp = f->getFarLeftUp();
	core::vector3df lefttoright = f->getFarRightUp() - farLeftUp;
	core::vector3df uptodown = f->getFarLeftDown() - farLeftUp;

	const core::rect<s32>& viewPort = Driver->getViewPort();
	core::dimension2d<s32> screenSize(viewPort.getWidth(), viewPort.getHeight());

	f32 dx = pos.X / (f32)screenSize.Width;
	f32 dy = pos.Y / (f32)screenSize.Height;

	if (camera->isOrthogonal())
		ln.start = f->cameraPosition + (lefttoright * (dx-0.5f)) + (uptodown * (dy-0.5f));
	else
		ln.start = f->cameraPosition;

	ln.end = farLeftUp + (lefttoright * dx) + (uptodown * dy);

	return ln;
}


//! Calculates 2d screen position from a 3d position.
core::position2d<s32> CSceneCollisionManager::getScreenCoordinatesFrom3DPosition(
	core::vector3df pos3d, ICameraSceneNode* camera)
{
	if (!SceneManager || !Driver)
		return core::position2d<s32>(-1000,-1000);

	if (!camera)
		camera = SceneManager->getActiveCamera();

	if (!camera)
		return core::position2d<s32>(-1000,-1000);

	const core::rect<s32>& viewPort = Driver->getViewPort();
	core::dimension2d<s32> dim(viewPort.getWidth(), viewPort.getHeight());

	dim.Width /= 2;
	dim.Height /= 2;

	core::matrix4 trans = camera->getProjectionMatrix();
	trans *= camera->getViewMatrix();

	f32 transformedPos[4] = { pos3d.X, pos3d.Y, pos3d.Z, 1.0f };

	trans.multiplyWith1x4Matrix(transformedPos);

	if (transformedPos[3] < 0)
		return core::position2d<s32>(-10000,-10000);

	const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
		core::reciprocal(transformedPos[3]);

	return core::position2d<s32>(
			core::round32(dim.Width * transformedPos[0] * zDiv) + dim.Width,
			dim.Height - core::round32(dim.Height * (transformedPos[1] * zDiv)));
}


inline bool CSceneCollisionManager::getLowestRoot(f32 a, f32 b, f32 c, f32 maxR, f32* root)
{
	// check if solution exists
	f32 determinant = b*b - 4.0f*a*c;

	// if determinant is negative, no solution
	if (determinant < 0.0f) return false;

	// calculate two roots: (if det==0 then x1==x2
	// but lets disregard that slight optimization)
	// burningwater: sqrt( 0) is an illegal operation.... smth should be done...

	f32 sqrtD = (f32)sqrt(determinant);

	f32 r1 = (-b - sqrtD) / (2*a);
	f32 r2 = (-b + sqrtD) / (2*a);

	// sort so x1 <= x2
	if (r1 > r2) { f32 tmp=r2; r2=r1; r1=tmp; }

	// get lowest root
	if (r1 > 0 && r1 < maxR)
	{
		*root = r1;
		return true;
	}

	// its possible that we want x2, this can happen if x1 < 0
	if (r2 > 0 && r2 < maxR)
	{
		*root = r2;
		return true;
	}

	return false;
}


} // end namespace scene
} // end namespace irr

