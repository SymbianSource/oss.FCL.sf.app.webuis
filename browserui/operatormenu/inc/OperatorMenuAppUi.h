/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Header file for COperatorMenuAppUi
*
*/


#ifndef OPERATORMENUAPPUI_H
#define OPERATORMENUAPPUI_H

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
#include "OperatorMenuPrivateCRKeys.h"
#include <favouriteslimits.h> //KFavouritesMaxUrl
#include <BrowserOverriddenSettings.h>

// CONSTANTS
const TUid KUidBrowserApplication = { 0x10008d39 };				//Browser UID
const TUid KUidCacheManager = { 0x101f8557 };			   //Cache manager UID
_LIT( KBrowserPath, "Z:\\system\\apps\\Browser\\Browser.app" );	//Browser path

// FORWARD DECLARATIONS
class COperatorMenuContainer; 
class COperatorMenuAppUi;

// CLASS DECLARATION

/**
* Class responsible for delayed browser lunching.
* 
* @since Series 60 2.0
*/
class COperatorDelayedStarter : public CActive
{
public:
	COperatorDelayedStarter(COperatorMenuAppUi* aOperatorMenuAppUi);
	~COperatorDelayedStarter();
	//Start() puts the class to the active scheduler
	void Start();
	// From CActive
	void RunL();
	void DoCancel();
private:
	COperatorMenuAppUi* iOperatorMenuAppUi;
};
/**
* Application UI class.
* Provides support for the EIKON control architecture
* 
* @since Series 60 2.0
*/
class COperatorMenuAppUi :	public	CAknAppUi, 
							public	MDownloadedContentHandler, 
									MApaEmbeddedDocObserver
    {
    public: // Constructors and destructor

		/**
		* Default Constructor
		*/
		COperatorMenuAppUi();
			
        /**
        * Destructor.
        */      
        ~COperatorMenuAppUi();
        
    public: // New functions

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
        */
        void HandleCommandL( TInt aCommand );

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event. 
        * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
        */
        virtual TKeyResponse HandleKeyEventL( 
            const TKeyEvent& aKeyEvent,TEventCode aType );

    private: //From MApaEmbeddedDocObserver
		/**
		* From MApaEmbeddedDocObserver, observes when the 
		* embedded browser exits
		* @param aMode The browser exit mode (ignored in OperatorMenu)
		*/
		void NotifyExit( TExitMode aMode );

	private: //New functions
		/**
		* Launches browser with operator specific url as start page
		* @since Series 60 2.0
		*/
		void LaunchBrowserEmbeddedWithOperatorUrlL();

		/**
		* Launches browser in embedded mode with the default 
		* Access Point's homepage as start page
		* @since Series 60 2.0
		*/
		void LaunchBrowserEmbeddedWithDefaultAPHomepageL();
	public:	
		/**
		* Launches the browser according to the available parameters 
		* ( operator specific url )
		* @since Series 60 2.0
		*/
		void LaunchBrowserL();

    public: // Functions from MEikStatusPaneObserver

        /**
        * Handles status pane size change.
        */
        void HandleStatusPaneSizeChange();

	private: //Data

		//OperatorMenu App view container (not shown in final App)
        COperatorMenuContainer* iAppContainer;
        
        // Access central repository of HttpCacheManager
        CRepository* iRepository;
        
        // Access central repository of OperatorMenu
        CRepository* iRepositoryOp;

		//Buffer where the url is read
		TBuf<KFavouritesMaxUrl>	iUrl;

        //settings to override
        TBrowserOverriddenSettings *iOverriddenSettings;

		TBool iLaunched;
		COperatorDelayedStarter* iDelayedStarter;
    };

#endif

// End of File
