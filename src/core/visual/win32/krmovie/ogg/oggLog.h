//===========================================================================
// Copyright (C) 2009 Cristian Adam
//
// Based upon Dr. Dobb's "Logging In C++" (September 05, 2007) article
// by Petru Marginean
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//- Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//- Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//
//- Neither the name of Cristian Adam nor the names of contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ORGANISATION OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//===========================================================================

#ifndef __LOG_H__
#define __LOG_H__

#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <MMSystem.h>

inline std::wstring NowTime();

enum LogLevel
{
    logNONE, // Used with ReportingLevel to suppress all logging at runtime.
    logERROR,
    logWARNING,
    logINFO,
    logDEBUG,
    logDEBUG1,
    logDEBUG2,
    logDEBUG3,
    logDEBUG4
};

template <typename T>
class LogT : public T
{
public:
    LogT();
    virtual ~LogT();

    std::wostringstream& Get(LogLevel level = logINFO);

public:
    static LogLevel& ReportingLevel();
    static std::wstring ToString(LogLevel level);
    static LogLevel FromString(const std::wstring& level);

protected:
    std::wostringstream os;

private:
    LogT(const LogT&);
    LogT& operator =(const LogT&);
};

template <typename T>
LogT<T>::LogT()
{
}

template <typename T>
std::wostringstream& LogT<T>::Get(LogLevel level)
{
    os << L"- " << NowTime();
    os << std::setfill(L'0') << std::hex;
    os << L" (0x" << std::setw(2) << ::GetCurrentThreadId() << L")";
    os << std::setfill(L' ') << std::dec;
    os << L" " << ToString(level) << L": ";
    os << std::wstring(level > logDEBUG ? level - logDEBUG : 0, L'\t');

    return os;
}

template <typename T>
LogT<T>::~LogT()
{
    os << L"\r\n";
    T::Output(os.str());
}

template <typename T>
LogLevel& LogT<T>::ReportingLevel()
{
    static LogLevel reportingLevel = logNONE;
    return reportingLevel;
}

template <typename T>
std::wstring LogT<T>::ToString(LogLevel level)
{
	static const wchar_t* const buffer[] =
    {
        L"ERROR", L"WARNING", L"INFO", L"DEBUG",
        L"DEBUG1", L"DEBUG2", L"DEBUG3", L"DEBUG4"
    };
    return buffer[level - 1];
}

template <typename T>
LogLevel LogT<T>::FromString(const std::wstring& level)
{
    typedef std::map<std::wstring, LogLevel> LogLevelNamesMap;
    static LogLevelNamesMap logLevels;

    if (logLevels.empty())
    {
        logLevels[L"NONE"]    = logNONE;
        logLevels[L"ERROR"]   = logERROR;
        logLevels[L"WARNING"] = logWARNING;
        logLevels[L"INFO"]    = logINFO;
        logLevels[L"DEBUG"]   = logDEBUG;
        logLevels[L"DEBUG1"]  = logDEBUG1;
        logLevels[L"DEBUG2"]  = logDEBUG2;
        logLevels[L"DEBUG3"]  = logDEBUG3;
        logLevels[L"DEBUG4"]  = logDEBUG4;
    }

    LogLevelNamesMap::iterator it = logLevels.find(level);
    if (it == logLevels.end())
    {
        LogT<T>().Get(logWARNING) << L"Unknown logging level '" << level
            << L"'. Using INFO level as default.";

        return logINFO;
    }

    return it->second;
}

//===========================================================================

#ifdef __ATLSTR_H__
inline std::wostream& operator << (std::wostream& wos, const CString& str)
{
    wos << static_cast<const wchar_t*>(str);
    return wos;
}
#endif

#ifdef __ATLCOMCLI_H__
inline std::wostream& operator << (std::wostream& wos, const CComBSTR& str)
{
    wos << static_cast<const wchar_t*>(str);
    return wos;
}

inline std::wostream& operator << (std::wostream& wos, const CComVariant& var)
{
    switch (var.vt)
    {
    case VT_I1:
        wos << var.cVal;
        break;
    case VT_UI1:
        wos << var.bVal;
        break;
    case VT_I2:
        wos << var.iVal;
        break;
    case VT_UI2:
        wos << var.uiVal;
        break;
    case VT_I4:
        wos << var.lVal;
        break;
    case VT_UI4:
        wos << var.ulVal;
        break;
    case VT_INT:
        wos << var.intVal;
        break;
    case VT_UINT:
        wos << var.uintVal;
        break;
    case VT_ERROR:
        wos << var.lVal;
        break;
#if (_WIN32_WINNT >= 0x0501) || defined(_ATL_SUPPORT_VT_I8)
    case VT_I8:
        wos << var.llVal;
        break;
    case VT_UI8:
        wos << var.ullVal;
        break;
#endif
    case VT_R4:
        wos << var.fltVal;
        break;
    case VT_R8:
    case VT_CY:
    case VT_DATE:
        wos << var.dblVal;
    case VT_BOOL:
        wos << std::boolalpha << (var.iVal == VARIANT_TRUE);
        break;
    case VT_BSTR:
        wos << var.bstrVal;
        break;
    }
    return wos;
}

#endif

typedef __int64 REFERENCE_TIME;

class ReferenceTime
{
    static const REFERENCE_TIME UNITS = 10000;

    REFERENCE_TIME m_time;

public:
    ReferenceTime() : m_time(0)
    {
    }

    ReferenceTime(REFERENCE_TIME time) : m_time(time)
    {
    }

    ReferenceTime(long milliseconds)
    {
        m_time = milliseconds * UNITS;
    }

    REFERENCE_TIME GetUnits() const
    {
        return m_time;
    }

    long Millisecs() const
    {
        return static_cast<long>(m_time / UNITS);
    }

