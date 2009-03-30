/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  View class for playing media in full screen.
*
*/


#ifndef BAVPVIEWFULLSCREEN_H
#define BAVPVIEWFULLSCREEN_H

// INCLUDES
#include <coecntrl.h>
#include <e32property.h>  // RProperty class
#include <ScreensaverInternalPSKeys.h>
#include <HWRMLight.h>

// CLASS DECLARATION
class CBavpControllerVideo;
/**
*  CBavpViewFullScreen
*  Controller for full screen
*/
class CBavpViewFullScreen : public CCoeControl
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBavpViewFullScreen* NewL(CBavpControllerVideo* aController);

        /**
        * Destructor.
        */
        virtual ~CBavpViewFullScreen();

    public: // From base classes
        /**
         * Returns RWidnows
         * @since S60 v3.2
         * @param  none
         * @return RWindow: handle to a standard window
         */
        RWindow& WindowInst() { return Window(); }
        /**
         * Draw from CCoeControl
         * @since S60 v5.0
         * @param  aRect
         * @return void
         */
        void Draw( const TRect& aRect ) const;
       /**
        * Handles pointer events
        * @since S60 v5.0
        * @param  aPointerEvent
        * @return void
        */
        void HandlePointerEventL(const TPointerEvent& aPointerEvent);

    private:

        /**
        * C++ default constructor.
        */
        CBavpViewFullScreen(CBavpControllerVideo* aController);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:

    // Rectangle that defines the control's extent
       TRect iRect;
	   CBavpControllerVideo* iNormalController;
    };

#endif // BAVPVIEWFULLSCREEN_H

// End of File
