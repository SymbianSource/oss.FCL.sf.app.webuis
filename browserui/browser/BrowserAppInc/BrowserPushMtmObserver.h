/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Definition of CBrowserPushMtmObserver
*
*/


#ifndef BROWSERPUSHMTMOBSERVER_H
#define BROWSERPUSHMTMOBSERVER_H

//  INCLUDES
#include <e32base.h>
#include <f32file.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class MApiProvider;

// CLASS DECLARATION

/**
*
*  @lib
*  @since 3.0
*/
NONSHARABLE_CLASS(CBrowserPushMtmObserver) : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @since 3.0
        * @param
        * @param
        * @return CacheHandler object.
        */
        static CBrowserPushMtmObserver* NewL( MApiProvider* aApiProvider );

        /**
        * Destructor.
        */
        virtual ~CBrowserPushMtmObserver();

    public: // new functions
        void StartObserver();
        HBufC8* ReadMsgFileL();

    public: // from base class CActive
        void RunL();
        TInt RunError(TInt aError);
        void DoCancel();

    private:

        /**
        * Construct.
        * @since 3.0
        * @param
        * @param
        * @return PushMtmObserver object.
        */
        CBrowserPushMtmObserver( MApiProvider* aApiProvider );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        RFs iFsSession; // not owned
        MApiProvider* iApiProvider;
    };

#endif      // BROWSERPUSHMTMOBSERVER_H

// End of File
