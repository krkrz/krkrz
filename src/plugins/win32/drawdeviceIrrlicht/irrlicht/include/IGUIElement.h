// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_GUI_ELEMENT_H_INCLUDED__
#define __I_GUI_ELEMENT_H_INCLUDED__

#include "IAttributeExchangingObject.h"
#include "irrList.h"
#include "rect.h"
#include "irrString.h"
#include "IEventReceiver.h"
#include "EGUIElementTypes.h"
#include "IAttributes.h"

namespace irr
{
namespace gui
{

class IGUIEnvironment;

enum EGUI_ALIGNMENT
{
	//! Aligned to parent's top or left side (default)
	EGUIA_UPPERLEFT=0,
	//! Aligned to parent's bottom or right side
	EGUIA_LOWERRIGHT,
	//! Aligned to the center of parent
	EGUIA_CENTER,
	//! Scaled within its parent
	EGUIA_SCALE
};

//! Names for alignments
const c8* const GUIAlignmentNames[] =
{
	"upperLeft",
	"lowerRight",
	"center",
	"scale",
	0
};

//! Base class of all GUI elements.
class IGUIElement : public virtual io::IAttributeExchangingObject, public IEventReceiver
{
public:

	//! Constructor
	IGUIElement(EGUI_ELEMENT_TYPE type, IGUIEnvironment* environment, IGUIElement* parent,
		s32 id, core::rect<s32> rectangle)
		: Parent(0), RelativeRect(rectangle), AbsoluteRect(rectangle),
		AbsoluteClippingRect(rectangle), DesiredRect(rectangle),
		MaxSize(0,0), MinSize(1,1), IsVisible(true), IsEnabled(true),
		IsSubElement(false), NoClip(false), ID(id), IsTabStop(false), TabOrder(-1), IsTabGroup(false),
		AlignLeft(EGUIA_UPPERLEFT), AlignRight(EGUIA_UPPERLEFT), AlignTop(EGUIA_UPPERLEFT), AlignBottom(EGUIA_UPPERLEFT),
		Environment(environment), Type(type)
	{
		#ifdef _DEBUG
		setDebugName("IGUIElement");
		#endif

		// if we were given a parent to attach to
		if (parent)
			parent->addChild(this);

		// if we succeeded in becoming a child
		if (Parent)
		{
			LastParentRect = Parent->getAbsolutePosition();
			AbsoluteRect += LastParentRect.UpperLeftCorner;
			AbsoluteClippingRect = AbsoluteRect;
			AbsoluteClippingRect.clipAgainst(Parent->AbsoluteClippingRect);
		}
	}


	//! Destructor
	virtual ~IGUIElement()
	{
		// delete all children
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			(*it)->Parent = 0;
			(*it)->drop();
		}
	}


	//! Returns parent of this element.
	IGUIElement* getParent() const
	{
		return Parent;
	}


	//! Returns the relative rectangle of this element.
	core::rect<s32> getRelativePosition() const
	{
		return RelativeRect;
	}


	//! Sets the relative rectangle of this element.
	void setRelativePosition(const core::rect<s32>& r)
	{
		if (Parent)
		{
			const core::rect<s32>& r2 = Parent->getAbsolutePosition();

			core::dimension2df d((f32)(r2.getSize().Width), (f32)(r2.getSize().Height));

			if (AlignLeft   == EGUIA_SCALE)
				ScaleRect.UpperLeftCorner.X = (f32)r.UpperLeftCorner.X / d.Width;
			if (AlignRight  == EGUIA_SCALE)
				ScaleRect.LowerRightCorner.X = (f32)r.LowerRightCorner.X / d.Width;
			if (AlignTop    == EGUIA_SCALE)
				ScaleRect.UpperLeftCorner.Y = (f32)r.UpperLeftCorner.Y / d.Height;
			if (AlignBottom == EGUIA_SCALE)
				ScaleRect.LowerRightCorner.Y = (f32)r.LowerRightCorner.Y / d.Height;
		}

		DesiredRect = r;
		updateAbsolutePosition();
	}


