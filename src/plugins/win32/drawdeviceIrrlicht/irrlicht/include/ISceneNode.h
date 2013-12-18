// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_SCENE_NODE_H_INCLUDED__
#define __I_SCENE_NODE_H_INCLUDED__

#include "IAttributeExchangingObject.h"
#include "ESceneNodeTypes.h"
#include "ECullingTypes.h"
#include "EDebugSceneTypes.h"
#include "ISceneNodeAnimator.h"
#include "ITriangleSelector.h"
#include "SMaterial.h"
#include "irrString.h"
#include "aabbox3d.h"
#include "matrix4.h"
#include "irrList.h"
#include "IAttributes.h"

namespace irr
{
namespace scene
{
	class ISceneManager;

	//! Scene node interface.
	/** A scene node is a node in the hierarchical scene graph. Every scene
	node may have children, which are also scene nodes. Children move
	relative to their parent's position. If the parent of a node is not
	visible, its children won't be visible either. In this way, it is for
	example easily possible to attach a light to a moving car, or to place
	a walking character on a moving platform on a moving ship.
	*/
	class ISceneNode : public io::IAttributeExchangingObject
	{
	public:

		//! Constructor
		ISceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id=-1,
				const core::vector3df& position = core::vector3df(0,0,0),
				const core::vector3df& rotation = core::vector3df(0,0,0),
				const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f))
			: RelativeTranslation(position), RelativeRotation(rotation), RelativeScale(scale),
				Parent(parent), ID(id), SceneManager(mgr), TriangleSelector(0),
				AutomaticCullingState(EAC_BOX), IsVisible(true),
				DebugDataVisible(EDS_OFF), IsDebugObject(false)
		{
			if (Parent)
				Parent->addChild(this);

			updateAbsolutePosition();
		}


		//! Destructor
		virtual ~ISceneNode()
		{
			// delete all children
			removeAll();

			// delete all animators
			core::list<ISceneNodeAnimator*>::Iterator ait = Animators.begin();
			for (; ait != Animators.end(); ++ait)
				(*ait)->drop();

			if (TriangleSelector)
				TriangleSelector->drop();
		}


		//! This method is called just before the rendering process of the whole scene.
		/** Nodes may register themselves in the render pipeline during this call,
		precalculate the geometry which should be renderered, and prevent their
		children from being able to register themselves if they are clipped by simply
		not calling their OnRegisterSceneNode method.
		If you are implementing your own scene node, you should overwrite this method
		with an implementation code looking like this:
		\code
		if (IsVisible)
			SceneManager->registerNodeForRendering(this);

		ISceneNode::OnRegisterSceneNode();
		\endcode
		*/
		virtual void OnRegisterSceneNode()
		{
			if (IsVisible)
			{
				core::list<ISceneNode*>::Iterator it = Children.begin();
				for (; it != Children.end(); ++it)
					(*it)->OnRegisterSceneNode();
			}
		}


		//! OnAnimate() is called just before rendering the whole scene.
		/** Nodes may calculate or store animations here, and may do other useful things,
		depending on what they are. Also, OnAnimate() should be called for all
		child scene nodes here. This method will be called once per frame, independent
		of whether the scene node is visible or not.
		\param timeMs Current time in milliseconds. */
		virtual void OnAnimate(u32 timeMs)
		{
			if (IsVisible)
			{
				// animate this node with all animators

				core::list<ISceneNodeAnimator*>::Iterator ait = Animators.begin();
				for (; ait != Animators.end(); ++ait)
					(*ait)->animateNode(this, timeMs);

				// update absolute position
				updateAbsolutePosition();

				// perform the post render process on all children

				core::list<ISceneNode*>::Iterator it = Children.begin();
				for (; it != Children.end(); ++it)
					(*it)->OnAnimate(timeMs);
			}
		}


		//! Renders the node.
		virtual void render() = 0;


		//! Returns the name of the node.
		/** \return Name as character string. */
		virtual const c8* getName() const
		{
			return Name.c_str();
		}


		//! Sets the name of the node.
		/** \param name New name of the scene node. */
		virtual void setName(const c8* name)
		{
			Name = name;
		}


