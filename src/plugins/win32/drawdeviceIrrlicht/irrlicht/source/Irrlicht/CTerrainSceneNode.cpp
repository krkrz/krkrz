// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// The code for the TerrainSceneNode is based on the GeoMipMapSceneNode
// developed by Spintz. He made it available for Irrlicht and allowed it to be
// distributed under this licence. I only modified some parts. A lot of thanks
// go to him.

#include "CTerrainSceneNode.h"
#include "CTerrainTriangleSelector.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "ICameraSceneNode.h"
#include "SMeshBufferLightMap.h"
#include "SViewFrustum.h"
#include "irrMath.h"
#include "os.h"
#include "IGUIFont.h"
#include "IFileSystem.h"
#include "IReadFile.h"
#include "ITextSceneNode.h"

namespace irr
{
namespace scene
{

	//! constructor
	CTerrainSceneNode::CTerrainSceneNode(ISceneNode* parent, ISceneManager* mgr,
			io::IFileSystem* fs, s32 id, s32 maxLOD, E_TERRAIN_PATCH_SIZE patchSize,
			const core::vector3df& position,
			const core::vector3df& rotation, 
			const core::vector3df& scale)
	: ITerrainSceneNode(parent, mgr, id, position, rotation, scale),
	TerrainData(patchSize, maxLOD, position, rotation, scale),
	VerticesToRender(0), IndicesToRender(0), DynamicSelectorUpdate(false),
	OverrideDistanceThreshold(false), UseDefaultRotationPivot(true), ForceRecalculation(false),
	OldCameraPosition(core::vector3df(-99999.9f, -99999.9f, -99999.9f)),
	OldCameraRotation(core::vector3df(-99999.9f, -99999.9f, -99999.9f)),
	CameraMovementDelta(10.0f), CameraRotationDelta(1.0f),
	TCoordScale1(1.0f), TCoordScale2(1.0f), FileSystem(fs)
	{
		#ifdef _DEBUG
		setDebugName("CTerrainSceneNode");
		#endif

		if (FileSystem)
			FileSystem->grab();

		setAutomaticCulling( scene::EAC_OFF );
	}


	//! destructor
	CTerrainSceneNode::~CTerrainSceneNode ( )
	{
		delete [] TerrainData.LODDistanceThreshold;

		delete [] TerrainData.Patches;

		if (FileSystem)
			FileSystem->drop();
	}


	//! Initializes the terrain data.  Loads the vertices from the heightMapFile
	bool CTerrainSceneNode::loadHeightMap( io::IReadFile* file, video::SColor vertexColor, s32 smoothFactor )
	{
		if( !file )
			return false;

		u32 startTime = os::Timer::getRealTime();
		video::IImage* heightMap = SceneManager->getVideoDriver()->createImageFromFile( file );

		if( !heightMap )
		{
			os::Printer::print( "Was not able to load heightmap." );
			return false;
		}

		HeightmapFile = file->getFileName();

		// Get the dimension of the heightmap data
		TerrainData.Size = heightMap->getDimension().Width;

		switch( TerrainData.PatchSize )
		{
			case ETPS_9:
				if( TerrainData.MaxLOD > 3 )
				{
					TerrainData.MaxLOD = 3;
				}
			break;
			case ETPS_17:
				if( TerrainData.MaxLOD > 4 )
				{
					TerrainData.MaxLOD = 4;
				}
			break;
			case ETPS_33:
				if( TerrainData.MaxLOD > 5 )
				{
					TerrainData.MaxLOD = 5;
				}
			break;
			case ETPS_65:
				if( TerrainData.MaxLOD > 6 )
				{
					TerrainData.MaxLOD = 6;
				}
			break;
			case ETPS_129:
				if( TerrainData.MaxLOD > 7 )
				{
					TerrainData.MaxLOD = 7;
				}
			break;
		}

		// --- Generate vertex data from heightmap ----
		// resize the vertex array for the mesh buffer one time ( makes loading faster )
		SMeshBufferLightMap* pMeshBuffer = new SMeshBufferLightMap();
		pMeshBuffer->Vertices.set_used( TerrainData.Size * TerrainData.Size );

		video::S3DVertex2TCoords vertex;
		vertex.Normal.set( 0.0f, 1.0f, 0.0f );
		vertex.Color = vertexColor;

		// Read the heightmap to get the vertex data
		// Apply positions changes, scaling changes
		const f32 tdSize = 1.0f/(f32)(TerrainData.Size-1);
		s32 index = 0;
		for( s32 x = 0; x < TerrainData.Size; ++x )
		{
			for( s32 z = 0; z < TerrainData.Size; ++z )
			{
				vertex.Pos.X = (f32)x;
				video::SColor pixelColor(heightMap->getPixel(x,z));
				vertex.Pos.Y = (f32) pixelColor.getLuminance();
				vertex.Pos.Z = (f32)z;

				vertex.TCoords.X = vertex.TCoords2.X = x * tdSize;
				vertex.TCoords.Y = vertex.TCoords2.Y = z * tdSize;

				pMeshBuffer->Vertices[index] = vertex;
				++index;
			}
		}

		// drop heightMap, no longer needed
		heightMap->drop();

		//! Terrain smoothing. Applause to DeusXL!
		// http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?p=91272#91272

		s32 run;

		for( run = 0; run < smoothFactor; ++run )
		{
			for( index = 2; index < (TerrainData.Size * TerrainData.Size - 2); ++index)
			{
				pMeshBuffer->Vertices[index].Pos.Y =
					(pMeshBuffer->Vertices[index - 2].Pos.Y + pMeshBuffer->Vertices[index - 1].Pos.Y +
					pMeshBuffer->Vertices[index + 1].Pos.Y + pMeshBuffer->Vertices[index + 2].Pos.Y) / 4.0f;
			}
		}

		for( run = 0; run < smoothFactor; ++run)
		{
			for( index = TerrainData.Size; index < (TerrainData.Size * (TerrainData.Size - 1)); ++index)
			{
				pMeshBuffer->Vertices[index].Pos.Y =
					(pMeshBuffer->Vertices[index - TerrainData.Size].Pos.Y +
					pMeshBuffer->Vertices[index + TerrainData.Size].Pos.Y ) / 2.0f;
			}
		}


		// calculate smooth normals for the vertices
		calculateNormals( pMeshBuffer );

		// add the MeshBuffer to the mesh
		Mesh.addMeshBuffer( pMeshBuffer );
		const u32 vertexCount = pMeshBuffer->getVertexCount();

		// We copy the data to the renderBuffer, after the normals have been calculated.
		RenderBuffer.Vertices.set_used( vertexCount );

		for( u32 i = 0; i < vertexCount; ++i )
		{
			RenderBuffer.Vertices[i] = pMeshBuffer->Vertices[i];
			RenderBuffer.Vertices[i].Pos *= TerrainData.Scale;
			RenderBuffer.Vertices[i].Pos += TerrainData.Position;
		}

		// We no longer need the pMeshBuffer
		pMeshBuffer->drop();

		// calculate all the necessary data for the patches and the terrain
		calculateDistanceThresholds();
		createPatches();
		calculatePatchData();

		// set the default rotation pivot point to the terrain nodes center
		TerrainData.RotationPivot = TerrainData.Center;

		// Rotate the vertices of the terrain by the rotation
		// specified.  Must be done after calculating the terrain data,
		// so we know what the current center of the terrain is.
		setRotation( TerrainData.Rotation );

		// Pre-allocate memory for indices
		RenderBuffer.Indices.set_used( TerrainData.PatchCount * TerrainData.PatchCount *
			TerrainData.CalcPatchSize * TerrainData.CalcPatchSize * 6 );

		const u32 endTime = os::Timer::getRealTime();

		c8 tmp[255];
		sprintf(tmp, "Generated terrain data (%dx%d) in %.4f seconds",
			TerrainData.Size, TerrainData.Size, ( endTime - startTime ) / 1000.0f );
		os::Printer::log( tmp );

		return true;
	}


