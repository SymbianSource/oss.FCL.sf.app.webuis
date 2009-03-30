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
*     This class implements the behavior of an anchor element.
*
*/

#ifndef WMLBROWSERBMOTABINSENDER_H
#define WMLBROWSERBMOTABINSENDER_H

// INCLUDES

	// System includes
#include <e32base.h>
#include <bldvariant.hrh>

	// User includes
//#include "WmlBrowserBuild.h"



#include "BmOTABinSender.h"

// FORWARD DECLARATION
class CSendUi;
class CMessageData;

// CLASS DECLARATION
	
/**
*  The class for implementing the CBookmarkOTAItem.
*  Item is binary encoded in WBXML format.
*
*  @lib Browser.app
*  @since Series 60 1.2
*/
class CBookmarkOTAItem : public CBase 
    {
	friend class CWmlBrowserBmOTABinSender;

    public:
        
        /**
        * Two-phased constructor. Leaves on failure.
        * @param aURL is URL of the bookmark or current card's URL address
        * @param aTitle is title of the bookmark or title of the current card's URL address
        * @return new Bookmark OTA item
        */
        static CBookmarkOTAItem* NewL( const TDesC& aURL, 
                                       const TDesC& aTitle );

        /**
        * Destructor.
        */
        virtual ~CBookmarkOTAItem();

    protected:

        /**
        * Constructor.
        */
        CBookmarkOTAItem();

        /**
        * Second phase constructor. Leaves on failure.
        * Encode input params to binary WBXML format.
        * @since Series 60 1.2
        * @param aURL is URL of the bookmark or current card's URL address
        * @param aTitle is title of the bookmark or title of the current card's URL address
        */
        void ConstructL( const TDesC& aURL, const TDesC& aTitle );
    
    protected:    // data
        
        /**
        * Binary encoded bookmark in WBXML format
        */
        HBufC *iBookmark;
    };

/**
*  The class for implementing the CWmlBrowserBmOTABinSender.
*  
*  @lib Browser.app
*  @since Series 60 1.2
*/
class CWmlBrowserBmOTABinSender : public CBase, public MBmOTABinSender
    {
	public :	// public construction

        /**
        * Two-phased constructor. Leaves on failure.
        */
        static CWmlBrowserBmOTABinSender* NewL();

        /**
        * Destructor.
        */
        virtual ~CWmlBrowserBmOTABinSender();

    public:     // from MBmOTABinSender

        /**
        * Reset and destroy bookmark list.
        * @since Series 60 1.2
        */
        void ResetAndDestroy();

        /**
        * Add bookmark title and URL to be sent
        * this class will destroy passed objects.
        * @since Series 60 1.2
        * @param aURL bookmark's URL
        * @param aTitle bookmark's title can be Null
        */
        void AppendL( const TText *aURL, const TText *aTitle = NULL );

        /**
        * Add bookmark title and URL to be sent
        * this class will destroy passed objects.
        * @since Series 60 1.2
        * @param aURL bookmark's URL
        * @param aTitle bookmark's title
        */
        void AppendL( const TDesC& aURL, const TDesC& aTitle );

        /**
        * Sends all the bookmarks added with AddL().
        * @since Series 60 1.2 
        */
        void SendAddressL(  );
        
        /**
        * Sends an OPML file.
        * @since Series 60 3.2
        */
        virtual void SendOPMLFileL(const TDesC& aOPMLFile);

    protected:  // construction

        /**
        * Second phase constructor. Leaves on failure.
        */
        void ConstructL();
    
        /**
        * Constructor.
        */
        CWmlBrowserBmOTABinSender();

    private:    // new methods
        
        /**
        * Creates a new attachment file for BT and IR´.
        * @since Series 60 1.2
        * @param fs Already connected file session
        * @param aBody body of the message
        * @return system error code or KErrNone
        */
        TInt WriteMessageBodyIntoFileL( RFs &fs, TDesC& aBody );

    private:    // data

        /// List of bookmarks to be sent. Owned.
        CArrayPtrFlat<CBookmarkOTAItem> iBookmarks;

        /// Pointer to SendUi. Owned.
        CSendUi*      iSendUi;
    };


#endif  // WMLBROWSERBMOTABINSENDER_H
// End of File