		//! Get the axis aligned, not transformed bounding box of this node.
		/** This means that if this node is an animated 3d character,
		moving in a room, the bounding box will always be around the
		origin. To get the box in real world coordinates, just
		transform it with the matrix you receive with
		getAbsoluteTransformation() or simply use
		getTransformedBoundingBox(), which does the same.
		\return The non-transformed bounding box. */
		virtual const core::aabbox3d<f32>& getBoundingBox() const = 0;


		//! Get the axis aligned, transformed and animated absolute bounding box of this node.
		/** \return The transformed bounding box. */
		virtual core::aabbox3d<f32> getTransformedBoundingBox() const
		{
			core::aabbox3d<f32> box = getBoundingBox();
			AbsoluteTransformation.transformBox(box);
			return box;
		}


		//! Get the absolute transformation of the node. Is recalculated every OnAnimate()-call.
		//! \return The absolute transformation matrix.
		const core::matrix4& getAbsoluteTransformation() const
		{
			return AbsoluteTransformation;
		}


		//! Returns the relative transformation of the scene node.
		/** The relative transformation is stored internally as 3
		vectors: translation, rotation and scale. To get the relative
		transformation matrix, it is calculated from these values.
		\return The relative transformation matrix. */
		virtual core::matrix4 getRelativeTransformation() const
		{
			core::matrix4 mat;
			mat.setRotationDegrees(RelativeRotation);
			mat.setTranslation(RelativeTranslation);

			if (RelativeScale != core::vector3df(1.f,1.f,1.f))
			{
				core::matrix4 smat;
				smat.setScale(RelativeScale);
				mat *= smat;
			}

			return mat;
		}


		//! Returns true if the node is visible.
		/** This is only an option set by the user, but has nothing to
		do with geometry culling
		\return The visibility of the node, true means visible. */
		virtual bool isVisible() const
		{
			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
			return IsVisible;
		}


		//! Sets if the node should be visible or not.
		/** All children of this node won't be visible either, when set
		to false.
		\param isVisible If the node shall be visible. */
		virtual void setVisible(bool isVisible)
		{
			IsVisible = isVisible;
		}


		//! Get the id of the scene node.
		/** This id can be used to identify the node.
		\return The id. */
		virtual s32 getID() const
		{
			return ID;
		}


		//! Sets the id of the scene node.
		/** This id can be used to identify the node.
		\param id The new id. */
		virtual void setID(s32 id)
		{
			ID = id;
		}


		//! Adds a child to this scene node.
		/** If the scene node already has a parent it is first removed
		from the other parent.
		\param child A pointer to the new child. */
		virtual void addChild(ISceneNode* child)
		{
			if (child && (child != this))
			{
				child->grab();
				child->remove(); // remove from old parent
				Children.push_back(child);
				child->Parent = this;
			}
		}


		//! Removes a child from this scene node.
		/** \param child A pointer to the new child.
		\return True if the child was removed, and false if not,
		e.g. because it couldn't be found in the children list. */
		virtual bool removeChild(ISceneNode* child)
		{
			core::list<ISceneNode*>::Iterator it = Children.begin();
			for (; it != Children.end(); ++it)
				if ((*it) == child)
				{
					(*it)->Parent = 0;
					(*it)->drop();
					Children.erase(it);
					return true;
				}

			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
			return false;
		}


		//! Removes all children of this scene node
		virtual void removeAll()
		{
			core::list<ISceneNode*>::Iterator it = Children.begin();
			for (; it != Children.end(); ++it)
			{
				(*it)->Parent = 0;
				(*it)->drop();
			}

			Children.clear();
		}


		//! Removes this scene node from the scene, deleting it.
		virtual void remove()
		{
			if (Parent)
				Parent->removeChild(this);
		}


		//! Adds an animator which should animate this node.
		/** \param animator A pointer to the new animator. */
		virtual void addAnimator(ISceneNodeAnimator* animator)
		{
			if (animator)
			{
				Animators.push_back(animator);
				animator->grab();
			}
		}


		//! Get a list of all scene node animators.
		/** \return The list of animators attached to this node. */
		const core::list<ISceneNodeAnimator*>& getAnimators() const
		{
			return Animators;
		}


