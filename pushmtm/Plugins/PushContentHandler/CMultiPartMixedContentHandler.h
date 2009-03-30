/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Declaration of CMultiPartMixedContentHandler.
*
*/



#ifndef __CMULTIPARTMIXEDCONTENTHANDLER_H__
#define __CMULTIPARTMIXEDCONTENTHANDLER_H__


// INCLUDE FILES

#include "PushMtmDef.hrh"
#include "PushContentHandlerDef.hrh"
#include <CContentHandlerBase.h>
#include <E32Base.h>

// CONSTANTS

const TUid KUidPushMultiPartMixedContentHandler	= { 
           EUidPushMultiPartMixedContentHandler };
_LIT(KMultiPartMixedContentHandlerData, 
     "application/vnd.wap.multipart.mixed||multipart/mixed");

// FORWARD DECLARATIONS

class CMultipartIteratorBase;

// CLASS DECLARATION

/**
* The MultiPart/Mixed Content Handler 
*
* Takes apart the mulitpart message, creating a new sub message from each 
* part, then calls on new Push Handlers to process each sub message.
*
* All multipart/mixed messages are dropped if __TEST_MULTIPART_MIX_SUPP 
* is not defined.
*/ 
class CMultiPartMixedContentHandler : public CContentHandlerBase
	{
    public: // Constructors and destructor

	    static CMultiPartMixedContentHandler* NewL();

	    virtual ~CMultiPartMixedContentHandler();

    private: // Constructors

	    CMultiPartMixedContentHandler();

	    void ConstructL();

    private: // New functions

#ifdef __TEST_MULTIPART_MIX_SUPP

	    void LoadMultipartMsgL();

	    void HandlePartL();

	    void NextPartL();

#endif // __TEST_MULTIPART_MIX_SUPP

    private:	// Methods from CPushHandlerBase
	    
	    void HandleMessageL(CPushMessage* aPushMsg, TRequestStatus& aStatus);

	    void HandleMessageL(CPushMessage* aPushMsg);

	    void CancelHandleMessage();

	    void CPushHandlerBase_Reserved1();

	    void CPushHandlerBase_Reserved2();

    private:	// Methods from CActive

	    void DoCancel();

	    void RunL();

	    TInt RunError(TInt aError);

    private:	// Attributes

	    enum TState
            {
            ELoadMsgData, 
            EHandlePart, 
            ENextPart, 
            EDone
            };

	    CMultipartIteratorBase*		iMultiMessage;

	    CContentHandlerBase*		iContentHandler;
	    
	};


#endif	// __CMULTIPARTMIXEDCONTENTHANDLER_H__
