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
*      Interface to Status Pane.
*  
*
*/


#ifndef MDISPLAY_H
#define MDISPLAY_H

// INCLUDES
#include <Browser_platform_variant.hrh>

// FORWARD DECLARATIONS

class CAknNavigationControlContainer;
class CEikStatusPane;

// CLASS DECLARATION

/**
*  Interface to GUI components.
*/
class MDisplay
    {
	public:	// Status Pane / NaviPane handling

        /**
        * Return Status Pane. Can be NULL.
        * @return Status Pane.
        */
        virtual CEikStatusPane* StatusPane() const = 0;

        // VAG 01.Feb.2002.
        // Please do not add Push/Pop-NaviDecoratorL methods. Ownership of
        // the decorator becomes problematic. The safe way is:
        // Create a decorator as an own member, push it to Navi Pane, and
        // simply delete that decorator in your destructor (deleting it pops
        // automatically). This cannot go wrong, and does not contain leaving
        // code in destructors.

        /**
        * Get Navi Pane; leave on error.
        * @return Navi Pane. Never NULL.
        */
        virtual CAknNavigationControlContainer* NaviPaneL() const = 0;

	public:	// TitlePane handling

        /**
        * Set title pane text.
        * @param aTitle Title text to be displayed.
        */
        virtual void SetTitleL( const TDesC& aTitle ) = 0;
        
        /**
        * Set title pane text from resource.
        * @param aTitle Resource id of title text to be displayed.
        */
        virtual void SetTitleL( TInt aResourceId ) = 0;                
                
	public:	// Animation in contextpane

		/**
		* Starts animation and progress indicator.
		* @param aBearerType Determines what is to be animated.
		*/
		virtual void StartProgressAnimationL() = 0;

		/**
		* Stops animation and progress indicator.
		*/
		virtual void StopProgressAnimationL() = 0;

        /**
        * Notify that new data has arrived to the loaders.
        * @since Series60_version 2.5
        * @return none
        */
        virtual void NotifyProgress() = 0;

    public:  // Transaction

		/**
		* Adds transaction id
		* @param aId Transaction id
		*/
        virtual void AddTransActIdL( TUint16 aId ) const = 0;

        /**
        * Saves maximum and received data with a transction id. 
        * @since Series60 1.2
        * @param aId Id of the transaction
        * @param aRecvdData Amount of data we have received with current transaction Id
        * @param aMaxData Max size of the transaction
        */
        virtual void AddProgressDataL( TUint16 aId, TUint32 aRecvdData, TUint32 aMaxData ) const = 0;

        /**
        * Marks particular transaction id to complete status
        * @since Series60 2.0
        * @param aId Id of the transaction
        */
        virtual void TransActIdComplete( TUint16 aId ) const = 0;

	public:	// Secure indicator

		/**
		* Shows or hides secure indicator depending on aState.
		* @param aState EAknIndicatorStateOff/EAknIndicatorStateOn.
		*/
		virtual void UpdateSecureIndicatorL( const TInt aState ) = 0;
        
		/**
		* Sets GPRS indicator on for Full Screen Mode
        */
        virtual void SetGPRSIndicatorOnL() = 0;

        /**
        * Starts the Wlan Indicator observer
        */
        virtual void StartWlanObserverL() = 0;
        
        /**
        * Updates the Wlan Indicator to current value
        */
        virtual void UpdateWlanIndicator( const TInt aWlanValue ) = 0;

        /**
        * Initialises Full Screen pane indicators
        */

        virtual void FSPaneOnL( ) = 0;

         /**
        * Updates Full Screen progress indicator
        */       

        virtual void UpdateFSProgressIndicator(const TInt aMaxData,  const TInt aReceivedData) = 0;

         /**
        * Starts Full Screen Wait indicator
        */  

        virtual void StartFSWaitIndicator() = 0;

         /**
        * Updates 'downloaded data amount' message on Full Screen pane
        */   

        virtual void UpdateFSProgressDataL(const TDesC16& aReceivedDataText) = 0;

        /**
        * Clears the Message Info indicator in the navipane
        */
        virtual void ClearMessageInfo() = 0;

        /**
        * Sets formerly saved page title. SetTitleL's functionality moved here, because page title restoration is
        * necessary in Full Screen mode
        */       

        virtual void RestoreTitleL() = 0;

        /**
        * Updates initial download indicator on Full Screen pane
        */       

        virtual void UpdateFSDownloadInitialIndicator(const TBool aState) = 0;

        /**
        * Initializes Full Screen Indicators
        */

        virtual void InitIndicatorsL() = 0;
        
        /**
		* Updates the Multiple Windows Indicator
		* @param aState Indicator On or Off
		* @param aWinCount The number of windows open
		* @since Series60 3.0
		*/
        virtual void UpdateMultipleWindowsIndicatorL( 
                            const TBool aState, const TInt aNumWindows ) = 0;
    };

#endif
            
// End of File