	//! Sets the relative rectangle of this element.
	void setRelativePosition(const core::rect<f32>& r)
	{
		if (!Parent)
			return;

		const core::dimension2di& d = Parent->getAbsolutePosition().getSize();

		DesiredRect = core::rect<s32>(
					core::floor32((f32)d.Width * r.UpperLeftCorner.X),
					core::floor32((f32)d.Height * r.UpperLeftCorner.Y),
					core::floor32((f32)d.Width * r.LowerRightCorner.X),
					core::floor32((f32)d.Height * r.LowerRightCorner.Y));

		ScaleRect = r;

		updateAbsolutePosition();
	}


	//! Returns the absolute rectangle of element.
	core::rect<s32> getAbsolutePosition() const
	{
		return AbsoluteRect;
	}


	//! Returns the visible area of the element.
	core::rect<s32> getAbsoluteClippingRect() const
	{
		return AbsoluteClippingRect;
	}


	//! Sets whether the element will ignore its parent's clipping rectangle
	void setNotClipped(bool noClip)
	{
		NoClip = noClip;
	}


	//! Gets whether the element will ignore its parent's clipping rectangle
	bool isNotClipped() const
	{
		return NoClip;
	}


	//! Sets the maximum size allowed for this element
	/** If set to 0,0, there is no maximum size */
	void setMaxSize(core::dimension2di size)
	{
		MaxSize = size;
		updateAbsolutePosition();
	}


	//! Sets the minimum size allowed for this element
	void setMinSize(core::dimension2di size)
	{
		MinSize = size;
		if (MinSize.Width < 1)
			MinSize.Width = 1;
		if (MinSize.Height < 1)
			MinSize.Height = 1;
		updateAbsolutePosition();
	}


	void setAlignment(EGUI_ALIGNMENT left, EGUI_ALIGNMENT right, EGUI_ALIGNMENT top, EGUI_ALIGNMENT bottom)
	{
		AlignLeft = left;
		AlignRight = right;
		AlignTop = top;
		AlignBottom = bottom;

		if (Parent)
		{
			core::rect<s32> r(Parent->getAbsolutePosition());

			core::dimension2df d((f32)r.getSize().Width, (f32)r.getSize().Height);

			if (AlignLeft   == EGUIA_SCALE)
				ScaleRect.UpperLeftCorner.X = (f32)DesiredRect.UpperLeftCorner.X / d.Width;
			if (AlignRight  == EGUIA_SCALE)
				ScaleRect.LowerRightCorner.X = (f32)DesiredRect.LowerRightCorner.X / d.Width;
			if (AlignTop    == EGUIA_SCALE)
				ScaleRect.UpperLeftCorner.Y = (f32)DesiredRect.UpperLeftCorner.Y / d.Height;
			if (AlignBottom == EGUIA_SCALE)
				ScaleRect.LowerRightCorner.Y = (f32)DesiredRect.LowerRightCorner.Y / d.Height;
		}
	}


