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
* Description: 
*      Declaration of CStringResourceReader.
*      
*
*/


#ifndef STRINGRESOURCEREADER_H
#define STRINGRESOURCEREADER_H

// INCLUDE FILES

#include <e32base.h>
#include <e32std.h>
#include <barsc.h>

// FORWARD DECLARATIONS

class RFs;

// CLASS DECLARATION

/**
* Utility class that helps reading string resources without CONE.
* Not intended for derivation.
*/
class CStringResourceReader : public CBase
    {
    public: // Constructors and destructor

        CStringResourceReader( RFs& aFs, const TDesC& aRscFileWithPathAndDrive );

        ~CStringResourceReader();

    public: // New functions

        /**
        * Return a given string from resource.
        * @param aResId Resource ID.
        * @return See above.
        */
        HBufC* AllocReadResourceL( TInt aResId );
        HBufC* AllocReadResourceLC( TInt aResId );

    private: // New functions

        void InitializeL();

    private: // Data members

        RFs& iFs;                ///< File server session.
        TBuf<64> iRscFileName;
        RResourceFile iResourceFile;
        TBool iInitialized;
    };

#endif // STRINGRESOURCEREADER_H

// End of file.
