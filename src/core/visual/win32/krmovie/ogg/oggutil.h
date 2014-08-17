//===========================================================================
//Copyright (C) 2009 Cristian Adam
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

#ifndef UTIL_H
#define UTIL_H

#define CHECK_HR(expr) CHECK_HR_CORE(expr, __FUNCTION__)

#define CHECK_HR_CORE(expr, func) \
    do { \
    HRESULT hr = expr; \
    if (FAILED(hr)) \
        { \
            LOG(logERROR) << "Expression " << #expr << " in function " << func \
                << " has failed, error code: 0x" << std::hex << hr; \
            throw CAtlException(hr); \
        } \
    } while (false);

#include <shlobj.h>
#include "XmlSettings.h"

namespace util
{
    inline std::wstring GetModuleName(HANDLE hModule)
    {
        std::wstring moduleFileName;
        moduleFileName.resize(MAX_PATH);

        int chars = ::GetModuleFileName(static_cast<HMODULE>(hModule), 
                        &*moduleFileName.begin(), static_cast<int>(moduleFileName.size()));

        moduleFileName.resize(chars);

        size_t lastBackslash = moduleFileName.rfind(L'\\') + 1;
        size_t lastDot = moduleFileName.rfind(L'.');

        return moduleFileName.substr(lastBackslash, lastDot - lastBackslash);
    }

    inline std::wstring GetConfigurationPath()
    {
        std::wstring configLocation;
        configLocation.resize(MAX_PATH);
        
#ifndef WINCE
        ::SHGetSpecialFolderPath(0, &*configLocation.begin(), CSIDL_COMMON_APPDATA, false);
#else
        ::SHGetSpecialFolderPath(0, &*configLocation.begin(), CSIDL_APPDATA, false);
#endif
        
        configLocation.resize(wcslen(configLocation.c_str()));
        configLocation += L"\\Xiph.Org\\Open Codecs";

        return configLocation;
    }

    struct ComInitializer
    {
        ComInitializer()
        {
            ::CoInitializeEx(0, COINIT_APARTMENTTHREADED);
        }
        ~ComInitializer()
        {
            ::CoUninitialize();
        }
    };

    inline HMODULE& GetHModule()
    {
        static HMODULE module = 0;
        return module;
    }

    inline void ConfigureLogSettings() 
    {
        std::wstring moduleName = util::GetModuleName(util::GetHModule());
        std::wstring configurationPath = util::GetConfigurationPath();

        std::wstring xmlConfigurationFile = configurationPath;
        xmlConfigurationFile+= L"\\settings.xml";

        std::wstring levelString;

        XmlSettings settings;
        if (settings.Load(xmlConfigurationFile, moduleName))
        {
            levelString = settings.GetAttributeValue(L"Log", L"Level");
        }

        if (!levelString.empty())
        {
            Log::ReportingLevel() = Log::FromString(levelString);

            if (Log::ReportingLevel() != logNONE)
            {
                std::wstring logFileName = configurationPath;
                logFileName += L"\\";
                logFileName += moduleName + L".log";

                Log::Stream(logFileName);
            }
        }
    }

    struct Deleter
    {	
        template<typename T>
        void operator()(const T* arg) const
        {	
            delete arg;
        }
    };

}

#endif // UTIL_H