	//! Initializes the terrain data.  Loads the vertices from the heightMapFile
	bool CTerrainSceneNode::loadHeightMapRAW( io::IReadFile* file, s32 bitsPerPixel, video::SColor vertexColor, s32 smoothFactor )
	{
		if( !file )
			return false;

		// start reading
		u32 startTime = os::Timer::getTime();

		// get file size
		const long fileSize = file->getSize();
		const s32 bytesPerPixel = bitsPerPixel / 8;

		// Get the dimension of the heightmap data
		TerrainData.Size = core::floor32(sqrtf( (f32)( fileSize / bytesPerPixel ) ));

		switch( TerrainData.PatchSize )
		{
			case ETPS_9:
				if( TerrainData.MaxLOD > 3 )
				{
					TerrainData.MaxLOD = 3;
				}
			break;
			case ETPS_17:
				if( TerrainData.MaxLOD > 4 )
				{
					TerrainData.MaxLOD = 4;
				}
			break;
			case ETPS_33:
				if( TerrainData.MaxLOD > 5 )
				{
					TerrainData.MaxLOD = 5;
				}
			break;
			case ETPS_65:
				if( TerrainData.MaxLOD > 6 )
				{
					TerrainData.MaxLOD = 6;
				}
			break;
			case ETPS_129:
				if( TerrainData.MaxLOD > 7 )
				{
					TerrainData.MaxLOD = 7;
				}
			break;
		}

		// --- Generate vertex data from heightmap ----
		// resize the vertex array for the mesh buffer one time ( makes loading faster )
		SMeshBufferLightMap* pMeshBuffer = new SMeshBufferLightMap();
		pMeshBuffer->Vertices.reallocate( TerrainData.Size * TerrainData.Size );

		video::S3DVertex2TCoords vertex;
		vertex.Normal.set( 0.0f, 1.0f, 0.0f );
		vertex.Color = vertexColor;

		// Read the heightmap to get the vertex data
		// Apply positions changes, scaling changes
		const f32 tdSize = 1.0f/(f32)(TerrainData.Size-1);
		for( s32 x = 0; x < TerrainData.Size; ++x )
		{
			for( s32 z = 0; z < TerrainData.Size; ++z )
			{
				vertex.Pos.X = (f32)x;

				if( file->read( &vertex.Pos.Y, bytesPerPixel ) != bytesPerPixel )
				{
					os::Printer::print("Error reading heightmap RAW file.");
					pMeshBuffer->drop();
					return false;
				}

				vertex.Pos.Z = (f32)z;

				vertex.TCoords.X = vertex.TCoords2.X = x * tdSize;
				vertex.TCoords.Y = vertex.TCoords2.Y = z * tdSize;

				pMeshBuffer->Vertices.push_back( vertex );
			}
		}

		//! Terrain smoothing. Applause to DeusXL!
		// http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?p=91272#91272

		s32 run;
		s32 index;

		for( run = 0; run < smoothFactor; ++run )
		{
			for( index = 2; index < (TerrainData.Size * TerrainData.Size - 2); ++index)
			{
				pMeshBuffer->Vertices[index].Pos.Y =
					(pMeshBuffer->Vertices[index - 2].Pos.Y + pMeshBuffer->Vertices[index - 1].Pos.Y +
					pMeshBuffer->Vertices[index + 1].Pos.Y + pMeshBuffer->Vertices[index + 2].Pos.Y) / 4.0f;
			}
		}

		for( run = 0; run < smoothFactor; ++run)
		{
			for( index = TerrainData.Size; index < (TerrainData.Size * (TerrainData.Size - 1)); ++index)
			{
				pMeshBuffer->Vertices[index].Pos.Y =
					(pMeshBuffer->Vertices[index - TerrainData.Size].Pos.Y +
					pMeshBuffer->Vertices[index + TerrainData.Size].Pos.Y ) / 2.0f;
			}
		}

		// calculate smooth normals for the vertices
		calculateNormals( pMeshBuffer );

		// add the MeshBuffer to the mesh
		Mesh.addMeshBuffer( pMeshBuffer );
		const u32 vertexCount = pMeshBuffer->getVertexCount();

		// We copy the data to the renderBuffer, after the normals have been calculated.
		RenderBuffer.Vertices.set_used( vertexCount );

		for( u32 i = 0; i < vertexCount; i++ )
		{
			RenderBuffer.Vertices[i] = pMeshBuffer->Vertices[i];
			RenderBuffer.Vertices[i].Pos *= TerrainData.Scale;
			RenderBuffer.Vertices[i].Pos += TerrainData.Position;
		}

		// We no longer need the pMeshBuffer
		pMeshBuffer->drop();

		// calculate all the necessary data for the patches and the terrain
		calculateDistanceThresholds();
		createPatches();
		calculatePatchData();

		// set the default rotation pivot point to the terrain nodes center
		TerrainData.RotationPivot = TerrainData.Center;

		// Rotate the vertices of the terrain by the rotation specified.  Must be done
		// after calculating the terrain data, so we know what the current center of the
		// terrain is.
		setRotation( TerrainData.Rotation );

		// Pre-allocate memory for indices
		RenderBuffer.Indices.set_used( TerrainData.PatchCount * TerrainData.PatchCount *
			TerrainData.CalcPatchSize * TerrainData.CalcPatchSize * 6 );

		u32 endTime = os::Timer::getTime();

		c8 tmp[255];
		sprintf( tmp, "Generated terrain data (%dx%d) in %.4f seconds",
			TerrainData.Size, TerrainData.Size, (endTime - startTime) / 1000.0f );
		os::Printer::print( tmp );

		return true;
	}


