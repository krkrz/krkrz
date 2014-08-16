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

#ifndef XMLSETTINGS_H
#define XMLSETTINGS_H

#include <MsXml.h>
#include <string>

class XmlSettings
{
    CComPtr<IXMLDOMDocument> m_xmlDoc;
    std::wstring m_moduleName;

public:

    XmlSettings()
    {
        m_xmlDoc.CoCreateInstance(CLSID_DOMDocument);
    }

    virtual ~XmlSettings()
    {
    }

    bool Load(const std::wstring& xmlFileName, const std::wstring& moduleName)
    {
        using std::wstringstream;
        using std::wstring;

        m_moduleName = moduleName;
        
        if (!m_xmlDoc)
        {
            return false;
        }

        VARIANT_BOOL isSuccessful = VARIANT_FALSE;
        HRESULT hr = m_xmlDoc->load(CComVariant(xmlFileName.c_str()), &isSuccessful);

        return SUCCEEDED(hr) && isSuccessful == VARIANT_TRUE;
    }

    std::wstring GetAttributeValue(const std::wstring& keyName, const std::wstring& attributeName)
    {
        std::wstring result;

        std::wstringstream queryString;
        queryString << L"/Configuration/Module[@Name=\"" << m_moduleName << L"\"]/" << keyName;

        CComPtr<IXMLDOMNode> node;
        HRESULT hr = m_xmlDoc->selectSingleNode(CComBSTR(queryString.str().c_str()), &node);
        if (FAILED(hr) || !node)
        {
            return result;
        }

        CComPtr<IXMLDOMNamedNodeMap> attributesMap;
        hr = node->get_attributes(&attributesMap);

        if (FAILED(hr))
        {
            return result;
        }

        CComPtr<IXMLDOMNode> levelAttribute;
        hr = attributesMap->getNamedItem(CComBSTR(attributeName.c_str()), &levelAttribute);

        if (FAILED(hr))
        {
            return result;
        }

        CComVariant levelValue;
        hr = levelAttribute->get_nodeValue(&levelValue);

        if (FAILED(hr))
        {
            return result;
        }

        result = static_cast<wchar_t*>(CComBSTR(levelValue.bstrVal));
        return result;
    }
};

#endif // XMLSETTINGS_H