	//! Updates the absolute position.
	virtual void updateAbsolutePosition()
	{
		core::rect<s32> parentAbsolute(0,0,0,0);
		core::rect<s32> parentAbsoluteClip;
		s32 diffx, diffy;
		f32 fw=0.f, fh=0.f;

		if (Parent)
		{
			parentAbsolute = Parent->AbsoluteRect;

			if (NoClip)
			{
				IGUIElement* p=this;
				while (p && p->Parent)
					p = p->Parent;
				parentAbsoluteClip = p->AbsoluteClippingRect;
			}
			else
				parentAbsoluteClip = Parent->AbsoluteClippingRect;
		}


		diffx = parentAbsolute.getWidth() - LastParentRect.getWidth();
		diffy = parentAbsolute.getHeight() - LastParentRect.getHeight();

		if (AlignLeft == EGUIA_SCALE || AlignRight == EGUIA_SCALE)
			fw = (f32)parentAbsolute.getWidth();

		if (AlignTop == EGUIA_SCALE || AlignBottom == EGUIA_SCALE)
			fh = (f32)parentAbsolute.getHeight();


		switch (AlignLeft)
		{
			case EGUIA_UPPERLEFT:
				break;
			case EGUIA_LOWERRIGHT:
				DesiredRect.UpperLeftCorner.X += diffx;
				break;
			case EGUIA_CENTER:
				DesiredRect.UpperLeftCorner.X += diffx/2;
				break;
			case EGUIA_SCALE:
				DesiredRect.UpperLeftCorner.X = (s32)(ScaleRect.UpperLeftCorner.X * fw);
				break;
		}

		switch (AlignRight)
		{
			case EGUIA_UPPERLEFT:
				break;
			case EGUIA_LOWERRIGHT:
				DesiredRect.LowerRightCorner.X += diffx;
				break;
			case EGUIA_CENTER:
				DesiredRect.LowerRightCorner.X += diffx/2;
				break;
			case EGUIA_SCALE:
				DesiredRect.LowerRightCorner.X = (s32)(ScaleRect.LowerRightCorner.X * fw);
				break;
		}

		switch (AlignTop)
		{
			case EGUIA_UPPERLEFT:
				break;
			case EGUIA_LOWERRIGHT:
				DesiredRect.UpperLeftCorner.Y += diffy;
				break;
			case EGUIA_CENTER:
				DesiredRect.UpperLeftCorner.Y += diffy/2;
				break;
			case EGUIA_SCALE:
				DesiredRect.UpperLeftCorner.Y = (s32)(ScaleRect.UpperLeftCorner.Y * fh);
				break;
		}

		switch (AlignBottom)
		{
			case EGUIA_UPPERLEFT:
				break;
			case EGUIA_LOWERRIGHT:
				DesiredRect.LowerRightCorner.Y += diffy;
				break;
			case EGUIA_CENTER:
				DesiredRect.LowerRightCorner.Y += diffy/2;
				break;
			case EGUIA_SCALE:
				DesiredRect.LowerRightCorner.Y = (s32)(ScaleRect.LowerRightCorner.Y * fh);
				break;
		}

		RelativeRect = DesiredRect;

		s32 w = RelativeRect.getWidth();
		s32 h = RelativeRect.getHeight();

		// make sure the desired rectangle is allowed
		if (w < MinSize.Width)
			RelativeRect.LowerRightCorner.X = RelativeRect.UpperLeftCorner.X + MinSize.Width;
		if (h < MinSize.Height)
			RelativeRect.LowerRightCorner.Y = RelativeRect.UpperLeftCorner.Y + MinSize.Height;
		if (MaxSize.Width && w > MaxSize.Width)
			RelativeRect.LowerRightCorner.X = RelativeRect.UpperLeftCorner.X + MaxSize.Width;
		if (MaxSize.Height && h > MaxSize.Height)
			RelativeRect.LowerRightCorner.Y = RelativeRect.UpperLeftCorner.Y + MaxSize.Height;

		RelativeRect.repair();

		AbsoluteRect = RelativeRect + parentAbsolute.UpperLeftCorner;

		if (!Parent)
			parentAbsoluteClip = AbsoluteRect;

		AbsoluteClippingRect = AbsoluteRect;
		AbsoluteClippingRect.clipAgainst(parentAbsoluteClip);

		LastParentRect = parentAbsolute;

		// update all children
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			(*it)->updateAbsolutePosition();
		}
	}


	//! Returns the child element, which is at the position of the point.
	IGUIElement* getElementFromPoint(const core::position2d<s32>& point)
	{
		IGUIElement* target = 0;

		// we have to search from back to front, because later children
		// might be drawn over the top of earlier ones.

		core::list<IGUIElement*>::Iterator it = Children.getLast();

		if (IsVisible)
			while(it != Children.end())
			{
				target = (*it)->getElementFromPoint(point);
				if (target)
					return target;

				--it;
			}

		if (IsVisible && isPointInside(point))
			target = this;

		return target;
	}


	//! Returns true if a point is within this element.
	//! Elements with a shape other than a rectangle will override this method
	virtual bool isPointInside(const core::position2d<s32>& point) const
	{
		return AbsoluteClippingRect.isPointInside(point);
	}


	//! Adds a GUI element as new child of this element.
	virtual void addChild(IGUIElement* child)
	{
		if (child)
		{
			child->grab();
			child->remove(); // remove from old parent
			child->LastParentRect = getAbsolutePosition();
			child->Parent = this;
			Children.push_back(child);
		}
	}


	//! Removes a child.
	virtual void removeChild(IGUIElement* child)
	{
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
			if ((*it) == child)
			{
				(*it)->Parent = 0;
				(*it)->drop();
				Children.erase(it);
				return;
			}
	}


	//! Removes this element from its parent.
	virtual void remove()
	{
		if (Parent)
			Parent->removeChild(this);
	}