	//! Sets the scale of the scene node.
	//! \param scale: New scale of the node
	void CTerrainSceneNode::setScale(const core::vector3df& scale)
	{
		TerrainData.Scale = scale;
		applyTransformation();
		ForceRecalculation = true;
	}

	//! Sets the rotation of the node. This only modifies
	//! the relative rotation of the node.
	//! \param rotation: New rotation of the node in degrees.
	void CTerrainSceneNode::setRotation(const core::vector3df& rotation)
	{
		TerrainData.Rotation = rotation;
		applyTransformation();
		ForceRecalculation = true;
	}

	//! Sets the pivot point for rotation of this node.  This is useful for the TiledTerrainManager to
	//! rotate all terrain tiles around a global world point.
	//! NOTE: The default for the RotationPivot will be the center of the individual tile.
	void CTerrainSceneNode::setRotationPivot( const core::vector3df& pivot )
	{
		UseDefaultRotationPivot = false;
		TerrainData.RotationPivot = pivot;
	}

	//! Sets the position of the node.
	//! \param newpos: New postition of the scene node.
	void CTerrainSceneNode::setPosition ( const core::vector3df& newpos )
	{
		TerrainData.Position = newpos;
		applyTransformation();
		ForceRecalculation = true;
	}

	//! Apply transformation changes( scale, position, rotation )
	void CTerrainSceneNode::applyTransformation()
	{
		if( !Mesh.getMeshBufferCount() )
			return;

		TerrainData.Position = TerrainData.Position;
		video::S3DVertex2TCoords* meshVertices = (video::S3DVertex2TCoords*)Mesh.getMeshBuffer( 0 )->getVertices();
		s32 vtxCount = Mesh.getMeshBuffer( 0 )->getVertexCount();
		core::matrix4 rotMatrix;
		rotMatrix.setRotationDegrees( TerrainData.Rotation );

		for( s32 i = 0; i < vtxCount; ++i )
		{
			RenderBuffer.Vertices[i].Pos = meshVertices[i].Pos * TerrainData.Scale + TerrainData.Position;

			RenderBuffer.Vertices[i].Pos -= TerrainData.RotationPivot;
			rotMatrix.inverseRotateVect( RenderBuffer.Vertices[i].Pos );
			RenderBuffer.Vertices[i].Pos += TerrainData.RotationPivot;
		}

		calculateDistanceThresholds( true );
		calculatePatchData();
	}

	//! Updates the scene nodes indices if the camera has moved or rotated by a certain
	//! threshold, which can be changed using the SetCameraMovementDeltaThreshold and
	//! SetCameraRotationDeltaThreshold functions.  This also determines if a given patch
	//! for the scene node is within the view frustum and if it's not the indices are not
	//! generated for that patch.
	void CTerrainSceneNode::OnRegisterSceneNode()
	{
		if (!IsVisible || !SceneManager->getActiveCamera())
			return;

		preRenderLODCalculations();
		preRenderIndicesCalculations();
		ISceneNode::OnRegisterSceneNode();
		ForceRecalculation = false;
	}

