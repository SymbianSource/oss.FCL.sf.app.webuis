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
* Description:  Uses the interface MPluginAdapter to notify browser of construction 
				 and destruction of the plug-in, implements interface MPluginOptionHandler
				 to add menus and draws animated images while video player loading the data
*
*/


#ifndef BAVPVIEW_H
#define BAVPVIEW_H

//  INCLUDES
#include <coecntrl.h>
#include <coedef.h>

#include "BrowserAudioVideoPlugin.hrh"
#include <pluginadapterinterface.h>

// CLASS DECLARATION
class CBavpPlugin;
class CBavpController;
class CAknBitmapAnimation;

/**
*  MBavpView
*  Handles window controls for the Browser Audio Video Plugin
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class MBavpView
	{
    public:
		/**
        * Handles the player animation state changes
		* @since 3.2
        * @param  none
        * @return void
        */
		virtual void UpdateView() = 0;

		/**
        * Returns CCoeControl instance 
		* @since 3.2
        * @param  none
        * @return CCoeControl instance itself
        */
		virtual CCoeControl& CoeControl() = 0;

		/**
        * Returns handle to a standard window
		* @since 3.2
        * @param  none
        * @return void
        */
		virtual RWindow& WindowInst() = 0;

		/**
        * Check if the window is the top window instance
		* @since 3.2
        * @param  none
        * @return void
        */
		virtual TBool IsTopWindowL() = 0;

		/**
        * Check controller status 
		* @since 3.2
        * @param  none
        * @return void
        */
        virtual void ControllerStateChangedL() = 0;

        virtual TRect WindowRect() = 0;
        virtual CBavpPlugin* bavPlugin() = 0;
	};


/**
*  CBavpView
*  Implent the interface of MBavpView
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class CBavpView : public CCoeControl,
				  public MBavpView,
				  public MOptionMenuHandler
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CBavpView* NewL( const CCoeControl* aParent, const TRect& aRect, CBavpPlugin* aBavPlugin );

        /**
        * Destructor.
        */
        ~CBavpView();

    public: // New functions

		/**
		* Sets the controller instance on the view
		* @since 3.2
        * @param CBavpController aController
        * @return void
		*/
		void SetBavpController( CBavpController* aController )
		                        { iBavpController = aController; }

		/**
        * Sets the control's extent, specifying a TRect
		* @since 3.2
        * @param  TRect aRect:The rectangle that defines the control's extent 
        * @return void
        */
		void SetRect( NPWindow* aWindow );

	public: // Methods from MOptionMenuHandler

		/**
        * AddPluginOptionMenuItemsL
        * Add the Option Menu items to the menuPane, these will be handled by plugin.
		* @since 3.2
        * @param aMenuPane handle of the menu that will contain the options 
        * @param aCommandBase base id that needs to be added to the command ID 
        * @param aAfter Plugin options should be added after this index
        * @return void
        */
		void AddPluginOptionMenuItemsL( CEikMenuPane& aMenuPane, TInt aCommandBase,
		                                TInt aAfter );

        /**
        * HandlePluginCommandL
        * Handle the user command from the option menu to the plugin.
		* @since 3.2
        * @param aCommand the command ID of the option
        * @return void
        */
		void HandlePluginCommandL( TInt aCommand );

	public:// Methods from MBavpView

		/**
        * Handles the animation start or cancel status
		* @since 3.2
        * @param  none
        * @return void
        */
		void UpdateView();

		/**
        * CCoeControl instance 
		* @since 3.2
        * @param none
        * @return CCoeControl instance itself
        */
		CCoeControl& CoeControl() { return *this; }

		/**
        * Returns RWidnows 
		* @since 3.2
        * @param  none
        * @return RWindow: handle to a standard window
        */
		RWindow& WindowInst() { return Window(); }
		TRect WindowRect() { return iWindowRect; }

		/**
        * Check if the window is the top window instance
		* @since 3.2
        * @param  none
        * @return void
        */
		TBool IsTopWindowL();

		/**
        * Monitor the controller's status and display mode changes
		* @since 3.2
        * @param  none
        * @return void
        */
		void ControllerStateChangedL();

    public:     // Methods from CCoeControl

        /**
        * OfferKeyEventL 
		* Handle Key press event
		* @since 3.2
        * @param aKeyEvent : The Keyevent that is going to be passed in
        * @param aType : The type of eventcode  that is going to be passed in
        * @return TKeyResponse
        */
		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );
		
		void HandlePointerEventL(const TPointerEvent &aPointerEvent);

	protected:  // Methods from CCoeControl

		/**
		* Responds to size changes to sets the size and position of the contents of this control. 
		* @param none
		* @return void
		*/
		void SizeChanged();

		/**
		* Called when focus changes
		* @param aDrawNow: Flag to redraw now 
		* @return void
		*/
        void FocusChanged( TDrawNow /*aDrawNow*/ );

		/**
        * Draw from CCoeControl 
		* @param aRect: The rect to draw
		* @return void
		*/
        void Draw(const TRect& aRect) const;

	public:
	    CBavpPlugin* bavPlugin() { return iBavPlugin; }
    private:    // methods

        /**
		* C++ default constructor.
        */
        CBavpView();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const CCoeControl* aParent, 
                         const TRect& aRect, CBavpPlugin* aBavPlugin );

		/**
		* Allows us to insert plugin specific option menu items
		*/
		void InsertOneMenuItemL( CEikMenuPane& aMenuPane,
                                 TInt aCommand, TInt aResourceId, TInt index );

		/**
		* Stops all animations and run the one specified
		*/
        void RunAnimation( CAknBitmapAnimation* aAnimation, TBool &aAnimRunning );
        
	private:    // Data

        // Pointer to controller
		CBavpController* iBavpController;

		// The window server session
        RWsSession iWsSession; 

		// Window id
		TInt iWindowId;

		// Rectangle that defines the control's extent
		TRect iRect;
		TRect iWindowRect;
		TRect iClipRect;

		// Flag to add a control from the control stack
		TBool iIsOnStack;

		// Flag to check if video buffering animation running
		TBool iIsVideoBuffAnimRunning;
		CAknBitmapAnimation* iVideoBuffAnimation;

        // Flag to check if bad content animation is running 
		TBool iIsBadContentAnimRunning;
		CAknBitmapAnimation* iBadContentAnimation;
        
        // Flag to check if video stopped animation is running 
		TBool iIsVideoStoppedAnimRunning;
		CAknBitmapAnimation* iVideoStoppedAnimation;
        
        // Flag to check if video paused animation is running 
		TBool iIsVideoPausedAnimRunning;
		CAknBitmapAnimation* iVideoPausedAnimation;
        
		// Flag to check if audio playing animation running 
		TBool iIsAudioPlayAnimRunning;
		CAknBitmapAnimation* iAudioPlayAnimation;

        // Flag to check if audio stopped animation is running 
		TBool iIsAudioStoppedAnimRunning;
		CAknBitmapAnimation* iAudioStoppedAnimation;
        
        // Flag to check if audio paused animation is running 
		TBool iIsAudioPausedAnimRunning;
		CAknBitmapAnimation* iAudioPausedAnimation;
        
		// Command Id that webengine passed in for plugin
		TInt iCommandBase;
		CBavpPlugin* iBavPlugin;
    };

#endif      // CBavpVIEW_H

// End of File
