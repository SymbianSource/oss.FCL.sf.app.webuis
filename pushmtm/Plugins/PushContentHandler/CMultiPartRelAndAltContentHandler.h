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
* Description:  Declaration of CMultiPartRelAndAltContentHandler.
*
*/



#ifndef __CMULTIPARTRELANDALTCONTENTHANDLER_H__
#define __CMULTIPARTRELANDALTCONTENTHANDLER_H__

// System includes
//
#include "PushMtmDef.hrh"
#include "PushContentHandlerDef.hrh"
#include <CContentHandlerBase.h>
#include <E32Base.h>
#include <msvstd.h>
#include <msvapi.h>

// Constants
//
const TUid KUidPushMultiPartRelAndAltContentHandler	= { 
           EUidPushMultiPartRelAndAltContentHandler };

// Forward class declarations
//
class CMsvSession;
class CMultipartIteratorBase;

/**
* The MultiPart/Alternative & Multipart/Alternative Content Handler 
*
* Saves the whole message in the Message Server, each part getting saved as a child
* entry to the main part.
* 
* All multipart/rel+alt messages are dropped if __TEST_MULTIPART_REL_SUPP 
* is not defined.
*/
class CMultiPartRelAndAltContentHandler : public CContentHandlerBase, 
                                          public MMsvSessionObserver
	{
    public:	// Methods

	    static CMultiPartRelAndAltContentHandler* NewL();

	    virtual ~CMultiPartRelAndAltContentHandler();

    private: // Constructors

	    CMultiPartRelAndAltContentHandler();

	    void ConstructL();

    private: // New functions

#ifdef __TEST_MULTIPART_REL_SUPP

        void LoadMultiPartMsgL();
	    
	    void HandlePartL();

	    void NextPartL();

	    void SaveMsgRootL();

	    void LoadMsgDataL();

	    void SetMsgCorruptFlagL();

#endif // __TEST_MULTIPART_REL_SUPP

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

    private: // from MMsvSessionObserver

        void HandleSessionEventL( TMsvSessionEvent aEvent, 
                                  TAny* aArg1, TAny* aArg2, TAny* aArg3);

    private:	// Attributes

	    enum TState
            {
            ELoadMsgData,
            EHandlePart, 
            EDone, 
            ENextPart
            };

        CMsvSession* iMsvSession; ///< Has.

        TMsvId              iSavedMsgId;

	    CMultipartIteratorBase*	iMultiMessage;
    };

#endif    // __CMULTIPARTRELANDALTCONTENTHANDLER_H__