	void CTerrainSceneNode::preRenderLODCalculations()
	{
		SceneManager->registerNodeForRendering( this );
		// Do Not call ISceneNode::OnRegisterSceneNode ( ), this node should have no children

		// Determine the camera rotation, based on the camera direction.
		const core::vector3df cameraPosition = SceneManager->getActiveCamera()->getAbsolutePosition();
		const core::vector3df cameraRotation = core::line3d<f32>(cameraPosition, SceneManager->getActiveCamera()->getTarget()).getVector().getHorizontalAngle();

		// Only check on the Camera's Y Rotation
		if (!ForceRecalculation)
		{
			if (( fabs(cameraRotation.X - OldCameraRotation.X) < CameraRotationDelta) &&
				( fabs(cameraRotation.Y - OldCameraRotation.Y) < CameraRotationDelta))
			{
				if ((fabs(cameraPosition.X - OldCameraPosition.X) < CameraMovementDelta) &&
					(fabs(cameraPosition.Y - OldCameraPosition.Y) < CameraMovementDelta) &&
					(fabs(cameraPosition.Z - OldCameraPosition.Z) < CameraMovementDelta))
				{
					return;
				}
			}
		}

		OldCameraPosition = cameraPosition;
		OldCameraRotation = cameraRotation;
		const SViewFrustum* frustum = SceneManager->getActiveCamera()->getViewFrustum();

		// Determine each patches LOD based on distance from camera ( and whether or not they are in
		// the view frustum ).
		for( s32 j = 0; j < TerrainData.PatchCount * TerrainData.PatchCount; ++j )
		{
			if( frustum->getBoundingBox().intersectsWithBox( TerrainData.Patches[j].BoundingBox ) )
			{
				const f32 distance = (cameraPosition.X - TerrainData.Patches[j].Center.X) * (cameraPosition.X - TerrainData.Patches[j].Center.X) +
					(cameraPosition.Y - TerrainData.Patches[j].Center.Y) * (cameraPosition.Y - TerrainData.Patches[j].Center.Y) +
					(cameraPosition.Z - TerrainData.Patches[j].Center.Z) * (cameraPosition.Z - TerrainData.Patches[j].Center.Z);

				for( s32 i = TerrainData.MaxLOD - 1; i >= 0; --i )
				{
					if( distance >= TerrainData.LODDistanceThreshold[i] )
					{
						TerrainData.Patches[j].CurrentLOD = i;
						break;
					}
					//else if( i == 0 )
					{
						// If we've turned off a patch from viewing, because of the frustum, and now we turn around and it's
						// too close, we need to turn it back on, at the highest LOD.  The if above doesn't catch this.
						TerrainData.Patches[j].CurrentLOD = 0;
					}
				}
			}
			else
			{
				TerrainData.Patches[j].CurrentLOD = -1;
			}
		}
	}

	void CTerrainSceneNode::preRenderIndicesCalculations()
	{
		IndicesToRender = 0;
		s32 index11;
		s32 index21;
		s32 index12;
		s32 index22;

		// Then generate the indices for all patches that are visible.
		for( s32 i = 0; i < TerrainData.PatchCount; ++i )
		{
			for( s32 j = 0; j < TerrainData.PatchCount; ++j )
			{
				s32 index = i * TerrainData.PatchCount + j;
				if( TerrainData.Patches[index].CurrentLOD >= 0 )
				{
					s32 x = 0;
					s32 z = 0;

					// calculate the step we take this patch, based on the patches current LOD
					s32 step = 1 << TerrainData.Patches[index].CurrentLOD;

					// Loop through patch and generate indices
					while( z < TerrainData.CalcPatchSize )
					{
						index11 = getIndex( j, i, index, x, z );
						index21 = getIndex( j, i, index, x + step, z );
						index12 = getIndex( j, i, index, x, z + step );
						index22 = getIndex( j, i, index, x + step, z + step );

						RenderBuffer.Indices[IndicesToRender++] = index12;
						RenderBuffer.Indices[IndicesToRender++] = index11;
						RenderBuffer.Indices[IndicesToRender++] = index22;
						RenderBuffer.Indices[IndicesToRender++] = index22;
						RenderBuffer.Indices[IndicesToRender++] = index11;
						RenderBuffer.Indices[IndicesToRender++] = index21;

						// increment index position horizontally
						x += step;

						if ( x >= TerrainData.CalcPatchSize ) // we've hit an edge
						{
							x = 0;
							z += step;
						}
					}
				}
			}
		}

		if ( DynamicSelectorUpdate && TriangleSelector )
		{
			CTerrainTriangleSelector* selector = (CTerrainTriangleSelector*)TriangleSelector;
			selector->setTriangleData ( this, -1 );
		}
	}


	//! Render the scene node
	void CTerrainSceneNode::render()
	{
		if (!IsVisible || !SceneManager->getActiveCamera())
			return;

		if (!Mesh.getMeshBufferCount())
			return;

		video::IVideoDriver* driver = SceneManager->getVideoDriver();

		core::matrix4 identity;
		driver->setTransform (video::ETS_WORLD, identity);

		driver->setMaterial(Mesh.getMeshBuffer(0)->getMaterial());

		// For use with geomorphing
		driver->drawVertexPrimitiveList(
			RenderBuffer.getVertices(), RenderBuffer.getVertexCount(),
			RenderBuffer.getIndices(), IndicesToRender / 3,
			video::EVT_2TCOORDS, EPT_TRIANGLES);

		// for debug purposes only:
		if (DebugDataVisible)
		{
			video::SMaterial m;
			m.Lighting = false;
			driver->setMaterial(m);
			if ( DebugDataVisible & scene::EDS_BBOX )
				driver->draw3DBox( TerrainData.BoundingBox, video::SColor(0,255,255,255));

			const s32 count = TerrainData.PatchCount * TerrainData.PatchCount;
			s32 visible = 0;
			if ( DebugDataVisible & scene::EDS_BBOX_BUFFERS )
				for( s32 j = 0; j < count; ++j )
				{
					driver->draw3DBox( TerrainData.Patches[j].BoundingBox, video::SColor(0,255,0,0));
					visible += ( TerrainData.Patches[j].CurrentLOD >= 0 );
				}

			static u32 lastTime = 0;

			const u32 now = os::Timer::getRealTime();
			if ( now - lastTime > 1000 )
			{
				char buf[64];
				sprintf ( buf, "Count: %d, Visible: %d", count, visible );
				os::Printer::print ( buf );

				lastTime = now;
			}
		}
	}

	//! Return the bounding box of the entire terrain.
	const core::aabbox3d<f32>& CTerrainSceneNode::getBoundingBox() const
	{
		return TerrainData.BoundingBox;
	}

	//! Return the bounding box of a patch
	const core::aabbox3d<f32>& CTerrainSceneNode::getBoundingBox( s32 patchX, s32 patchZ ) const
	{
		return TerrainData.Patches[patchX * TerrainData.PatchCount + patchZ].BoundingBox;
	}

