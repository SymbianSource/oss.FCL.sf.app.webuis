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
*      Helper class to put animation to contextPane.
*      
*
*/


#ifndef BROWSERANIMATION_H
#define BROWSERANIMATION_H

//  INCLUDES

#include <E32BASE.H>

// FORWARD DECLARATIONS

class CAknBitmapAnimation;
class CFbsBitmap;
class CEikImage;

// CLASS DECLARATION

/**
*  Encapsulates the animation feature of the browser.
*/
class CBrowserAnimation : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
		* @param aResourceId BMPANIM_DATA resource
        */
 	static CBrowserAnimation* NewL( TInt aResourceId );

       /**
        * Destructor.
        */
        virtual ~CBrowserAnimation();

    public: // New functions
        
        /**
        * Starts animation.
        */
        void StartL();

        /**
        * Stops animation.
        */
        void StopL();

        TBool IsRunning() { return iIsRunning; };

    protected:
        
        /**
        * constructor that can leave.
        * @param aResourceId BMPANIM_DATA resource
        */
        void ConstructL( TInt aResourceId );
        
    private:    // Data
        
        CAknBitmapAnimation* iAnimation; // animation object
        
        TBool iIsRunning; // Animation is running
    };

#endif
            
// End of File
