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
* Description:  Declaration of CCOContentHandler.
*
*/



#ifndef __CCOCONTENTHANDLER_H__
#define __CCOCONTENTHANDLER_H__

// INCLUDE FILES

#include "PushContentHandlerDef.hrh"
#include <push/CContentHandlerBase.h>
#include <E32Base.h>

// CONSTANTS

const TUid KUidPushCOContentHandler	= { EUidPushCOContentHandler };
_LIT( KCOContentHandlerData, "text/vnd.wap.co||application/vnd.wap.coc" );

// FORWARD DECLARATIONS

/*class CMsvSession;
class CSLPushMsgEntry;
class CPushMtmUtil;
class CPushMtmSettings;*/

// CLASS DECLARATION

/**
* CCOContentHandler Handles Cache Operation Content
*
* This handler takes ownership of the CO message and processes it
* according to the processing rules
* [WAP Cache Operation version 31-July-2001].
* Also S60 specific requirements are applied.
*
* NB. Currently all CO push messages are discarded!
*/
class CCOContentHandler : public CContentHandlerBase
	{
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @return The constructed object.
        */
	    static CCOContentHandler* NewL();

        /**
        * D'tor.
        */
	    virtual ~CCOContentHandler();

    private: // Constructors

        /**
        * Constructor.
        */
	    CCOContentHandler();

        /**
        * Symbian OS constructor.
        */
	    void ConstructL();

    private: // Methods from CPushHandlerBase

        /**
        * HandleMessage Async. Version. Takes ownership of Push Message and
        * sets self active to continue processing message.
        * @param aPushMsg CPushMessage to process.
        * @param aStatus Request status variable for use in asynchronous
        *        operations.
        * @return None.
        */
	    void HandleMessageL( CPushMessage* aPushMsg, TRequestStatus& aStatus );

        /**
        * HandleMessage Sync. Version. Takes ownership of Push Message and
        * sets self active to continue processing message.
        * Initial State: Set data members then go to the next state
        * @param aPushMsg CPushMessage to process.
        */
	    void HandleMessageL( CPushMessage* aPushMsg );

        /**
        * Same functionality as DoCancel()
        */
	    void CancelHandleMessage();

        /**
        * Reserved function.
        */
	    void CPushHandlerBase_Reserved1();

        /**
        * Reserved function.
        */
	    void CPushHandlerBase_Reserved2();

    private: // Methods from CActive

        /**
        * Current implementation does nothing. TODO on 2.1.
        * @return None.
        */
	    void DoCancel();

        /**
        * Current implementation does nothing. TODO on 2.1.
        * @return None.
        */
	    void RunL();

        /**
        * Current implementation does nothing. TODO on 2.1.
        * @return Error code to scheduler.
        */
	    TInt RunError( TInt aError );

    private: // Data

	    enum TState
            {
            EDone
            };
    };

#endif	// __CCOCONTENTHANDLER_H__