	//! Gets the meshbuffer data based on a specified Level of Detail.
	//! \param mb: A reference to an SMeshBuffer object
	//! \param LOD: The Level Of Detail you want the indices from.
	void CTerrainSceneNode::getMeshBufferForLOD(SMeshBufferLightMap& mb, s32 LOD ) const
	{
		if (!Mesh.getMeshBufferCount())
			return;

		if ( LOD < 0 )
			LOD = 0;
		else if ( LOD > TerrainData.MaxLOD - 1 )
			LOD = TerrainData.MaxLOD - 1;

		s32 numVertices = Mesh.getMeshBuffer( 0 )->getVertexCount ( );
		mb.Vertices.reallocate ( numVertices );
		video::S3DVertex2TCoords* vertices = (video::S3DVertex2TCoords*)Mesh.getMeshBuffer ( 0 )->getVertices ( );

		s32 i;
		for (i=0; i<numVertices; ++i)
			mb.Vertices.push_back(vertices[i]);

		// calculate the step we take for all patches, since LOD is the same
		s32 step = 1 << LOD;
		s32 index11;
		s32 index21;
		s32 index12;
		s32 index22;

		// Generate the indices for all patches at the specified LOD
		for (i=0; i<TerrainData.PatchCount; ++i)
		{
			for (s32 j=0; j<TerrainData.PatchCount; ++j)
			{
				s32 index = i*TerrainData.PatchCount + j;
				s32 x = 0;
				s32 z = 0;

				// Loop through patch and generate indices
				while (z < TerrainData.CalcPatchSize)
				{
					index11 = getIndex( j, i, index, x, z );
					index21 = getIndex( j, i, index, x + step, z );
					index12 = getIndex( j, i, index, x, z + step );
					index22 = getIndex( j, i, index, x + step, z + step );

					mb.Indices.push_back( index12 );
					mb.Indices.push_back( index11 );
					mb.Indices.push_back( index22 );
					mb.Indices.push_back( index22 );
					mb.Indices.push_back( index11 );
					mb.Indices.push_back( index21 );

					// increment index position horizontally
					x += step;

					if (x >= TerrainData.CalcPatchSize) // we've hit an edge
					{
						x = 0;
						z += step;
					}
				}
			}
		}
	}

	//! Gets the indices for a specified patch at a specified Level of Detail.
	//! \param mb: A reference to an array of u32 indices.
	//! \param patchX: Patch x coordinate.
	//! \param patchZ: Patch z coordinate.
	//! \param LOD: The level of detail to get for that patch.  If -1, then get
	//! the CurrentLOD.  If the CurrentLOD is set to -1, meaning it's not shown,
	//! then it will retrieve the triangles at the highest LOD ( 0 ).
	//! \return: Number if indices put into the buffer.
	s32 CTerrainSceneNode::getIndicesForPatch(core::array<u32>& indices, s32 patchX, s32 patchZ, s32 LOD)
	{
		if ( patchX < 0 || patchX > TerrainData.PatchCount - 1 || patchZ < 0 || patchZ > TerrainData.PatchCount - 1 )
			return -1;

		if ( LOD < -1 || LOD > TerrainData.MaxLOD - 1 )
			return -1;

		s32 rv = 0;
		core::array<s32> cLODs;
		bool setLODs = false;

		// If LOD of -1 was passed in, use the CurrentLOD of the patch specified
		if ( LOD == -1 )
		{
			LOD = TerrainData.Patches[patchX * TerrainData.PatchCount + patchZ].CurrentLOD;
		}
		else
		{
			getCurrentLODOfPatches(cLODs);
			setCurrentLODOfPatches(LOD);
			setLODs = true;
		}

		if ( LOD < 0 )
			return -2; // Patch not visible, don't generate indices.

		// calculate the step we take for this LOD
		s32 step = 1 << LOD;

		// Generate the indices for the specified patch at the specified LOD
		s32 index = patchX * TerrainData.PatchCount + patchZ;

		s32 x = 0;
		s32 z = 0;
		s32 index11;
		s32 index21;
		s32 index12;
		s32 index22;

		indices.set_used ( TerrainData.PatchSize * TerrainData.PatchSize * 6 );

		// Loop through patch and generate indices
		while (z<TerrainData.CalcPatchSize)
		{
			index11 = getIndex( patchZ, patchX, index, x, z );
			index21 = getIndex( patchZ, patchX, index, x + step, z );
			index12 = getIndex( patchZ, patchX, index, x, z + step );
			index22 = getIndex( patchZ, patchX, index, x + step, z + step );

			indices[rv++] = index12;
			indices[rv++] = index11;
			indices[rv++] = index22;
			indices[rv++] = index22;
			indices[rv++] = index11;
			indices[rv++] = index21;

			// increment index position horizontally
			x += step;

			if (x >= TerrainData.CalcPatchSize) // we've hit an edge
			{
				x = 0;
				z += step;
			}
		}

		if ( setLODs )
			setCurrentLODOfPatches (cLODs);

		return rv;
	}

	//! Populates an array with the CurrentLOD of each patch.
	//! \param LODs: A reference to a core::array<s32> to hold the values
	//! \return Returns the number of elements in the array
	s32 CTerrainSceneNode::getCurrentLODOfPatches(core::array<s32>& LODs) const
	{
		s32 numLODs;
		LODs.clear ( );

		for ( numLODs = 0; numLODs < TerrainData.PatchCount * TerrainData.PatchCount; numLODs++ )
			LODs.push_back ( TerrainData.Patches[numLODs].CurrentLOD );

		return LODs.size();
	}


	//! Manually sets the LOD of a patch
	//! \param patchX: Patch x coordinate.
	//! \param patchZ: Patch z coordinate.
	//! \param LOD: The level of detail to set the patch to.
	void CTerrainSceneNode::setLODOfPatch( s32 patchX, s32 patchZ, s32 LOD )
	{
		TerrainData.Patches[patchX * TerrainData.PatchCount + patchZ].CurrentLOD = LOD;
	}


