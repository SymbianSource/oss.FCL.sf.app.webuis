/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BROWSERWINDOWFOCUSNOTFIER_H
#define BROWSERWINDOWFOCUSNOTFIER_H

//  INCLUDES
// User includes
// System Includes
#include <e32base.h>

// CONSTANTS

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Provide dialogs needed for browser operation
*
*  @since 3.0
*/
class CBrowserWindowFocusNotifier : public CBase   
    {
    friend class CBrowserDialogsProviderProxy;
    
    // Construction/Destruction
    public:
        
        // Two stage constructor
        static CBrowserWindowFocusNotifier* NewL();
              
        // Destructor
        virtual ~CBrowserWindowFocusNotifier();

    protected:

        // 1st stage Constructor
        CBrowserWindowFocusNotifier();

        // 2nd stage constructor
        void ConstructL();

    public:         // New functions
        
        TInt Add( TRequestStatus* aStatus );
        void OnFocusGained();
        
    protected:      // New functions

        void FlushAOStatusArray();
        
    // DATA MEMBERS
    private:
    
        RPointerArray<TRequestStatus> iAOStatusArray;
    };

#endif      // BROWSERWINDOWFOCUSNOTFIER_H   
            
// End of File
