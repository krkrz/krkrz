// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_LOGGER_H_INCLUDED__
#define __I_LOGGER_H_INCLUDED__

#include "IReferenceCounted.h"

namespace irr
{

//! Possible log levels.
enum ELOG_LEVEL
{
	//! High log level, warnings, errors and important information texts are printed out.
	ELL_INFORMATION = 0,

	//! Default log level, warnings and errors are printed out
	ELL_WARNING,

	//! Low log level, only errors are printed into the log
	ELL_ERROR,

	//! Nothing is printed out to the log
	ELL_NONE
};


//! Interface for logging messages, warnings and errors
class ILogger : public virtual IReferenceCounted
{
public:

	//! Destructor
	virtual ~ILogger() {}

	//! Returns the current set log level.
	virtual ELOG_LEVEL getLogLevel() const = 0;

	//! Sets a new log level.
	/** With this value, texts which are sent to the logger are filtered
	out. For example setting this value to ELL_WARNING, only warnings and
	errors are printed out. Setting it to ELL_INFORMATION, which is the
	default setting, warnings, errors and informational texts are printed
	out.
	\param ll: new log level filter value. */
	virtual void setLogLevel(ELOG_LEVEL ll) = 0;

	//! Prints out a text into the log
	/** \param text: Text to print out.
	\param ll: Log level of the text. If the text is an error, set
	it to ELL_ERROR, if it is warning set it to ELL_WARNING, and if it
	is just an informational text, set it to ELL_INFORMATION. Texts are
	filtered with these levels. If you want to be a text displayed,
	independent on what level filter is set, use ELL_NONE. */
	virtual void log(const c8* text, ELOG_LEVEL ll=ELL_INFORMATION) = 0;

	//! Prints out a text into the log
	/** \param text: Text to print out.
	\param hint: Additional info. This string is added after a " :" to the
	string.
	\param ll: Log level of the text. If the text is an error, set
	it to ELL_ERROR, if it is warning set it to ELL_WARNING, and if it
	is just an informational text, set it to ELL_INFORMATION. Texts are
	filtered with these levels. If you want to be a text displayed,
	independent on what level filter is set, use ELL_NONE. */
	virtual void log(const c8* text, const c8* hint, ELOG_LEVEL ll=ELL_INFORMATION) = 0;

	//! Prints out a text into the log
	/** \param text: Text to print out.
	\param hint: Additional info. This string is added after a " :" to the
	string.
	\param ll: Log level of the text. If the text is an error, set
	it to ELL_ERROR, if it is warning set it to ELL_WARNING, and if it
	is just an informational text, set it to ELL_INFORMATION. Texts are
	filtered with these levels. If you want to be a text displayed,
	independent on what level filter is set, use ELL_NONE. */
	virtual void log(const wchar_t* text, const wchar_t* hint, ELOG_LEVEL ll=ELL_INFORMATION) = 0;

	//! Prints out a text into the log
	/** \param text: Text to print out.
	\param ll: Log level of the text. If the text is an error, set
	it to ELL_ERROR, if it is warning set it to ELL_WARNING, and if it
	is just an informational text, set it to ELL_INFORMATION. Texts are
	filtered with these levels. If you want to be a text displayed,
	independent on what level filter is set, use ELL_NONE. */
	virtual void log(const wchar_t* text, ELOG_LEVEL ll=ELL_INFORMATION) = 0;
};

} // end namespace

#endif

