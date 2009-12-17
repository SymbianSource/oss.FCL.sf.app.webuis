/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Handle special load events such as network connection,
*                deal with non-http or non-html requests
*
*/



#ifndef BROWSERLOADOBSERVER_H
#define BROWSERLOADOBSERVER_H

//  INCLUDES
#include <brctlinterface.h>
#include <browserdialogsproviderobserver.h>

//  FORWARD DECLARATIONS
class MApiProvider;
class CBrowserContentView;
class CBrowserWindow;

// define this macro to log event time, temporary
// #define I__LOG_EVENT_TIME

//  CLASS DECLARATION

/**
*  This observer registers for load events
*/
class CBrowserLoadObserver : public CBase,
                             public MBrCtlLoadEventObserver,
                             public MBrowserDialogsProviderObserver
    {
    public: // Constructors and destructors

        /**
        * Constructor.
        */
        static CBrowserLoadObserver* NewL( MApiProvider& aApiProvider,
                                           CBrowserContentView& aContentView,
                                           CBrowserWindow& aWindow );

        /**
        * Destructor.
        */
        virtual ~CBrowserLoadObserver();

    public:

        enum TBrowserLoadState
            {
            ELoadStateIdle = 0x01,
            // ELoadStateContentStart is not used.
            // If you want to set it from BrowserUi, please note
            // that it's not guarantued that Kimono will send
            // any load event in case the content is already moved to 
            // Download Manager!
            ELoadStateContentStart = 0x02,
            ELoadStateResponseInProgress = 0x04,
            ELoadStateLoadDone = 0x08,
            ELoadStateContentAvailable = 0x10
            };

        enum TBrowserLoadUrlType
            {
            ELoadUrlTypeSavedDeck = 0x01,
            ELoadUrlTypeStartPage,
            ELoadUrlTypeEmbeddedBrowserWithUrl,
            ELoadUrlTypeOther  // for example GotoPane's url, Bookmark
            };

        enum TBrowserLoadStatus
            {
            // these flags are the status of loading a page,
            // can be OR'd for querying, setting
            ELoadStatusFirstChunkArrived = 0x0001 ,
            ELoadStatusContentDisplayed = 0x0002 ,
            ELoadStatusMainError = 0x0004 ,
            ELoadStatusSubError = 0x0008 ,
            ELoadStatusError = ELoadStatusMainError | ELoadStatusSubError,
                // 0x000C , // MainError or SubError
            ELoadStatusTitleAvailable = 0x0010,
            ELoadStatusFirstChunkDisplayed = 0x0020,
            ELoadStatusDone = 0x0040,
            ELoadStatusSecurePage = 0x0080,
            ELoadStatusSecureItemNonSecurePage = 0x0100,
            ELoadStatusAllItemIsSecure = 0x0200,  // !SomeItemsNotSecure
            ELoadStatusSecurePageVisited = 0x0400
            };

    public: // Functions from MBrCtlLoadEventObserver

        /**
        * A load events notification
        * @since 2.8
        * @param aLoadEvent The load event
        * @param aSize Size depends on the event
        * @param aTransactionId The transaction that had this event
        * @return void
        */
        virtual void HandleBrowserLoadEventL(
            TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
            TUint aSize,
            TUint16 aTransactionId );

    public: // Functions from MBrowserDialogsProviderObserver

        /**
        * Callback function for dialog events.
        */
        void ReportDialogEventL( TInt aType, TInt aFlags = 0 );

    public:  // new functions

        /**
        * Loading must be started with this function by the owner.
        * @since 3.0
        * @param aLoadUrlType What is the url's type?
        *   if url doesn't need to be handled a special way, use ELoadUrlTypeOther
        *   otherwise implement special handling for all the events.
        * @return void
        */
        void DoStartLoad( TBrowserLoadUrlType aLoadUrlType );

        /**
        * Loading must be terminated with this function by the owner.
        * @since 3.0
        * @param aIsCancelled Is user cancelled the fetching operation?
        */
        void DoEndLoad( TBool aIsUserInitiated = EFalse );

        /**
        * Checks loading status against some flags.
        * @param aFlags 
        * @return ETrue only if ALL the aFlags are set
        */
        inline TBool LoadStatus( TUint32 aFlag ) const
            { return ( ( aFlag ) && (( iStatus & ( aFlag )) == ( aFlag ))); }

    protected: // new functions

        /**
        * Default C++ constructor.
        */
        CBrowserLoadObserver( MApiProvider& aApiProvider,
                              CBrowserContentView& aContentView,
                              CBrowserWindow& aWindow );

        /**
        * 2nd phase constructor. Leaves on failure.
        */
        void ConstructL();

    private:  // new functions

        void HandleLoadEventOtherL(
            TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
            TUint aSize,
            TUint16 aTransactionId );

        inline void SetStatus( TUint32 aFlag )   { iStatus |= aFlag; }
        inline void ClearStatus( TUint32 aFlag ) { iStatus &= ~aFlag; }
        inline void ClearStatus( )               { iStatus = 0; }

        void UpdateSecureIndicatorL();

    private:
        void StateChange( TBrowserLoadState aNextState );
        void NewTitleAvailableL();
        TBool InBrowserContentView();
        void ContentArrivedL();
        inline void SetRestoreContentFlag( TBool aFlag )
            { iRestoreContentFlag = aFlag; }

    public:
        // Get functions
        TBrowserLoadUrlType LoadUrlType() const;
        TBrowserLoadState   LoadState()   const;
        inline TBool ContentDisplayed(){return  iNewContentDisplayed;}
        void SetContentDisplayed( TBool aValue );

    private:  // data members
        MApiProvider        *iApiProvider;  // owner app, not owned
        CBrowserContentView *iContentView;  // contentView, not owned
        CBrowserWindow      *iWindow;
        TBrowserLoadState   iLoadState;
        TBrowserLoadUrlType iLoadUrlType;
        TUint32             iStatus;

        TBool               iRestoreContentFlag;
        TBool               iBrowserInitLoad;
        TBool               iNewContentDisplayed;
        // Large file upload max content
        TUint32             iMaxUploadContent;
        
#ifdef I__LOG_EVENT_TIME
        TTime               iStartDownloadTime;
        TInt64              iTotalUpdateTime;
        TInt64              iNumberOfUpdates;
#endif

    };

#endif      // BROWSERLOADOBSERVER_H

// End of File
