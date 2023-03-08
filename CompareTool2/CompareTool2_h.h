

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for CompareTool2.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __CompareTool2_h_h__
#define __CompareTool2_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICompareTool2_FWD_DEFINED__
#define __ICompareTool2_FWD_DEFINED__
typedef interface ICompareTool2 ICompareTool2;

#endif 	/* __ICompareTool2_FWD_DEFINED__ */


#ifndef __CompareTool2_FWD_DEFINED__
#define __CompareTool2_FWD_DEFINED__

#ifdef __cplusplus
typedef class CompareTool2 CompareTool2;
#else
typedef struct CompareTool2 CompareTool2;
#endif /* __cplusplus */

#endif 	/* __CompareTool2_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __CompareTool2_LIBRARY_DEFINED__
#define __CompareTool2_LIBRARY_DEFINED__

/* library CompareTool2 */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_CompareTool2;

#ifndef __ICompareTool2_DISPINTERFACE_DEFINED__
#define __ICompareTool2_DISPINTERFACE_DEFINED__

/* dispinterface ICompareTool2 */
/* [uuid] */ 


EXTERN_C const IID DIID_ICompareTool2;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("7016d84c-9b4c-43a7-9e01-98513f02b17e")
    ICompareTool2 : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ICompareTool2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICompareTool2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICompareTool2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICompareTool2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICompareTool2 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICompareTool2 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICompareTool2 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICompareTool2 * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ICompareTool2Vtbl;

    interface ICompareTool2
    {
        CONST_VTBL struct ICompareTool2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICompareTool2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICompareTool2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICompareTool2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICompareTool2_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICompareTool2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICompareTool2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICompareTool2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ICompareTool2_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CompareTool2;

#ifdef __cplusplus

class DECLSPEC_UUID("b5f38c0c-b559-4b79-af02-5657a53350b3")
CompareTool2;
#endif
#endif /* __CompareTool2_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