	//! Draws the element and its children.
	virtual void draw()
	{
		if (!IsVisible)
			return;

		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
			(*it)->draw();
	}


	//! animate the element and its children.
	virtual void OnPostRender(u32 timeMs)
	{
		if (!IsVisible)
			return;

		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
			(*it)->OnPostRender( timeMs );
	}


	//! Moves this element.
	virtual void move(core::position2d<s32> absoluteMovement)
	{
		setRelativePosition(DesiredRect + absoluteMovement);
	}


	//! Returns true if element is visible.
	virtual bool isVisible() const
	{
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return IsVisible;
	}


	//! Sets the visible state of this element.
	virtual void setVisible(bool visible)
	{
		IsVisible = visible;
	}


	//! Returns true if this element was created as part of its parent control
	virtual bool isSubElement() const
	{
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return IsSubElement;
	}


	//! Sets whether this control was created as part of its parent,
	//! for example when a scrollbar is part of a listbox.
	//! SubElements are not saved to disk when calling guiEnvironment->saveGUI()
	virtual void setSubElement(bool subElement)
	{
		IsSubElement = subElement;
	}


	//! If set to true, the focus will visit this element when using
	//! the tab key to cycle through elements.
	//! If this element is a tab group (see isTabGroup/setTabGroup) then
	//! ctrl+tab will be used instead.
	void setTabStop(bool enable)
	{
		IsTabStop = enable;
	}


	//! Returns true if this element can be focused by navigating with the tab key
	bool isTabStop() const
	{
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return IsTabStop;
	}


	//! Sets the priority of focus when using the tab key to navigate between a group
	//! of elements. See setTabGroup, isTabGroup and getTabGroup for information on tab groups.
	//! Elements with a lower number are focused first
	void setTabOrder(s32 index)
	{
		// negative = autonumber
		if (index < 0)
		{
			TabOrder = 0;
			IGUIElement *el = getTabGroup();
			while (IsTabGroup && el && el->Parent)
				el = el->Parent;

			IGUIElement *first=0, *closest=0;
			if (el)
			{
				// find the highest element number
				el->getNextElement(-1, true, IsTabGroup, first, closest, true);
				if (first)
				{
					TabOrder = first->getTabOrder() + 1;
				}
			}

		}
		else
			TabOrder = index;
	}


	//! Returns the number in the tab order sequence
	s32 getTabOrder() const
	{
		return TabOrder;
	}


	//! Sets whether this element is a container for a group of elements which
	//! can be navigated using the tab key. For example, windows are tab groups.
	//! Groups can be navigated using ctrl+tab, providing isTabStop is true.
	void setTabGroup(bool isGroup)
	{
		IsTabGroup = isGroup;
	}


	//! Returns true if this element is a tab group.
	bool isTabGroup() const
	{
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return IsTabGroup;
	}


	//! Returns the container element which holds all elements in this element's
	//! tab group.
	IGUIElement* getTabGroup()
	{
		IGUIElement *ret=this;

		while (ret && !ret->isTabGroup())
			ret = ret->getParent();

		return ret;
	}


	//! Returns true if element is enabled.
	virtual bool isEnabled() const
	{
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return IsEnabled;
	}


	//! Sets the enabled state of this element.
	virtual void setEnabled(bool enabled)
	{
		IsEnabled = enabled;
	}


	//! Sets the new caption of this element.
	virtual void setText(const wchar_t* text)
	{
		Text = text;
	}


	//! Returns caption of this element.
	virtual const wchar_t* getText() const
	{
		return Text.c_str();
	}


	//! Sets the new caption of this element.
	virtual void setToolTipText(const wchar_t* text)
	{
		ToolTipText = text;
	}


	//! Returns caption of this element.
	virtual const core::stringw& getToolTipText() const
	{
		return ToolTipText;
	}


	//! Returns id. Can be used to identify the element.
	virtual s32 getID() const
	{
		return ID;
	}


	//! Sets the id of this element
	virtual void setID(s32 id)
	{
		ID = id;
	}


	//! Called if an event happened.
	virtual bool OnEvent(const SEvent& event)
	{
		return Parent ? Parent->OnEvent(event) : false;
	}


