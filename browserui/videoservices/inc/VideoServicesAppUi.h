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
* Description:  Header file for CVideoServicesAppUi
*
*/


#ifndef VideoServicesAPPUI_H
#define VideoServicesAPPUI_H

// INCLUDES
#include <eikapp.h>
#include <eikdoc.h>
#include <eikproc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknappui.h>
#include <browserlauncher.h>
#include <downloadedcontenthandler.h>
#include <centralrepository.h>
#include <HttpCacheManagerInternalCRKeys.h>    
#include "VideoServicesPrivateCRKeys.h"

#include <favouriteslimits.h> // KFavouritesMaxUrl
#include <BrowserOverriddenSettings.h>

// CONSTANTS
const TUid KUidBrowserApplication = { 0x10008d39 };				//Browser UID
_LIT( KBrowserPath, "Z:\\system\\apps\\Browser\\Browser.app" );	//Browser path

// FORWARD DECLARATIONS
class CVideoServicesContainer; 
class CVideoServicesAppUi;

// CLASS DECLARATION

/**
* Application UI class.
* Provides support for the EIKON control architecture
* 
* @since Series 60 3.1
*/
class CVideoServicesAppUi :	public	CAknAppUi, 
							public	MDownloadedContentHandler, 
									MApaEmbeddedDocObserver
    {
    public: // Constructors and destructor

		/**
		* Default Constructor
		*/
		CVideoServicesAppUi();
			
        /**
        * Destructor.
        */      
        ~CVideoServicesAppUi();
        
	public: // From MDownloadedContentHandler

        /**
        * Handle downloaded content that has been saved to a file, 
        * and return ETrue if it was handled successfully.
        * @param aFileName File name with path.
        * @param aParamList Generic parameter list.
        * @param aContinue Output parameter: It must be ETrue if the caller wants 
        *                  Browser not to terminate after the content is handled.
        * @return ETrue if the implementer has processed the content. If it is 
        *                  EFalse, BrowserLauncher opens that content in an 
        *                  appropriate viewer application.
        */
        TBool HandleContentL( const TDesC& aFileName, const CAiwGenericParamList& aParamList, TBool& aContinue );

        /**
        * Reserved for future use.
        */
        TAny* DownloadedContentHandlerReserved1( TAny* aAnyParam );

    private:
		/**
		* EPOC Default Constructor
		*/
        void ConstructL();

		/**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
		* @return void
		* @since Series 60 3.1
        */
        void HandleCommandL( TInt aCommand );

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event. 
        * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
		* @since Series 60 3.1
        */
        virtual TKeyResponse HandleKeyEventL( 
            const TKeyEvent& aKeyEvent,TEventCode aType );

    private: // From MApaEmbeddedDocObserver
		/**
		* From MApaEmbeddedDocObserver, observes when the 
		* embedded browser exits
		* @param aMode The browser exit mode (ignored in VideoServices)
		* @return void
		* @since Series 60 3.1
		*/
		void NotifyExit( TExitMode aMode );

	private: // New functions
		/**
		* Launches browser with "Operator-specified" url as start page
		* @return void
		* @since Series 60 3.1
		*/
		void LaunchBrowserEmbeddedWithVideoUrlL();

    public: // New functions
		/**
		* Launches the browser according to the available parameters 
		* ( "Operator-specified" video url )
		* @return void
		* @since Series 60 3.1
		*/
		void LaunchBrowserL();

    public: // Functions from MEikStatusPaneObserver

        /**
        * Handles status pane size change.
		* @return void
		* @since Series 60 3.1
        */
        void HandleStatusPaneSizeChange();

	private: // Data

		// VideoServices App view container (not shown in final App)
        CVideoServicesContainer* iAppContainer;
        
        // Access central repository of VideoServices
        CRepository* iVideoServicesRepository;

		// Buffer where the url is read
		TBuf<KFavouritesMaxUrl>	iUrl;
    };

#endif

// End of File
