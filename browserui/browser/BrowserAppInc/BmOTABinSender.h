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
*     Declaration of MBmOTABinSender.
*
*
*/



#ifndef MBMOTABINSENDER_H
#define MBMOTABINSENDER_H

// INCLUDES

	// System includes
#include <e32def.h>

	// User includes

// CLASS DECLARATION
	
/**
*  Interface to Bookmarks OTA Binary Sending.
*  @lib Browser.app
*  @since Series 60 1.2
*/
class MBmOTABinSender
    {
    public:

        /**
        * Reset and destroy bookmark list.
        * @since Series 60 1.2
        */
        virtual void ResetAndDestroy() = 0;

        /**
        * Add bookmark title and URL to be sent
        * this class will destroy passed objects.
        * @since Series 60 1.2
        * @param aURL bookmark's URL
        * @param aTitle bookmark's title can be Null
        */
        virtual void AppendL
            ( const TText *aURL, const TText *aTitle = NULL ) = 0;

        /**
        * Add bookmark title and URL to be sent
        * this class will destroy passed objects
        * @since Series 60 1.2
        * @param aURL bookmark's URL
        * @param aTitle bookmark's title
        */
        virtual void AppendL( const TDesC& aURL, const TDesC& aTitle ) = 0;

        /**
        * Sends all the bookmarks added with AddL().
        * @since Series 60 1.2
        */
        virtual void SendAddressL(  ) = 0;

		/**
        * Sends an OPML file.
        * @since Series 60 3.2
        */
        virtual void SendOPMLFileL(const TDesC& aOPMLFile) = 0;
    };

#endif


// End of File