	//! Override the default generation of distance thresholds for determining the LOD a patch
	//! is rendered at.
	bool CTerrainSceneNode::overrideLODDistance(s32 LOD, f64 newDistance)
	{
		OverrideDistanceThreshold = true;

		if ( LOD < 0 || LOD > TerrainData.MaxLOD - 1 )
			return false;

		TerrainData.LODDistanceThreshold[LOD] = newDistance * newDistance;

		return true;
	}

	//! Creates a planar texture mapping on the terrain
	//! \param resolution: resolution of the planar mapping. This is the value
	//! specifying the relation between world space and texture coordinate space.
	void CTerrainSceneNode::scaleTexture(f32 resolution, f32 resolution2)
	{
		TCoordScale1 = resolution;
		TCoordScale2 = resolution2;

		const f32 resBySize = resolution / (f32)(TerrainData.Size-1);
		const f32 res2BySize = resolution2 / (f32)(TerrainData.Size-1);
		u32 index = 0;
		f32 xval = 0, zval;
		f32 x2val = 0, z2val=0;
		for (s32 x=0; x<TerrainData.Size; ++x)
		{
			zval=z2val=0;
			for (s32 z=0; z<TerrainData.Size; ++z)
			{
				RenderBuffer.Vertices[index].TCoords.X = xval;
				RenderBuffer.Vertices[index].TCoords.Y = zval;

				if ( resolution2 == 0 )
				{
					RenderBuffer.Vertices[index].TCoords2 = RenderBuffer.Vertices[index].TCoords;
				}
				else
				{
					RenderBuffer.Vertices[index].TCoords2.X = x2val;
					RenderBuffer.Vertices[index].TCoords2.Y = z2val;
				}
				++index;
				zval += resBySize;
				z2val += res2BySize;
			}
			xval += resBySize;
			x2val += res2BySize;
		}
	}

	//! used to get the indices when generating index data for patches at varying levels of detail.
	u32 CTerrainSceneNode::getIndex(const s32 PatchX, const s32 PatchZ,
					const s32 PatchIndex, u32 vX, u32 vZ) const
	{
		// top border
		if (vZ == 0)
		{
			if (TerrainData.Patches[PatchIndex].Top &&
				TerrainData.Patches[PatchIndex].CurrentLOD < TerrainData.Patches[PatchIndex].Top->CurrentLOD &&
				(vX % ( 1 << TerrainData.Patches[PatchIndex].Top->CurrentLOD)) != 0 )
			{
				vX -= vX % ( 1 << TerrainData.Patches[PatchIndex].Top->CurrentLOD );
			}
		}
		else
		if ( vZ == (u32)TerrainData.CalcPatchSize ) // bottom border
		{
			if (TerrainData.Patches[PatchIndex].Bottom &&
				TerrainData.Patches[PatchIndex].CurrentLOD < TerrainData.Patches[PatchIndex].Bottom->CurrentLOD &&
				(vX % ( 1 << TerrainData.Patches[PatchIndex].Bottom->CurrentLOD)) != 0)
			{
				vX -= vX % ( 1 << TerrainData.Patches[PatchIndex].Bottom->CurrentLOD );
			}
		}

		// left border
		if ( vX == 0 )
		{
			if (TerrainData.Patches[PatchIndex].Left &&
				TerrainData.Patches[PatchIndex].CurrentLOD < TerrainData.Patches[PatchIndex].Left->CurrentLOD &&
				( vZ % ( 1 << TerrainData.Patches[PatchIndex].Left->CurrentLOD ) ) != 0)
			{
				vZ -= vZ % ( 1 << TerrainData.Patches[PatchIndex].Left->CurrentLOD );
			}
		}
		else
		if  ( vX == (u32)TerrainData.CalcPatchSize ) // right border
		{
			if (TerrainData.Patches[PatchIndex].Right &&
				TerrainData.Patches[PatchIndex].CurrentLOD < TerrainData.Patches[PatchIndex].Right->CurrentLOD &&
				( vZ %  ( 1 << TerrainData.Patches[PatchIndex].Right->CurrentLOD ) ) != 0)
			{
				vZ -= vZ % ( 1 << TerrainData.Patches[PatchIndex].Right->CurrentLOD );
			}
		}

		if ( vZ >= (u32)TerrainData.PatchSize )
			vZ = TerrainData.CalcPatchSize;

		if ( vX >= (u32)TerrainData.PatchSize )
			vX = TerrainData.CalcPatchSize;

		return (vZ + ((TerrainData.CalcPatchSize) * PatchZ)) * TerrainData.Size +
			(vX + ((TerrainData.CalcPatchSize) * PatchX));
	}

