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
* Description:  Download handling utilities.
*
*/


#ifndef BROWSER_DOWNLOAD_UTIL_H
#define BROWSER_DOWNLOAD_UTIL_H

// INCLUDE FILES

#include <e32base.h>

// FORWARD DECLARATIONS

class MApiProvider;

// CLASS DECLARATION

/**
* Execute the Downloads List in RunL.
*/
class CAsyncDownloadsListExecuter : public CActive
    {
    public:
    
        CAsyncDownloadsListExecuter( MApiProvider& aApiProvider );
        ~CAsyncDownloadsListExecuter();
        
    public:
    
        void Start();
        
    private:
    
        void DoCancel();
        void RunL();
        TInt RunError( TInt aError );
        
    private: // Data
    
        MApiProvider& iApiProvider;
    };

#endif // BROWSER_DOWNLOAD_UTIL_H

// End of file