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



#ifndef CBROWSERFAVICONSCALER_H
#define CBROWSERFAVICONSCALER_H

//  INCLUDES

#include <e32base.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

class CBitmapScaler;
class CFbsBitmap;

// CLASS DECLARATION

class MBrowserFaviconScalerCallback
    {
    public:
        virtual void ScalingCompletedL(CFbsBitmap* aResult, const TRect& aTargetRect) = 0;
    };


// CLASS DECLARATION

/**
*
*
*  @lib pagescaler.dll
*  @since 3.1
*/
class CBrowserFaviconScaler : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBrowserFaviconScaler* NewL(MBrowserFaviconScalerCallback& aCallback);

        /**
        * Destructor.
        */
        virtual ~CBrowserFaviconScaler();

    public: // New functions

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        void StartScalingL(CFbsBitmap &aSource, const TRect& aTargetRect, TBool aIsMask);


    private: // Functions from CActive

        void RunL();

        void DoCancel();


    private:

        /**
        * C++ constructor.
        */
        CBrowserFaviconScaler(MBrowserFaviconScalerCallback& aCallback);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        void DeleteResultBitmap();

    private:    // Data

        MBrowserFaviconScalerCallback* iCallback;

        CFbsBitmap* iResultBitmap; // owned
        CBitmapScaler* iScaler; // owned

        TRect iTargetRect;

    };

#endif      // CBROWSERFAVICONSCALER_H

// End of File
