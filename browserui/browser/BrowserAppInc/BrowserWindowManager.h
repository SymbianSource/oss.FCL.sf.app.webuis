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
*      Multiple Windows' WindowManager.
*  
*
*/


#ifndef __MULTIPLE_WINDOWS_WINDOWMANAGER_H__
#define __MULTIPLE_WINDOWS_WINDOWMANAGER_H__

// INCLUDES FILES
#include "WmlBrowserBuild.h"
#include "Preferences.h"
#include <e32base.h>
#include <brctldefs.h>
#include "WindowObserver.h"

// FORWARD DECLARATIONS
class CBrowserWindow;
class MApiProvider;
class CBrowserContentView;
class MWindowInfoProvider;
class CWindowInfo;
class CBrowserWindowQue;

// CLASS DECLARATION

/**
*  Interface to query several API implementations via Window class.
*  
*  @lib Browser.app
*  @since Series 60 3.0
*/
class CBrowserWindowManager :   public CBase,
                                public MPreferencesObserver
    {
    public:  // constructors / destructor

        static CBrowserWindowManager* NewLC(
            MApiProvider& aApiProvider,
            CBrowserContentView& aContentView,
            TInt aMaxWindowCount );
        static CBrowserWindowManager* NewL(
            MApiProvider& aApiProvider,
            CBrowserContentView& aContentView,
            TInt aMaxWindowCount );
        ~CBrowserWindowManager();

    public:  // accessing windows / window data

        TInt WindowCount() const;

        inline TInt MaxWindowCount() const;

        // @returns window object
        // possible error codes: KErrNotFound, KErrGeneral
        // searching for a window identified by an Id
        TInt Window( TInt aWindowId, CBrowserWindow*& aWindow );

        // @returns window object
        // possible error codes: KErrNotFound, KErrGeneral
        // searching for a window identified by an Id
        CBrowserWindow* FindWindowL(
            const TDesC& aTargetName );

        CBrowserWindow* CurrentWindow() const;
        
        CBrowserWindowQue* CurrentWindowQue() const;
        
        inline MApiProvider* ApiProvider() const;

        inline CBrowserContentView* ContentView() const;

        CArrayFixFlat<CWindowInfo*>* GetWindowInfoL( 
            MWindowInfoProvider* aWindowInfo );

        inline void SetUserExit( TBool aUserExit );

        void SendCommandToAllWindowsL( TInt aCommand );
        
        /**
        * Adds an observer which is notified about window events
        * @param aObserver The observer which will receive window events.
        * @since series 60 3.1
        */
        void AddObserverL( MWindowObserver* aObserver );

        /**
        * Removes an observer which won`t be notified about window events.
        * @param aObserver The observer which will not receive 
        *                  further window events.
        * @since 3.1        
        */
        void RemoveObserver( MWindowObserver* aObserver );
        
        /**
        * Notifies the observer about an event.
        * @param aEvent The type of the event.
        * @since 3.1        
        */
        void NotifyObserversL( TWindowEvent aEvent, TInt aWindowId = 0 );

    public:  // window management

        // possible leave codes: KErrNoMemory
        // Window specific settings are read from ApiProvider().Preferences()
        // based on the current state
        CBrowserWindow* CreateWindowL(
            TInt aParentId,
            const TDesC* aTargetName );

		/**
        * Deletes a window.
        * @param aWindowId the window to be deleted
        * @param aUserInitiated: user or script initiated the process
        * @return the windowId should be activated or 0
		*/
        TInt DeleteWindowL( TInt aWindowId, TBool aForceDelete = EFalse );

        // possible leave codes: KErrNotFound
        TInt SwitchWindowL( TInt aWindowId, TBool aDraw = ETrue );
        
        // is there any content in the window?
        inline TBool IsContentExist() const;

        // content was shown in the window
        inline void SetContentExist( TBool aValue );
        
        // Sets the type of content in the view
        void SetCurrentWindowViewState(TBrCtlDefs::TBrCtlState aViewState, TInt aValue);
        
        //Closes all windows except the current one
        void CloseAllWindowsExceptCurrent();

    public: // from MPreferencesObserver
    
        /**
        * Observer for Preference changes
        */
        virtual void HandlePreferencesChangeL( 
                                    const TPreferencesEvent aEvent,
    	                            TPreferencesValues& aValues,
    	                            TBrCtlDefs::TBrCtlSettings aSettingType );

    protected:

        CBrowserWindowManager(
            MApiProvider& aApiProvider,
            CBrowserContentView& aContentView,
            TInt aMaxWindowCount );
        /**
        * 2nd phase constructor
        */
        void ConstructL();

        // searching for a window identified by its Id and TargetName
        // returns with the Window
        TInt Window(
            TInt aParentId,
            const TDesC* aTargetName,
            CBrowserWindow*& aWindow,
            CBrowserWindowQue* aStartingItem ) const;

        // searching for a window identified by its Id
        // returns with the Queue element of that Window
        TInt Window(
            TInt aWindowId,
            CBrowserWindowQue*& aWindowQue,
            CBrowserWindowQue* aStartingItem ) const;
            
        // really delete a window
        TInt DeleteOneWindowL( TInt aWindowId );
        
        // removes already deleted (status) windows from the queue
        void RemoveDeletedWindowsL();

    private:
        static TInt iWindowIdGenerator;  // for generating window's id
        MApiProvider*        iApiProvider;  // not owned
        CBrowserContentView* iContentView;  // not owned
        TInt iMaxWindowCount;
        TInt iWindowCount;
        TInt iDeletedWindowCount;
        CBrowserWindowQue*   iWindowQueue;  // head of the windows
        CBrowserWindowQue*   iCurrentWindow;  // points to the current window
        TBool iUserExit;
        TBool iIsContentExist;
        CArrayPtrFlat< MWindowObserver >* iObservers;
    };

#include "BrowserWindowManager.inl"

#endif  // __MULTIPLE_WINDOWS_WINDOWMANAGER_H__

// End of File
