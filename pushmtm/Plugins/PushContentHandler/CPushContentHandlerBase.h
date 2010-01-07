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
* Description:  Declaration of CPushContentHandlerBase.
*
*/



#ifndef __CPUSHCONTENTHANDLERBASE_H__
#define __CPUSHCONTENTHANDLERBASE_H__


// INCLUDE FILES

#include <push/CContentHandlerBase.h>
#include <E32Base.h>
#include <msvstd.h>
#include <msvapi.h>

// FORWARD DECLARATIONS

class CMsvSession;
class CPushMtmUtil;
class CPushMtmSettings;
class CStringResourceReader;

// CLASS DECLARATION

/**
* CPushContentHandlerBase class stands as a base class for almost all push
* content handlers of the Push MTM. It collects the generic functionality of
* a push content handler, such as garbage collection when receiving new
* service message, supporting CMsvSession, CPushMtmUtil and CPushMtmSettings
* objects.
*/
class CPushContentHandlerBase : public CContentHandlerBase,
                                public MMsvSessionObserver
	{
    protected: // Constructors and destructor

        /**
        * Constructor.
        */
	    CPushContentHandlerBase();

        /**
        * Symbian OS constructor. CActiveScheduler::Add( this ) is called in
        * this method!!
        */
	    void ConstructL();

        /**
        * Destructor.
        */
	    virtual ~CPushContentHandlerBase();

    protected: // New functions

        /**
        * Do Garbage Collection synchronously.
        * @return None.
        */
	    void DoCollectGarbageL();

        /**
        * Apply Push MTM settings on the message.
        * @return ETrue if the message passed the filtering.
        */
        TBool FilterPushMsgL();

    protected: // From MMsvSessionObserver

        /**
        * Msv session events are handled by this method. This implementation
        * is empty.
        * @return None.
        */
        void HandleSessionEventL( TMsvSessionEvent aEvent,
                                  TAny* aArg1,
                                  TAny* aArg2,
                                  TAny* aArg3);

    protected: // Data

        CMsvSession* iMsvSession; ///< Session to Message Server. Has.
        CPushMtmUtil* iWapPushUtils; ///< Push Utility. Has.
        CPushMtmSettings* iMtmSettings; ///< Push Settings. Has.
        /// Owned. It uses iMsvSession's RFs!
        CStringResourceReader* iStrRscReader;
    };

#endif	// __CPUSHCONTENTHANDLERBASE_H__

