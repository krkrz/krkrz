// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_OS_OPERATOR_H_INCLUDED__
#define __I_OS_OPERATOR_H_INCLUDED__

#include "IReferenceCounted.h"

namespace irr
{

//! The Operating system operator provides operation system specific methods and informations.
class IOSOperator : public virtual IReferenceCounted
{
public:

	//! Destructor
	virtual ~IOSOperator() {}

	//! Get the current operation system version as string.
	virtual const wchar_t* getOperationSystemVersion() const = 0;

	//! Copies text to the clipboard
	virtual void copyToClipboard(const c8* text) const = 0;

	//! Get text from the clipboard
	/** \return Returns 0 if no string is in there. */
	virtual c8* getTextFromClipboard() const = 0;

	//! Get the processor speed in megahertz
	/** \param MHz The integer variable to store the speed in.
	\return True if successful, false if not */
	virtual bool getProcessorSpeedMHz(u32* MHz) const = 0;

	//! Get the total and available system RAM
	/** \param Total: will contain the total system memory
	\param Avail: will contain the available memory
	\return True if successful, false if not */
	virtual bool getSystemMemory(u32* Total, u32* Avail) const = 0;

};

} // end namespace

#endif