	//! Brings a child to front
	/** \return Returns true if successful, false if not. */
	virtual bool bringToFront(IGUIElement* element)
	{
		core::list<IGUIElement*>::Iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			if (element == (*it))
			{
				Children.erase(it);
				Children.push_back(element);
				return true;
			}
		}

		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return false;
	}


	//! Returns list with children of this element
	virtual const core::list<IGUIElement*>& getChildren() const
	{
		return Children;
	}


	//! Finds the first element with the given id.
	/** \param id: Id to search for.
	\param searchchildren: Set this to true, if also children of this
	element may contain the element with the searched id and they
	should be searched too.
	\return Returns the first element with the given id. If no element
	with this id was found, 0 is returned. */
	virtual IGUIElement* getElementFromId(s32 id, bool searchchildren=false) const
	{
		IGUIElement* e = 0;

		core::list<IGUIElement*>::ConstIterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			if ((*it)->getID() == id)
				return (*it);

			if (searchchildren)
				e = (*it)->getElementFromId(id, true);

			if (e)
				return e;
		}

		return e;
	}


	//! returns true if the given element is a child of this one.
	//! \param child: The child element to check
	bool isMyChild(IGUIElement* child) const
	{
		if (!child)
			return false;
		do
		{
			if (child->Parent)
				child = child->Parent;

		} while (child->Parent && child != this);

		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return child == this;
	}


	//! searches elements to find the closest next element to tab to
	//! \param startOrder: The TabOrder of the current element, -1 if none
	//! \param reverse: true if searching for a lower number
	//! \param group: true if searching for a higher one
	//! \param first: element with the highest/lowest known tab order depending on search direction
	//! \param closest: the closest match, depending on tab order and direction
	//! \param includeInvisible: includes invisible elements in the search (default=false)
	//! \return true if successfully found an element, false to continue searching/fail
	bool getNextElement(s32 startOrder, bool reverse, bool group,
		IGUIElement*& first, IGUIElement*& closest, bool includeInvisible=false) const
	{
		// we'll stop searching if we find this number
		s32 wanted = startOrder + ( reverse ? -1 : 1 );
		if (wanted==-2)
			wanted = 1073741824; // maximum s32

		core::list<IGUIElement*>::ConstIterator it = Children.begin();

		s32 closestOrder, currentOrder;

		while(it != Children.end())
		{
			// ignore invisible elements and their children
			if ( ( (*it)->isVisible() || includeInvisible ) &&
				(group == true || (*it)->isTabGroup() == false) )
			{
				// only check tab stops and those with the same group status
				if ((*it)->isTabStop() && ((*it)->isTabGroup() == group))
				{
					currentOrder = (*it)->getTabOrder();

					// is this what we're looking for?
					if (currentOrder == wanted)
					{
						closest = *it;
						return true;
					}

					// is it closer than the current closest?
					if (closest)
					{
						closestOrder = closest->getTabOrder();
						if ( ( reverse && currentOrder > closestOrder && currentOrder < startOrder)
							||(!reverse && currentOrder < closestOrder && currentOrder > startOrder))
						{
							closest = *it;
						}
					}
					else
					if ( (reverse && currentOrder < startOrder) || (!reverse && currentOrder > startOrder) )
					{
						closest = *it;
					}

					// is it before the current first?
					if (first)
					{
						closestOrder = first->getTabOrder();

						if ( (reverse && closestOrder < currentOrder) || (!reverse && closestOrder > currentOrder) )
						{
							first = *it;
						}
					}
					else
					{
						first = *it;
					}
				}
				// search within children
				if ((*it)->getNextElement(startOrder, reverse, group, first, closest))
				{
					_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
					return true;
				}
			}
			++it;
		}
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return false;
	}


	//! Returns the type of the gui element.
	/** This is needed for the .NET wrapper but will be used
	later for serializing and deserializing.
	If you wrote your own GUIElements, you need to set the type for your element as first parameter
	in the constructor of IGUIElement. For own (=unknown) elements, simply use EGUIET_ELEMENT as type */
	EGUI_ELEMENT_TYPE getType() const
	{
		return Type;
	}


	//! Returns the type name of the gui element.
	/** This is needed serializing elements. For serializing your own elements, override this function
	and return your own type name which is created by your IGUIElementFactory */
	virtual const c8* getTypeName() const
	{
		return GUIElementTypeNames[Type];
	}


	//! Writes attributes of the scene node.
	//! Implement this to expose the attributes of your scene node for
	//! scripting languages, editors, debuggers or xml serialization purposes.
	virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
	{
		out->addInt("Id", ID );
		out->addString("Caption", getText());
		out->addRect("Rect", DesiredRect);
		out->addPosition2d("MinSize", core::position2di(MinSize.Width, MinSize.Height));
		out->addPosition2d("MaxSize", core::position2di(MaxSize.Width, MaxSize.Height));
		out->addBool("NoClip", NoClip);
		out->addEnum("LeftAlign", AlignLeft, GUIAlignmentNames);
		out->addEnum("RightAlign", AlignRight, GUIAlignmentNames);
		out->addEnum("TopAlign", AlignTop, GUIAlignmentNames);
		out->addEnum("BottomAlign", AlignBottom, GUIAlignmentNames);
		out->addBool("Visible", IsVisible);
		out->addBool("Enabled", IsEnabled);
		out->addBool("TabStop", IsTabStop);
		out->addBool("TabGroup", IsTabGroup);
		out->addInt("TabOrder", TabOrder);
	}


	//! Reads attributes of the scene node.
	//! Implement this to set the attributes of your scene node for
	//! scripting languages, editors, debuggers or xml deserialization purposes.
	virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
	{
		setID(in->getAttributeAsInt("Id"));
		setText(in->getAttributeAsStringW("Caption").c_str());
		setVisible(in->getAttributeAsBool("Visible"));
		setEnabled(in->getAttributeAsBool("Enabled"));
		IsTabStop = in->getAttributeAsBool("TabStop");
		IsTabGroup = in->getAttributeAsBool("TabGroup");
		TabOrder = in->getAttributeAsInt("TabOrder");

		core::position2di p = in->getAttributeAsPosition2d("MaxSize");
		setMaxSize(core::dimension2di(p.X,p.Y));

		p = in->getAttributeAsPosition2d("MinSize");
		setMinSize(core::dimension2di(p.X,p.Y));

		setNotClipped(in->getAttributeAsBool("NoClip"));
		setAlignment((EGUI_ALIGNMENT) in->getAttributeAsEnumeration("LeftAlign", GUIAlignmentNames),
			(EGUI_ALIGNMENT)in->getAttributeAsEnumeration("RightAlign", GUIAlignmentNames),
			(EGUI_ALIGNMENT)in->getAttributeAsEnumeration("TopAlign", GUIAlignmentNames),
			(EGUI_ALIGNMENT)in->getAttributeAsEnumeration("BottomAlign", GUIAlignmentNames));

		setRelativePosition(in->getAttributeAsRect("Rect"));
	}

