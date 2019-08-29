#pragma once
/*
The MIT License (MIT)

Copyright (c) Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

Portions of this repo are provided under the SIL Open Font License.
See the LICENSE file in individual samples for additional details.

*/


    //////////////////////////////////////////////////////////////////////////
    //  AsyncCallback [template]
    //
    //  Description: 
    //  Helper class that routes IMFAsyncCallback::Invoke calls to a class
    //  method on the parent class.
    //
    //  Usage:
    //  Add this class as a member variable. In the parent class constructor,
    //  initialize the AsyncCallback class like this:
    //      m_cb(this, &CYourClass::OnInvoke)
    //  where
    //      m_cb       = AsyncCallback object
    //      CYourClass = parent class
    //      OnInvoke   = Method in the parent class to receive Invoke calls.
    //
    //  The parent's OnInvoke method (you can name it anything you like) must
    //  have a signature that matches the InvokeFn typedef below.
    //////////////////////////////////////////////////////////////////////////

    // T: Type of the parent object
    template<class T>
    class AsyncCallback : public IMFAsyncCallback
    {
    public: 
        typedef HRESULT (T::*InvokeFn)(IMFAsyncResult *pAsyncResult);

        AsyncCallback(T *pParent, InvokeFn fn) : m_pParent(pParent), m_pInvokeFn(fn)
        {
        }

        // IUnknown
        STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
        {
            if (!ppv)
            {
                return E_POINTER;
            }
            if (iid == __uuidof(IUnknown))
            {
                *ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
            }
            else if (iid == __uuidof(IMFAsyncCallback))
            {
                *ppv = static_cast<IMFAsyncCallback*>(this);
            }
            else
            {
                *ppv = NULL;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }
        STDMETHODIMP_(ULONG) AddRef() { 
            // Delegate to parent class.
            return m_pParent->AddRef(); 
        }
        STDMETHODIMP_(ULONG) Release() { 
            // Delegate to parent class.
            return m_pParent->Release(); 
        }


        // IMFAsyncCallback methods
        STDMETHODIMP GetParameters(DWORD*, DWORD*)
        {
            // Implementation of this method is optional.
            return E_NOTIMPL;
        }

        STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult)
        {
            return (m_pParent->*m_pInvokeFn)(pAsyncResult);
        }

        T *m_pParent;
        InvokeFn m_pInvokeFn;
    };

