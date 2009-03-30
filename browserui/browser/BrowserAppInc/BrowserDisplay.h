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
*      Access to display components
*
*
*/


#ifndef BROWSER_DISPLAY_H
#define BROWSER_DISPLAY_H

// INCLUDE FILES
#include <bldvariant.hrh>
#include <aknenv.h>
#include "Display.h"
#include "WlanObserver.h"

// FORWARD DECLARATIONS

class MApiProvider;
class CBrowserProgressIndicator;
class CAknNavigationDecorator;
class CAknIndicatorContainer;
class CBrowserWindow;

// CLASS DECLARATION

class CBrowserDisplay : public CBase, public MDisplay
	{
	public :	// constructor, destructor

        /**
        * Two-phased constructor.
        */
 		static CBrowserDisplay* NewL( MApiProvider& aApiProvider,
 		    CBrowserWindow& aWindow );

        /**
        * Destructor.
        */
 		virtual ~CBrowserDisplay();

	public :	// functions from MDisplay

        /**
        * Return Status Pane. Can be NULL.
        * @return Status Pane.
        */
        virtual CEikStatusPane* StatusPane() const;

        /**
        * Get Navi Pane; leave on error.
        * @return Navi Pane. Never NULL.
        */
        virtual CAknNavigationControlContainer* NaviPaneL() const;

        /**
        * Set title pane text.
        * @param aTitle Title text to be displayed.
        */
        virtual void SetTitleL( const TDesC& aTitle );

        /**
        * Set title pane text from resource.
        * @param aTitle Resource id of title text to be displayed.
        */
        virtual void SetTitleL( TInt aResourceId );

		/**
		* Starts animation.
		* @param aBearerType Determines what is to be animated.
		*/
		virtual void StartProgressAnimationL(/* TApBearerType aBearerType*/ );

		/**
		* Stops animation.
		*/
		virtual void StopProgressAnimationL();

		/**
		* Shows or hides secure indicator depending on aState.
		* @param aState EAknIndicatorStateOff/EAknIndicatorStateOn.
		*/
		virtual void UpdateSecureIndicatorL( const TInt aState );

        /**
        * Adds transaction id to array.
        * @param aId Transaction id
        */
        virtual void AddTransActIdL( TUint16 aId ) const;

        /**
        * Saves maximum and received data with a transction id. 
        * @since Series60 1.2
        * @param aId Id of the transaction
        * @param aRecvdData Amount of data we have received with current transaction Id
        * @param aMaxData Max size of the transaction
        */
        virtual void AddProgressDataL( TUint16 aId, TUint32 aRecvdData, TUint32 aMaxData ) const;

        /**
        * Marks particular transaction id to complete status
        * @since Series60 2.0
        * @param aId Id of the transaction
        */
        virtual void TransActIdComplete( TUint16 aId ) const;

        /**
        * Sets Secure indicator on/off, UpdateFullScreenIndicatorsL() needs to be called after indicators are set
        * @param aState indicator state on/off
        */
        void SetFullScreenSecureIndicatorL(const TBool aState);

        /**
        * Sets gprs indicator for Full Screen on
        */
        virtual void SetGPRSIndicatorOnL();

        /**
        * Starts the Wlan Indicator observer
        */
        virtual void StartWlanObserverL();
        
        /**
        * Updates the Wlan Indicator to current value
        */
        virtual void UpdateWlanIndicator( const TInt aWlanValue );

        /**
        * Initialises Full Screen pane indicators
        */
        virtual void FSPaneOnL( );

        /**
        * Updates Full Screen progress indicator
        */
        virtual void UpdateFSProgressIndicator(const TInt aMaxData,  const TInt aReceivedData);

        /**
        * Starts Full Screen Wait indicator
        */
        virtual void StartFSWaitIndicator();

        /**
        * Updates 'downloaded data amount' message on Full Screen pane
        */
        virtual void UpdateFSProgressDataL(const TDesC16& aReceivedDataText);

        /**
        * Updates initial download indicator on Full Screen pane
        */
        virtual void UpdateFSDownloadInitialIndicator(const TBool aState);

        /**
        * Initializes Full Screen Indicators
        */
        virtual void InitIndicatorsL();
        
        /**
        * Clears the Message Info indicator in the navipane
        */
        virtual void ClearMessageInfo();

        /**
        * Sets formerly saved page title. SetTitleL's functionality moved here, because page title restoration is
        * necessary in Full Screen mode
        */
        virtual void RestoreTitleL();

        /**
        * Notify that new data has arrived to the loaders.
        * @since Series60_version 2.5
        * @return none
        */
        virtual void NotifyProgress();

        /**
		* Updates the Multiple Windows Indicator
		* @param aState Indicator On or Off
		* @param aWinCount The number of windows open
		* @since Series60 3.0
		*/
        virtual void UpdateMultipleWindowsIndicatorL( 
                                const TBool aState, const TInt aNumWindows );

	public:

		inline MApiProvider& ApiProvider() { return iApiProvider; }

    private: //for internal use of full screen indicators

        /**
        * Selects Editor's IndicatorContainer to use.
        */
        CAknIndicatorContainer* SelectIndicContainerEditing();

        /**
        * Selects own IndicatorContainer to use. Editor's or our own.
        */
        CAknIndicatorContainer* SelectIndicContainerOwn();

	protected :	// construction

		/**
		* Default C++ constructor.
		* @param aApiProvider Browser's API provider
		*/
		CBrowserDisplay( MApiProvider& aApiProvider, CBrowserWindow& aWindow );

        /**
        * Symbian OS constructor that can leave.
        */
 		void ConstructL();

	private :	// data members

		CBrowserProgressIndicator* iProgressIndicator; ///< animation for CSD,HSCSD,GPRS connections

		MApiProvider& iApiProvider;		///< reference to API provider
		CBrowserWindow* iWindow;        ///< owner window, object not owned
		CWlanObserver* iWlanObserver;   ///< Wlan status observer

        CAknNavigationDecorator* iEditorIndicatorContainer; /// Full Screen Editor indicator container

        HBufC* iTitle; /// Page title, stored because it needs to be restored after downloading in Full Screen mode.
	};

#endif	// WMLBROWSER_DISPLAY_H

// End of file
