// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __S_MATERIAL_LAYER_H_INCLUDED__
#define __S_MATERIAL_LAYER_H_INCLUDED__

#include "matrix4.h"
#include "irrAllocator.h"

namespace irr
{
namespace video
{
	class ITexture;

	//! Texture coord clamp mode outside [0.0, 1.0]
	enum E_TEXTURE_CLAMP
	{
		//! Texture repeats
		ETC_REPEAT = 0,
		//! Texture is clamped to the last pixel
		ETC_CLAMP,
		//! Texture is clamped to the edge pixel
		ETC_CLAMP_TO_EDGE,
		//! Texture is clamped to the border pixel (if exists)
		ETC_CLAMP_TO_BORDER,
		//! Texture is alternatingly mirrored (0..1..0..1..0..)
		ETC_MIRROR
	};
	static const char* const aTextureClampNames[] = {
			"texture_clamp_repeat",
			"texture_clamp_clamp",
			"texture_clamp_clamp_to_edge",
			"texture_clamp_clamp_to_border",
			"texture_clamp_mirror", 0};

	//! Struct for holding material parameters which exist per texture layer
	class SMaterialLayer
	{
	public:
		//! Default constructor
		SMaterialLayer()
			: Texture(0),
				TextureWrap(ETC_REPEAT),
				BilinearFilter(true),
				TrilinearFilter(false),
				AnisotropicFilter(false), TextureMatrix(0)
			{}

		//! Copy constructor
		/** \param other Material layer to copy from. */
		SMaterialLayer(const SMaterialLayer& other)
		{
			// This pointer is checked during assignment
			TextureMatrix = 0;
			*this = other;
		}

		//! Destructor
		~SMaterialLayer()
		{
			MatrixAllocator.destruct(TextureMatrix);
			MatrixAllocator.deallocate(TextureMatrix); 
		}

		//! Assignment operator
		/** \param other Material layer to copy from.
		\return This material layer, updated. */
		SMaterialLayer& operator=(const SMaterialLayer& other)
		{
			Texture = other.Texture;
			if (TextureMatrix)
			{
				if (other.TextureMatrix)
					*TextureMatrix = *other.TextureMatrix;
				else
				{
					MatrixAllocator.destruct(TextureMatrix);
					MatrixAllocator.deallocate(TextureMatrix); 
					TextureMatrix = 0;
				}
			}
			else
			{
				if (other.TextureMatrix)
				{
					TextureMatrix = MatrixAllocator.allocate(1);
					MatrixAllocator.construct(TextureMatrix,*other.TextureMatrix);
				}
				else
					TextureMatrix = 0;
			}
			TextureWrap = other.TextureWrap;
			BilinearFilter = other.BilinearFilter;
			TrilinearFilter = other.TrilinearFilter;
			AnisotropicFilter = other.AnisotropicFilter;

			return *this;
		}

		//! Texture
		ITexture* Texture;

		//! Texture Clamp Mode
		E_TEXTURE_CLAMP TextureWrap;

		//! Is bilinear filtering enabled? Default: true
		bool BilinearFilter;

		//! Is trilinear filtering enabled? Default: false
		/** If the trilinear filter flag is enabled,
		the bilinear filtering flag is ignored. */
		bool TrilinearFilter;

		//! Is anisotropic filtering enabled? Default: false
		/** In Irrlicht you can use anisotropic texture filtering
		in conjunction with bilinear or trilinear texture
		filtering to improve rendering results. Primitives
		will look less blurry with this flag switched on. */
		bool AnisotropicFilter;

		//! Gets the texture transformation matrix
		/** \return Texture matrix of this layer. */
		core::matrix4& getTextureMatrix()
		{
			if (!TextureMatrix)
			{
				TextureMatrix = MatrixAllocator.allocate(1);
				MatrixAllocator.construct(TextureMatrix,core::IdentityMatrix);
			}
			return *TextureMatrix;
		}

		//! Gets the immutable texture transformation matrix
		/** \return Texture matrix of this layer. */
		const core::matrix4& getTextureMatrix() const
		{
			if (TextureMatrix)
				return *TextureMatrix;
			else
				return core::IdentityMatrix;
		}

		//! Sets the texture transformation matrix to mat
		/** \param mat New texture matrix for this layer. */
		void setTextureMatrix(const core::matrix4& mat)
		{
			if (!TextureMatrix)
			{
				TextureMatrix = MatrixAllocator.allocate(1);
				MatrixAllocator.construct(TextureMatrix,mat);
			}
			else
				*TextureMatrix = mat;
		}

		//! Inequality operator
		/** \param b Layer to compare to.
		\return True if layers are different, else false. */
		inline bool operator!=(const SMaterialLayer& b) const
		{
			bool different =
				Texture != b.Texture ||
				TextureWrap != b.TextureWrap ||
				BilinearFilter != b.BilinearFilter ||
				TrilinearFilter != b.TrilinearFilter ||
				AnisotropicFilter != b.AnisotropicFilter;
			if (different)
				return true;
			else
				different |= (TextureMatrix != b.TextureMatrix) &&
					TextureMatrix && b.TextureMatrix &&
					(*TextureMatrix != *(b.TextureMatrix));
			return different;
		}

		//! Equality operator
		/** \param b Layer to compare to.
		\return True if layers are equal, else false. */
		inline bool operator==(const SMaterialLayer& b) const
		{ return !(b!=*this); }

	private:
		friend class SMaterial;
		irr::core::irrAllocator<irr::core::matrix4> MatrixAllocator;

		//! Texture Matrix
		/** Do not access this element directly as the internal
		ressource management has to cope with Null pointers etc. */
		core::matrix4* TextureMatrix;
	};

} // end namespace video
} // end namespace irr

#endif // __S_MATERIAL_LAYER_H_INCLUDED__
