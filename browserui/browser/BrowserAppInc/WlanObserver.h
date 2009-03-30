/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/


#ifndef WLANOBSERVER_H
#define WLANOBSERVER_H

// INCLUDES
#include <e32base.h>
#include <e32property.h>

// FORWARD DECLARATIONS
class MApiProvider;


// CONSTANTS

// State enum and member variable

// CLASS DECLARATIONS
/**
*  CWlanObserver.
*  Observer class for handling wlan status requests
*  @since S60 v5.0
*/
class CWlanObserver : public CActive
    {
    public:    // Constructors and destructor

        /**
        * Constructor.
        * @param aApiProvider Reference to API provider
        */
        CWlanObserver(MApiProvider& aApiProvider);
        
        /**
         * Destructor.
         */
        virtual ~CWlanObserver();
        
        /**
        * Retrieves current state of wireless lan connection
        * @param aWlanStateValue reference to current Wlan state value which is updated here
        * @return: EFalse if error condition (ignore aWlanStateValue in this case)
        *          Etrue if no error (get updated value from aWlanStateValue)
        */
        TBool GetCurrentState(TInt &aWlanStateValue);        
        
    protected:    // CActive

        /**
        * Implements cancellation of an outstanding request.
        * @param none
        * @return void
        */
        void DoCancel();
       
        /**
        * Handles an active object’s request completion event.
        * @param none
        * @return void
        */
        void RunL();
 
        /**
        * Handles an active object’s error
        * @param error
        * @return void
        */
        TInt RunError(TInt aError);

    private:    // new methods

        MApiProvider& iApiProvider;         // Api provider
        
        RProperty iWlanProperty;
        
          
    };

#endif      // WLANOBSERVER_H

// End of File