    operator REFERENCE_TIME() const
    {
        return m_time;
    }

    ReferenceTime& operator = (const ReferenceTime& other)
    {
        m_time = other.m_time;
        return *this;
    }

    ReferenceTime& operator += (const ReferenceTime& other)
    {
        m_time += other.m_time;
        return *this;
    }

    ReferenceTime& operator -= (const ReferenceTime& other)
    {
        m_time -= other.m_time;
        return *this;
    }
};

inline std::wostream& operator << (std::wostream& wos, const ReferenceTime& time)
{
    long milliseconds = time.Millisecs();

    wos << std::setfill(L'0')
       << std::setw(2)
       << milliseconds / 1000 / 3600 << L":"
       << std::setw(2)
       << milliseconds / 1000 % 3600 / 60 << L":"
       << std::setw(2)
       << milliseconds / 1000 % 60 << L"."
       << std::setw(3)
       << milliseconds % 1000;

    return wos;
}

#ifdef __REFTIME__
inline std::wostream& operator << (std::wostream& wos, const CRefTime& time)
{
    wos << ReferenceTime(time);
    return wos;
}
#endif


inline std::wostream& operator << (std::wostream& wos, const IID& guid)
{
    wos << std::setfill(L'0') << L"{"
        << std::setw(8) << std::hex
        << guid.Data1 << L"-"
        << std::setw(4) << std::hex
        << guid.Data2 << L"-"
        << std::setw(4) << std::hex
        << guid.Data3 << L"-"
        << std::setw(2) << std::hex
        << guid.Data4[0]
        << std::setw(2) << std::hex
        << guid.Data4[1] << L"-"
        << std::setw(2) << std::hex
        << guid.Data4[2]
        << std::setw(2) << std::hex
        << guid.Data4[3]
        << std::setw(2) << std::hex
        << guid.Data4[4]
        << std::setw(2) << std::hex
        << guid.Data4[5]
        << std::setw(2) << std::hex
        << guid.Data4[6]
        << std::setw(2) << std::hex
        << guid.Data4[7] << L"}";

    return wos;
}

template<typename T>
struct ToStringImpl
{
    static std::wstring ToString(T t)
    {
        std::wostringstream wos;
        wos << t;
        return wos.str();
    }
};

template<typename T>
struct ToStringImpl<T*>
{
    static std::wstring ToString(T* p)
    {
        std::wostringstream wos;
        if (p)
        {
            wos << *p;
            return wos.str();
        }

        wos << "(null)";
        return wos.str();
    }
};

template<typename T>
inline std::wstring ToString(T t)
{
    return ToStringImpl<T>::ToString(t);
}

//===========================================================================

class Output2FILE
{
public:
    static HANDLE& Stream(const std::wstring& logFile = L"");
    static void Output(const std::wstring& msg);
};

inline HANDLE& Output2FILE::Stream(const std::wstring& logFile /*= L""*/)
{
#ifndef WINCE
    static HANDLE streamHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
#else
    static HANDLE streamHandle = INVALID_HANDLE_VALUE;
#endif

    if (!logFile.empty())
    {
        if (streamHandle != INVALID_HANDLE_VALUE
#ifndef WINCE
             && streamHandle != ::GetStdHandle(STD_OUTPUT_HANDLE)
#endif
           )
        {
            ::CloseHandle(streamHandle);
        }

        streamHandle = ::CreateFile(logFile.c_str(),
                            GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, 0);

        // Append to the existing file or set the UTF-8 BOM to a newly created file
        if (::GetLastError() ==  ERROR_ALREADY_EXISTS)
        {
            ::SetFilePointer(streamHandle, 0, 0, FILE_END);
        }
        else
        {
            std::vector<unsigned char> bom;
            bom.push_back(0xef);
            bom.push_back(0xbb);
            bom.push_back(0xbf);

            DWORD bytesWritten = 0;
            ::WriteFile(streamHandle, &*bom.begin(), static_cast<DWORD>(bom.size()), &bytesWritten, 0);
        }
    }

    return streamHandle;
}

inline void Output2FILE::Output(const std::wstring& msg)
{
    HANDLE streamHandle = Stream();
    
    if (streamHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    std::string utf8msg;

    // Transform wide chars to UTF-8 
    int chars = ::WideCharToMultiByte(CP_UTF8, 0, &*msg.begin(), static_cast<int>(msg.size()), 0, 0, 0, 0);
    utf8msg.resize(chars);

    ::WideCharToMultiByte(CP_UTF8, 0, &*msg.begin(), static_cast<int>(msg.size()), &*utf8msg.begin(), chars, 0, 0);

    DWORD bytesWritten;
    ::WriteFile(streamHandle, utf8msg.c_str(), static_cast<DWORD>(utf8msg.size()), &bytesWritten, 0);
}

typedef LogT<Output2FILE> Log;

#ifndef LOG_MAX_LEVEL
#define LOG_MAX_LEVEL logDEBUG4
#endif

#define LOG(level) \
    if (level > LOG_MAX_LEVEL) ; \
    else if (level > Log::ReportingLevel() || !Output2FILE::Stream()) ; \
    else Log().Get(level)

inline std::wstring NowTime()
{
    std::wstring time(9, L'0');

    if (::GetTimeFormat(LOCALE_USER_DEFAULT, 0, 0, L"HH':'mm':'ss",
                        &*time.begin(), static_cast<int>(time.size())) == 0)
    {
        return L"Error in NowTime()";
    }

    std::wostringstream os;
    static DWORD first = ::timeGetTime();

    os << time.c_str() << L"."
       << std::setfill(L'0') << std::setw(3)
       << (long)(::timeGetTime() - first) % 1000;

    return os.str();
}


#endif //__LOG_H__