	//! calculate smooth normals
	void CTerrainSceneNode::calculateNormals ( SMeshBufferLightMap* pMeshBuffer )
	{
		s32 count;
		core::vector3df a, b, c, t;

		for (s32 x=0; x<TerrainData.Size; ++x)
		{
			for (s32 z=0; z<TerrainData.Size; ++z)
			{
				count = 0;
				core::vector3df normal;

				// top left
				if (x>0 && z>0)
				{
					a = pMeshBuffer->Vertices[(x-1)*TerrainData.Size+z-1].Pos;
					b = pMeshBuffer->Vertices[(x-1)*TerrainData.Size+z].Pos;
					c = pMeshBuffer->Vertices[x*TerrainData.Size+z].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					a = pMeshBuffer->Vertices[(x-1)*TerrainData.Size+z-1].Pos;
					b = pMeshBuffer->Vertices[x*TerrainData.Size+z-1].Pos;
					c = pMeshBuffer->Vertices[x*TerrainData.Size+z].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					count += 2;
				}

				// top right
				if (x>0 && z<TerrainData.Size-1)
				{
					a = pMeshBuffer->Vertices[(x-1)*TerrainData.Size+z].Pos;
					b = pMeshBuffer->Vertices[(x-1)*TerrainData.Size+z+1].Pos;
					c = pMeshBuffer->Vertices[x*TerrainData.Size+z+1].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					a = pMeshBuffer->Vertices[(x-1)*TerrainData.Size+z].Pos;
					b = pMeshBuffer->Vertices[x*TerrainData.Size+z+1].Pos;
					c = pMeshBuffer->Vertices[x*TerrainData.Size+z].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					count += 2;
				}

				// bottom right
				if (x<TerrainData.Size-1 && z<TerrainData.Size-1)
				{
					a = pMeshBuffer->Vertices[x*TerrainData.Size+z+1].Pos;
					b = pMeshBuffer->Vertices[x*TerrainData.Size+z].Pos;
					c = pMeshBuffer->Vertices[(x+1)*TerrainData.Size+z+1].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					a = pMeshBuffer->Vertices[x*TerrainData.Size+z+1].Pos;
					b = pMeshBuffer->Vertices[(x+1)*TerrainData.Size+z+1].Pos;
					c = pMeshBuffer->Vertices[(x+1)*TerrainData.Size+z].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					count += 2;
				}

				// bottom left
				if (x<TerrainData.Size-1 && z>0)
				{
					a = pMeshBuffer->Vertices[x*TerrainData.Size+z-1].Pos;
					b = pMeshBuffer->Vertices[x*TerrainData.Size+z].Pos;
					c = pMeshBuffer->Vertices[(x+1)*TerrainData.Size+z].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					a = pMeshBuffer->Vertices[x*TerrainData.Size+z-1].Pos;
					b = pMeshBuffer->Vertices[(x+1)*TerrainData.Size+z].Pos;
					c = pMeshBuffer->Vertices[(x+1)*TerrainData.Size+z-1].Pos;
					b -= a;
					c -= a;
					t = b.crossProduct ( c );
					t.normalize ( );
					normal += t;

					count += 2;
				}

				if ( count != 0 )
				{
					normal.normalize ( );
				}
				else
				{
					normal.set( 0.0f, 1.0f, 0.0f );
				}

				pMeshBuffer->Vertices[x * TerrainData.Size + z].Normal = normal;
			}
		}
	}

	//! create patches, stuff that needs to be done only once for patches goes here.
	void CTerrainSceneNode::createPatches()
	{
		TerrainData.PatchCount = (TerrainData.Size - 1) / ( TerrainData.CalcPatchSize );

		if (TerrainData.Patches)
			delete [] TerrainData.Patches;

		TerrainData.Patches = new SPatch[TerrainData.PatchCount * TerrainData.PatchCount];
	}

	//! used to calculate the internal STerrainData structure both at creation and after scaling/position calls.
	void CTerrainSceneNode::calculatePatchData()
	{
		// Reset the Terrains Bounding Box for re-calculation
		TerrainData.BoundingBox = core::aabbox3df ( 999999.9f, 999999.9f, 999999.9f, -999999.9f, -999999.9f, -999999.9f );

		for( s32 x = 0; x < TerrainData.PatchCount; ++x )
		{
			for( s32 z = 0; z < TerrainData.PatchCount; ++z )
			{
				s32 index = x * TerrainData.PatchCount + z;
				TerrainData.Patches[index].CurrentLOD = 0;

				// For each patch, calculate the bounding box ( mins and maxes )
				TerrainData.Patches[index].BoundingBox =  core::aabbox3df (999999.9f, 999999.9f, 999999.9f,
					-999999.9f, -999999.9f, -999999.9f );

				for( s32 xx = x*(TerrainData.CalcPatchSize); xx <= ( x + 1 ) * TerrainData.CalcPatchSize; ++xx )
					for( s32 zz = z*(TerrainData.CalcPatchSize); zz <= ( z + 1 ) * TerrainData.CalcPatchSize; ++zz )
						TerrainData.Patches[index].BoundingBox.addInternalPoint( RenderBuffer.Vertices[xx * TerrainData.Size + zz].Pos );

				// Reconfigure the bounding box of the terrain as a whole
				TerrainData.BoundingBox.addInternalBox( TerrainData.Patches[index].BoundingBox );

				// get center of Patch
				TerrainData.Patches[index].Center = TerrainData.Patches[index].BoundingBox.getCenter();

				// Assign Neighbours
				// Top
				if( x > 0 )
					TerrainData.Patches[index].Top = &TerrainData.Patches[(x-1) * TerrainData.PatchCount + z];
				else
					TerrainData.Patches[index].Top = 0;

				// Bottom
				if( x < TerrainData.PatchCount - 1 )
					TerrainData.Patches[index].Bottom = &TerrainData.Patches[(x+1) * TerrainData.PatchCount + z];
				else
					TerrainData.Patches[index].Bottom = 0;

				// Left
				if( z > 0 )
					TerrainData.Patches[index].Left = &TerrainData.Patches[x * TerrainData.PatchCount + z - 1];
				else
					TerrainData.Patches[index].Left = 0;

				// Right
				if( z < TerrainData.PatchCount - 1 )
					TerrainData.Patches[index].Right = &TerrainData.Patches[x * TerrainData.PatchCount + z + 1];
				else
					TerrainData.Patches[index].Right = 0;
			}
		}

		// get center of Terrain
		TerrainData.Center = TerrainData.BoundingBox.getCenter();

		// if the default rotation pivot is still being used, update it.
		if( UseDefaultRotationPivot )
		{
			TerrainData.RotationPivot = TerrainData.Center;
		}
	}


	//! used to calculate or recalculate the distance thresholds
	void CTerrainSceneNode::calculateDistanceThresholds(bool scalechanged)
	{
		// Only update the LODDistanceThreshold if it's not manually changed
		if (!OverrideDistanceThreshold)
		{
			if( TerrainData.LODDistanceThreshold )
			{
				delete [] TerrainData.LODDistanceThreshold;
			}

			// Determine new distance threshold for determining what LOD to draw patches at
			TerrainData.LODDistanceThreshold = new f64[TerrainData.MaxLOD];

			for (s32 i=0; i<TerrainData.MaxLOD; ++i)
			{
				TerrainData.LODDistanceThreshold[i] =
					(TerrainData.PatchSize * TerrainData.PatchSize) *
					(TerrainData.Scale.X * TerrainData.Scale.Z) *
					((i+1+ i / 2) * (i+1+ i / 2));
			}
		}
	}

