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
* Description:  ?Description
*
*/


#ifndef __BROWSERDOWNLOADINDICATORTIMER_H
#define __BROWSERDOWNLOADINDICATORTIMER_H

// INCLUDES
#include <eikenv.h>

// FORWARD DECLARATIONS
class MBrowserDownloadIndicatorTimerEvent;

/**
 * CDownloadIndicator.
 * @lib Browser.app
 * @since Series 60 1.2
 */

class CBrowserDownloadIndicatorTimer : public CTimer 
    {
    public: //Constructors and destructor

        /**
        * Destructor.
        */
        ~CBrowserDownloadIndicatorTimer();

    public:

        /**
         * Two-phased constructor.
         * @param aUi   CBrowserDownloadIndicatorTimer (i.e. BrowserAppUi ) pointer
         */
        static CBrowserDownloadIndicatorTimer* NewL( 
                                    MBrowserDownloadIndicatorTimerEvent& aUi );

        /**
         * Two-phased constructor. Cleanupstack used.
         * @param aUi   CBrowserDownloadIndicatorTimer (i.e. BrowserAppUi ) pointer
         * @since       Series60 1.2
         */
        static CBrowserDownloadIndicatorTimer* NewLC( 
                                    MBrowserDownloadIndicatorTimerEvent& aUi );

    public:

        /**
        *
        *
        */
        void Start( TTimeIntervalMicroSeconds32 aPeriod );

    public: // From CActive

        void RunL();

    private:

        /**
        * Default constructor.
        */
        CBrowserDownloadIndicatorTimer();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( MBrowserDownloadIndicatorTimerEvent& aUi );

    private:    //data

    MBrowserDownloadIndicatorTimerEvent* iProgressIndicator;
    };

#endif //__BROWSERDOWNLOADINDICATORTIMER_H

// End of File
