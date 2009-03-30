/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef GPSOBSERVER_H
#define GPSOBSERVER_H

// INCLUDES
#include <e32base.h>
#include <e32property.h>
#include <npscript.h>
#include <lbs.h>

// FORWARD DECLARATIONS
class CGpsPlugin;

// CONSTANTS

// State enum and member variable

// CLASS DECLARATIONS
/**
*  CGpsObserver.
*  Controller class for handling browser widget requests
*  @lib npGpsPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CGpsObserver) : public CActive
    {
    public:
        /**
        * C++ default constructor.
        */
        static CGpsObserver* NewL( CGpsPlugin* aPlugin );

        /**
        * Destructor.
        */
        virtual ~CGpsObserver();

    public:
        /**
          * Request network status change notification.
          * @since 3.2
          * @param name of the callback
          * @return void
          */
        void RequestNotificationL( const NPString& aEvent );

        /*
         * Gets latitude
         * @since 3.2
         * @return latitude
         */
        TReal Latitude() const;

        /*
         * Gets longitude.
         * @since 3.2
         * @return longitude
         */
        TReal Longitude() const;

        /*
         * Gets altitude
         * @since 3.2
         * @return altitude
         */
        TReal Altitude() const;

        /*
         * Gets horizontal accuracy
         * @since 3.2
         * @return horizontal accuracy
         */
        TReal HorizontalAccuracy() const;

        /*
         * Gets vertical accuracy
         * @since 3.2
         * @return vertical accuracy
         */
        TReal VerticalAccuracy() const;

        /*
         * Gets tick count for the current sample
         * @since 3.2
         * @return
         */
        TInt TickCount() const;

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

    protected:
        /**
        * C++ default constructor.
        */
        CGpsObserver(CGpsPlugin* aPlugin);

        /**
        * C++ default constructor.
        */
        void ConstructL();

    protected:
        CGpsPlugin* iGpsPlugin;

        RPositionServer iServer;
        RPositioner iPositioner;
        TPositionInfo iPositionInfo;
        TPosition iPosition;

        TInt iTickCount;
        HBufC8* iEventHandler;
    };

#endif      // GPSOBSERVER_H

// End of File