protected:

	//! List of all children of this element
	core::list<IGUIElement*> Children;

	//! Pointer to the parent
	IGUIElement* Parent;

	//! relative rect of element
	core::rect<s32> RelativeRect;

	//! absolute rect of element
	core::rect<s32> AbsoluteRect;

	//! absolute clipping rect of element
	core::rect<s32> AbsoluteClippingRect;

	//! the rectangle the element would prefer to be,
	//! if it was not constrained by parent or max/min size
	core::rect<s32> DesiredRect;

	//! for calculating the difference when resizing parent
	core::rect<s32> LastParentRect;

	//! relative scale of the element inside its parent
	core::rect<f32> ScaleRect;

	//! maximum and minimum size of the element
	core::dimension2di MaxSize, MinSize;

	//! is visible?
	bool IsVisible;

	//! is enabled?
	bool IsEnabled;

	//! is a part of a larger whole and should not be serialized?
	bool IsSubElement;

	//! does this element ignore its parent's clipping rectangle?
	bool NoClip;

	//! caption
	core::stringw Text;

	//! tooltip
	core::stringw ToolTipText;

	//! id
	s32 ID;

	//! tab stop like in windows
	bool IsTabStop;

	//! tab order
	s32 TabOrder;

	//! tab groups are containers like windows, use ctrl+tab to navigate
	bool IsTabGroup;

	//! tells the element how to act when its parent is resized
	EGUI_ALIGNMENT AlignLeft, AlignRight, AlignTop, AlignBottom;

	//! GUI Environment
	IGUIEnvironment* Environment;

	//! type of element
	EGUI_ELEMENT_TYPE Type;
};


} // end namespace gui
} // end namespace irr

#endif