	void CTerrainSceneNode::setCurrentLODOfPatches(s32 lod)
	{
		for (s32 i=0; i< TerrainData.PatchCount * TerrainData.PatchCount; ++i)
			TerrainData.Patches[i].CurrentLOD = lod;
	}

	void CTerrainSceneNode::setCurrentLODOfPatches(const core::array<s32>& lodarray)
	{
		for (s32 i=0; i<TerrainData.PatchCount * TerrainData.PatchCount; ++i)
			TerrainData.Patches[i].CurrentLOD = lodarray[i];
	}


	//! Gets the height
	f32 CTerrainSceneNode::getHeight( f32 x, f32 z ) const
	{
		if (!Mesh.getMeshBufferCount())
			return 0;

		f32 height = -999999.9f;

		core::matrix4 rotMatrix;
		rotMatrix.setRotationDegrees( TerrainData.Rotation );
		core::vector3df pos( x, 0.0f, z );
		rotMatrix.rotateVect( pos );
		pos -= TerrainData.Position;
		pos /= TerrainData.Scale;

		s32 X(core::floor32( pos.X ));
		s32 Z(core::floor32( pos.Z ));

		if( X >= 0 && X < TerrainData.Size && Z >= 0 && Z < TerrainData.Size )
		{
			const video::S3DVertex2TCoords* Vertices = (const video::S3DVertex2TCoords*)Mesh.getMeshBuffer( 0 )->getVertices();
			const core::vector3df& a = Vertices[ X * TerrainData.Size + Z ].Pos;
			const core::vector3df& b = Vertices[ (X + 1) * TerrainData.Size + Z ].Pos;
			const core::vector3df& c = Vertices[ X * TerrainData.Size + ( Z + 1 ) ].Pos;
			const core::vector3df& d = Vertices[ (X + 1) * TerrainData.Size + ( Z + 1 ) ].Pos;

			// offset from integer position
			const f32 dx = pos.X - X;
			const f32 dz = pos.Z - Z;

			if( dx > dz )
				height = a.Y + (d.Y - b.Y)*dz + (b.Y - a.Y)*dx;
			else
				height = a.Y + (d.Y - c.Y)*dx + (c.Y - a.Y)*dz;

			height *= TerrainData.Scale.Y;
			height += TerrainData.Position.Y;
		}

		return height;
	}


	//! Writes attributes of the scene node.
	void CTerrainSceneNode::serializeAttributes(io::IAttributes* out,
				io::SAttributeReadWriteOptions* options) const
	{
		ISceneNode::serializeAttributes(out, options);

		out->addString("Heightmap", HeightmapFile.c_str());
		out->addFloat("TextureScale1", TCoordScale1);
		out->addFloat("TextureScale2", TCoordScale2);
	}


	//! Reads attributes of the scene node.
	void CTerrainSceneNode::deserializeAttributes(io::IAttributes* in,
													io::SAttributeReadWriteOptions* options)
	{
		core::stringc newHeightmap = in->getAttributeAsString("Heightmap");
		f32 tcoordScale1 = in->getAttributeAsFloat("TextureScale1");
		f32 tcoordScale2 = in->getAttributeAsFloat("TextureScale2");

		// set possible new heightmap

		if (newHeightmap.size() > 0 && 
			newHeightmap != HeightmapFile)
		{
			io::IReadFile* file = FileSystem->createAndOpenFile(newHeightmap.c_str());
			if (file)
			{
				loadHeightMap(file, video::SColor(255,255,255,255), 0);
				file->drop();
			}	
			else
				os::Printer::log("could not open heightmap", newHeightmap.c_str());
		}

		// set possible new scale

		if (core::equals(tcoordScale1, 0.f))
			tcoordScale1 = 1.0f;

		if (core::equals(tcoordScale2, 0.f))
			tcoordScale2 = 1.0f;

		if (!core::equals(tcoordScale1, TCoordScale1) ||
			!core::equals(tcoordScale2, TCoordScale2))
		{
			scaleTexture(tcoordScale1, tcoordScale2);
		}

		ISceneNode::deserializeAttributes(in, options);
	}


	//! Creates a clone of this scene node and its children.
	ISceneNode* CTerrainSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
	{
		if (!newParent)
			newParent = Parent;
		if (!newManager)
			newManager = SceneManager;

		CTerrainSceneNode* nb = new CTerrainSceneNode(
			newParent, newManager, FileSystem, ID, 
			4, ETPS_17, getPosition(), getRotation(), getScale());

		nb->cloneMembers(this, newManager);
		
		// instead of cloning the data structures, recreate the terrain.
		// (temporary solution)

		// load file

		io::IReadFile* file = FileSystem->createAndOpenFile(HeightmapFile.c_str());
		if (file)
		{
			nb->loadHeightMap(file, video::SColor(255,255,255,255), 0);
			file->drop();
		}	

		// scale textures

		nb->scaleTexture(TCoordScale1, TCoordScale2);

		// copy materials

		for (unsigned int m = 0; m<Mesh.getMeshBufferCount(); ++m)
		{
			if (nb->Mesh.getMeshBufferCount()>m &&
				nb->Mesh.getMeshBuffer(m) &&
				Mesh.getMeshBuffer(m))
			{
				nb->Mesh.getMeshBuffer(m)->getMaterial() = 
					Mesh.getMeshBuffer(m)->getMaterial();
			}
		}

		nb->RenderBuffer.Material = RenderBuffer.Material;

		// finish

		nb->drop();
		return nb;
	}

} // end namespace scene
} // end namespace irr