		//! Removes an animator from this scene node.
		/** \param animator A pointer to the animator to be deleted. */
		virtual void removeAnimator(ISceneNodeAnimator* animator)
		{
			core::list<ISceneNodeAnimator*>::Iterator it = Animators.begin();
			for (; it != Animators.end(); ++it)
				if ((*it) == animator)
				{
					(*it)->drop();
					Animators.erase(it);
					return;
				}
		}


		//! Removes all animators from this scene node.
		virtual void removeAnimators()
		{
			core::list<ISceneNodeAnimator*>::Iterator it = Animators.begin();
			for (; it != Animators.end(); ++it)
				(*it)->drop();

			Animators.clear();
		}


		//! Returns the material based on the zero based index i.
		/** To get the amount of materials used by this scene node, use
		getMaterialCount(). This function is needed for inserting the
		node into the scene hierarchy at an optimal position for
		minimizing renderstate changes, but can also be used to
		directly modify the material of a scene node.
		\param num Zero based index. The maximal value is getMaterialCount() - 1.
		\return The material at that index. */
		virtual video::SMaterial& getMaterial(u32 num)
		{
			return *((video::SMaterial*)0);
		}


		//! Get amount of materials used by this scene node.
		/** \return Current amount of materials of this scene node. */
		virtual u32 getMaterialCount() const
		{
			return 0;
		}


