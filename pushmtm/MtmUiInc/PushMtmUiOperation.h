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
* Description:  Declaration of CPushMtmUiOperation.
*
*/



#ifndef PUSHMTMUIOPERATION_H
#define PUSHMTMUIOPERATION_H

//  INCLUDES

#include "PushMtmOperation.h"
#include <e32std.h>
#include <ConeResLoader.h>

// FORWARD DECLARATIONS

class CCoeEnv;
class CPushMtmUtil;

// CLASS DECLARATION

/**
* Base class for UI operations. It extends CPushMtmOperation with 
* AssureResourceL().
*/
class CPushMtmUiOperation : public CPushMtmOperation
    {
    protected: // Constructors and destructor

        /**
        * Constructor.
        * @param aSession Message Server Session to be used by this operation.
        * @param aId Id of entry to operate on.
        * @param aObserverStatus Observer's status.
        */
        CPushMtmUiOperation( CMsvSession& aSession, 
                             TMsvId aId, 
                             TRequestStatus& aObserverStatus );

		/**
		* Symbian OS constructor. Derived classes must call this!
		*/
		void ConstructL();

        /**
        * Destructor.
        */
        virtual ~CPushMtmUiOperation();

    protected: // New functions

        /**
        * Add the resource file to the Cone.
        * Must be called before using a resource from this file to ensure that 
        * the resource file is loaded. Only one file can be loaded that is 
        * closed only in the destructor.
        * @param aResourceFile Resource file name to be loaded.
        * @return None.
        */
        void AssureResourceL( const TDesC& aResourceFile );

        /**
        * Show a global error note using CErrorUI.
        * @param aError Error code.
        * @return None.
        */
        void ShowGlobalErrorNoteL( TInt aError ) const;

    protected: // Data members

        CCoeEnv& iCoeEnv;
        CPushMtmUtil* iMtmUtil; ///< Utility. Has.
        RConeResourceLoader iResourceLoader; ///< Resource loader.
        TBool iResourceOpened; ///< Indicate if the resource is opened.
    };

#endif // PUSHMTMUIOPERATION_H
            
// End of file.

