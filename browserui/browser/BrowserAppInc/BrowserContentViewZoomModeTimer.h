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
* Description:  Header for zoom mode timer
*
*/


#ifndef __BROWSERCONTENTVIEWZOOMMODETIMER_H
#define __BROWSERCONTENTVIEWZOOMMODETIMER_H

// INCLUDES
#include <e32std.h>

// FORWARD DECLARATIONS
class CBrowserContentView;

/**
 * CBrowserContentViewZoomModeTimer.
 * @lib Browser.app
 * @since Series 60 5.0
 */
class CBrowserContentViewZoomModeTimer : public CTimer 
    {
    public: //Constructors and destructor

        /**
        * Destructor.
        */
        ~CBrowserContentViewZoomModeTimer();

        /**
         * Two-phased constructor.
         */
        static CBrowserContentViewZoomModeTimer* NewL( CBrowserContentView* aBrowserContentView );

        /**
         * Two-phased constructor. Cleanupstack used.
         * @since       Series60 5.0
         */
        static CBrowserContentViewZoomModeTimer* NewLC( CBrowserContentView* aBrowserContentView );

    public: // New methods

        /**
        *
        *
        */
        void Start( TTimeIntervalMicroSeconds32 aPeriod );

        /**
        *
        *
        */
        void Restart( TTimeIntervalMicroSeconds32 aPeriod );

    public: // From CActive

        void RunL();

    private:

        /**
        * Default constructor.
        */
        CBrowserContentViewZoomModeTimer();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( CBrowserContentView* aBrowserContentView );

    private:    //data

    CBrowserContentView* iBrowserContentView;
    };

#endif //__BROWSERCONTENTVIEWZOOMMODETIMER_H

// End of File