		//! Sets all material flags at once to a new value.
		/** Useful, for example, if you want the whole mesh to be
		affected by light.
		\param flag Which flag of all materials to be set.
		\param newvalue New value of that flag. */
		void setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue)
		{
			for (u32 i=0; i<getMaterialCount(); ++i)
				getMaterial(i).setFlag(flag, newvalue);
		}


		//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
		/** \param textureLayer Layer of texture to be set. Must be a
		value smaller than MATERIAL_MAX_TEXTURES.
		\param texture New texture to be used. */
		void setMaterialTexture(u32 textureLayer, video::ITexture* texture)
		{
			if (textureLayer >= video::MATERIAL_MAX_TEXTURES)
				return;

			for (u32 i=0; i<getMaterialCount(); ++i)
				getMaterial(i).setTexture(textureLayer, texture);
		}


		//! Sets the material type of all materials in this scene node to a new material type.
		/** \param newType New type of material to be set. */
		void setMaterialType(video::E_MATERIAL_TYPE newType)
		{
			for (u32 i=0; i<getMaterialCount(); ++i)
				getMaterial(i).MaterialType = newType;
		}


		//! Gets the relative scale of the scene node.
		/** \return The scale of the scene node. */
		virtual core::vector3df getScale() const
		{
			return RelativeScale;
		}


		//! Sets the relative scale of the scene node.
		/** \param scale New scale of the node */
		virtual void setScale(const core::vector3df& scale)
		{
			RelativeScale = scale;
		}


		//! Gets the rotation of the node.
		/** Note that this is the relative rotation of the node.
		\return Current relative rotation of the scene node. */
		virtual const core::vector3df& getRotation() const
		{
			return RelativeRotation;
		}


		//! Sets the rotation of the node.
		/** This only modifies the relative rotation of the node.
		\param rotation New rotation of the node in degrees. */
		virtual void setRotation(const core::vector3df& rotation)
		{
			RelativeRotation = rotation;
		}


		//! Gets the position of the node.
		/** Note that the position is relative to the parent.
		\return The current position of the node relative to the parent. */
		virtual core::vector3df getPosition() const
		{
			return RelativeTranslation;
		}


		//! Sets the position of the node.
		/** Note that the position is relative to the parent.
		\param newpos New relative postition of the scene node. */
		virtual void setPosition(const core::vector3df& newpos)
		{
			RelativeTranslation = newpos;
		}


		//! Gets the abolute position of the node.
		/** \return The current absolute position of the scene node. */
		virtual core::vector3df getAbsolutePosition() const
		{
			return AbsoluteTransformation.getTranslation();
		}


		//! Enables or disables automatic culling based on the bounding box.
		/** Automatic culling is enabled by default. Note that not
		all SceneNodes support culling and that some nodes always cull
		their geometry because it is their only reason for existence,
		for example the OctreeSceneNode.
		\param state The culling state to be used. */
		void setAutomaticCulling( E_CULLING_TYPE state)
		{
			AutomaticCullingState = state;
		}


		//! Gets the automatic culling state.
		/** \return The automatic culling state. */
		E_CULLING_TYPE getAutomaticCulling() const
		{
			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
			return AutomaticCullingState;
		}


		//! Sets if debug data like bounding boxes should be drawn.
		/** A bitwise OR of the types is supported.
		Please note that not all scene nodes support this feature.
		\param state The debug data visibility state to be used. */
		virtual void setDebugDataVisible(s32 state)
		{
			DebugDataVisible = state;
		}

		//! Returns if debug data like bounding boxes are drawn.
		/** \return A bitwise OR of the debug data values currently visible. */
		s32 isDebugDataVisible() const
		{
			return DebugDataVisible;
		}


		//! Sets if this scene node is a debug object.
		/** Debug objects have some special properties, for example they can be easily
		excluded from collision detection or from serialization, etc. */
		void setIsDebugObject(bool debugObject)
		{
			IsDebugObject = debugObject;
		}


		//! Returns if this scene node is a debug object.
		/** Debug objects have some special properties, for example they can be easily
		excluded from collision detection or from serialization, etc.
		\return If this node is a debug object, true is returned. */
		bool isDebugObject() const
		{
			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
			return IsDebugObject;
		}


		//! Returns a const reference to the list of all children.
		/** \return The list of all children of this node. */
		const core::list<ISceneNode*>& getChildren() const
		{
			return Children;
		}


		//! Changes the parent of the scene node.
		/** \param newParent The new parent to be used. */
		virtual void setParent(ISceneNode* newParent)
		{
			grab();
			remove();

			Parent = newParent;

			if (Parent)
				Parent->addChild(this);

			drop();
		}


		//! Returns the triangle selector attached to this scene node.
		/** The Selector can be used by the engine for doing collision
		detection. You can create a TriangleSelector with
		ISceneManager::createTriangleSelector() or
		ISceneManager::createOctTreeTriangleSelector and set it with
		ISceneNode::setTriangleSelector(). If a scene node got no triangle
		selector, but collision tests should be done with it, a triangle
		selector is created using the bounding box of the scene node.
		\return A pointer to the TriangleSelector or 0, if there
		is none. */
		virtual ITriangleSelector* getTriangleSelector() const
		{
			return TriangleSelector;
		}


		//! Sets the triangle selector of the scene node.
		/** The Selector can be used by the engine for doing collision
		detection. You can create a TriangleSelector with
		ISceneManager::createTriangleSelector() or
		ISceneManager::createOctTreeTriangleSelector(). Some nodes may
		create their own selector by default, so it would be good to
		check if there is already a selector in this node by calling
		ISceneNode::getTriangleSelector().
		\param selector New triangle selector for this scene node. */
		virtual void setTriangleSelector(ITriangleSelector* selector)
		{
			if (TriangleSelector)
				TriangleSelector->drop();

			TriangleSelector = selector;
			if (TriangleSelector)
				TriangleSelector->grab();
		}


		//! Updates the absolute position based on the relative and the parents position
		virtual void updateAbsolutePosition()
		{
			if (Parent)
			{
				AbsoluteTransformation =
					Parent->getAbsoluteTransformation() * getRelativeTransformation();
			}
			else
				AbsoluteTransformation = getRelativeTransformation();
		}


		//! Returns the parent of this scene node
		/** \return A pointer to the parent. */
		scene::ISceneNode* getParent() const
		{
			return Parent;
		}


		//! Returns type of the scene node
		/** \return The type of this node. */
		virtual ESCENE_NODE_TYPE getType() const
		{
			return ESNT_UNKNOWN;
		}


		//! Writes attributes of the scene node.
		/** Implement this to expose the attributes of your scene node
		for scripting languages, editors, debuggers or xml
		serialization purposes.
		\param out The attribute container to write into.
		\param options Additional options which might influence the
		serialization. */
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
		{
			if (!out)
				return;
			out->addString	("Name", Name.c_str());
			out->addInt	("Id", ID );

			out->addVector3d("Position", getPosition() );
			out->addVector3d("Rotation", getRotation() );
			out->addVector3d("Scale", getScale() );

			out->addBool	("Visible", IsVisible );
			out->addEnum	("AutomaticCulling", AutomaticCullingState, AutomaticCullingNames);
			out->addInt	("DebugDataVisible", DebugDataVisible );
			out->addBool	("IsDebugObject", IsDebugObject );
		}


		//! Reads attributes of the scene node.
		/** Implement this to set the attributes of your scene node for
		scripting languages, editors, debuggers or xml deserialization
		purposes.
		\param in The attribute container to read from.
		\param options Additional options which might influence the
		deserialization. */
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
		{
			if (!in)
				return;
			Name = in->getAttributeAsString("Name");
			ID = in->getAttributeAsInt("Id");

			setPosition(in->getAttributeAsVector3d("Position"));
			setRotation(in->getAttributeAsVector3d("Rotation"));
			setScale(in->getAttributeAsVector3d("Scale"));

			IsVisible = in->getAttributeAsBool("Visible");
			AutomaticCullingState = (scene::E_CULLING_TYPE) in->getAttributeAsEnumeration("AutomaticCulling",
					scene::AutomaticCullingNames);

			DebugDataVisible = in->getAttributeAsInt("DebugDataVisible");
			IsDebugObject = in->getAttributeAsBool("IsDebugObject");

			updateAbsolutePosition();
		}

		//! Creates a clone of this scene node and its children.
		/** \param newParent An optional new parent.
		\param newManager An optional new scene manager.
		\return The newly created clone of this node. */
		virtual ISceneNode* clone(ISceneNode* newParent=0, ISceneManager* newManager=0)
		{
			return 0; // to be implemented by derived classes
		}

	protected:

		//! A clone function for the ISceneNode members.
		/** This method can be used by clone() implementations of
		derived classes
		\param toCopyFrom The node from which the values are copied
		\param newManager The new scene manager. */
		void cloneMembers(ISceneNode* toCopyFrom, ISceneManager* newManager)
		{
			Name = toCopyFrom->Name;
			AbsoluteTransformation = toCopyFrom->AbsoluteTransformation;
			RelativeTranslation = toCopyFrom->RelativeTranslation;
			RelativeRotation = toCopyFrom->RelativeRotation;
			RelativeScale = toCopyFrom->RelativeScale;
			ID = toCopyFrom->ID;
			setTriangleSelector(toCopyFrom->TriangleSelector);
			AutomaticCullingState = toCopyFrom->AutomaticCullingState;
			DebugDataVisible = toCopyFrom->DebugDataVisible;
			IsVisible = toCopyFrom->IsVisible;
			IsDebugObject = toCopyFrom->IsDebugObject;

			if (newManager)
				SceneManager = newManager;
			else
				SceneManager = toCopyFrom->SceneManager;

			// clone children

			core::list<ISceneNode*>::Iterator it = toCopyFrom->Children.begin();
			for (; it != toCopyFrom->Children.end(); ++it)
				(*it)->clone(this, newManager);

			// clone animators

			core::list<ISceneNodeAnimator*>::Iterator ait = toCopyFrom->Animators.begin();
			for (; ait != toCopyFrom->Animators.end(); ++ait)
			{
				ISceneNodeAnimator* anim = (*ait)->createClone(this, SceneManager);
				if (anim)
				{
					addAnimator(anim);
					anim->drop();
				}
			}
		}

		//! Name of the scene node.
		core::stringc Name;

		//! Absolute transformation of the node.
		core::matrix4 AbsoluteTransformation;

		//! Relative translation of the scene node.
		core::vector3df RelativeTranslation;

		//! Relative rotation of the scene node.
		core::vector3df RelativeRotation;

		//! Relative scale of the scene node.
		core::vector3df RelativeScale;

		//! Pointer to the parent
		ISceneNode* Parent;

		//! List of all children of this node
		core::list<ISceneNode*> Children;

		//! List of all animator nodes
		core::list<ISceneNodeAnimator*> Animators;

		//! ID of the node.
		s32 ID;

		//! Pointer to the scene manager
		ISceneManager* SceneManager;

		//! Pointer to the triangle selector
		ITriangleSelector* TriangleSelector;

		//! Automatic culling state
		E_CULLING_TYPE AutomaticCullingState;

		//! Is the node visible?
		bool IsVisible;

		//! Flag if debug data should be drawn, such as Bounding Boxes.
		s32 DebugDataVisible;

		//! Is debug object?
		bool IsDebugObject;
	};

} // end namespace scene
} // end namespace irr

#endif

