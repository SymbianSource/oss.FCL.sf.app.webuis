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



#ifndef BROWSERSPECIALLOADOBSERVER_H
#define BROWSERSPECIALLOADOBSERVER_H

//  INCLUDES
#include <e32base.h>
#include <brctlspecialloadobserver.h>
#include <apparc.h>
#include <badesca.h>
#include <AknServerApp.h>

class MApiProvider;
class CEikDocument;
class CSchemeHandler;
class CDocumentHandler;
class CBrowserContentView;
class CAiwGenericParamList;
class CBrowserWindow;

enum TPopupPreferences
	{
	EBlockOnce = 0,
	EAllowOnce,	
	EBlock,
	EAllow	
	};

// CLASS DECLARATION

/**
*  This observer handles special load events such as network connection, 
*  deal with non-http(s) or non-html requests
*
*  @lib BrowserEngine.lib
*  @since 2.8
*/
class CBrowserSpecialLoadObserver : public CBase, 
                                    public MBrCtlSpecialLoadObserver, 
                                    public MAknServerAppExitObserver
    {
    public: // Constructors and destructors

        /**
        * Constructor.
        */
        static CBrowserSpecialLoadObserver* NewL(
            MApiProvider& aApiProvider,
            CBrowserWindow* aWindow,
            CEikDocument* aDocument,
            CBrowserContentView* aContentView );

        /**
        * Destructor.
        */
        virtual ~CBrowserSpecialLoadObserver();

    private:

#ifdef __WINS__
        TInt AskIapIdL( TUint32& aId );
#endif // __WINS__

    public: // From MBrCtlSpecialLoadObserver

        /**
        * Request to create a network connection.
        * @since 2.8
        * @param aConnectionPtr A pointer to the new connection.
        *        If NULL, the proxy filter will automatically create
        *        a network connection.
        * @param aSockSvrHandle A handle to the socket server.
        * @param aNewConn A flag if a new connection was created.
        *        If the connection is not new, proxy filter optimization 
        *        will not read the proxy again from CommsBd.
        * @return void
        */
        virtual void NetworkConnectionNeededL( TInt* aConnectionPtr,
                                               TInt* aSockSvrHandle,
                                               TBool* aNewConn,
                                               TApBearerType* aBearerType );

        /**
        * Request the host applicaion to handle non-http request.
        * @since 2.8
        * @param aUrl The non-http(s) or file URL
        * @param aParamList Parameters to pass to the host application. Contain referer header. It could be NULL
        * @return ETrue is handled by the host application. EFlase if not
        */
        virtual TBool HandleRequestL( RArray<TUint>* aTypeArray, 
                                      CDesCArrayFlat* aDesArray );

        /**
        * Request the host applicaion to handle downloads
        * @since 2.8
        * @param aTypeArray array of download parameter types
        * @param aDesArray array of values associated with the types in the type array
        * @return ETrue is handled by the host application. EFlase if not
        */
        virtual TBool HandleDownloadL( RArray<TUint>* aTypeArray, 
                                       CDesCArrayFlat* aDesArray );
    

    public:  // from MAknServerAppExitObserver

        void HandleServerAppExit( TInt aReason );

    protected: // Constructors

        /**
        * Default C++ constructor.
        */
        CBrowserSpecialLoadObserver( MApiProvider& aApiProvider,
                                     CBrowserWindow* aWindow,
                                     CEikDocument* aDocument,
                                     CBrowserContentView* aContentView );

        /**
        * 2nd phase constructor. Leaves on failure.
        */
        void ConstructL();

    public: // New functions

        inline TBool LongZeroStartup() 
                                { return iLongZeroStartup; }

        inline void SetLongZeroStartup( TBool aLongZeroStartup )
                                { iLongZeroStartup = aLongZeroStartup; }
        inline TBool IsConnectionStarted()
        						{ return iConnectionStarted; }
        /**
        * Request for the connection to be cancelled
        */
        inline void CancelConnection() { iConnectionCancelled = ETrue; }
                                      

    protected: // New functions

#ifdef  __RSS_FEEDS
        /**
        * Handles the content if it is special Ui content (rss or opml) and returns true, otherwise the method
        * returns false.
        * @param aTypeArray array of download parameter types
        * @param aDesArray array of values associated with the types in the type array
        * @return ETrue if it handled rss content, EFalse otherwise
        * @since 3.1
        */
    	TBool HandleUiContentL(RArray<TUint>* aTypeArray, 
                         		CDesCArrayFlat* aDesArray ) const;
#endif  // __RSS_FEEDS

        /**
        * Make generic parameter list from BrCtl parameter list.
        * @param aTypeArray array of download parameter types
        * @param aDesArray array of values associated with the types in the type array
        * @return The dynamically allocated list.
        * @since 2.8
        */
        CAiwGenericParamList* BrCtlParamList2GenericParamListL( 
                                 RArray<TUint>* aTypeArray, 
                                 CDesCArrayFlat* aDesArray ) const;

        /**
        * Extract the given BrCtl parameter from the list.
        * @param aParamTypeToFind Extract this parameter.
        * @param aTypeArray array of download parameter types.
        * @param aDesArray array of values associated with the types in the type array.
        * @param aParamFound Output: ETrue if the parameter was found.
        * @return A TPtrC pointer for the value.
        * @since 2.8
        */
        TPtrC ExtractBrCtlParam( 
                                 TUint aParamTypeToFind, 
                                 RArray<TUint>* aTypeArray,
                                 CDesCArrayFlat* aDesArray, 
                                 TBool& aParamFound ) const;

        /**
        * Check if the download's content type is in 
        * KBrowserSelfDownloadContentTypes.
        * @param aTypeArray array of download parameter types.
        * @param aDesArray array of values associated with the types in the type array.
        * @return ETrue, if it is there.
        * @since 3.0
        */
        TBool IsSelfDownloadContentTypeL( RArray<TUint>* aTypeArray, 
                                                                CDesCArrayFlat* aDesArray ) const;

        /**
        * Open the download embedded with Document Handler.
        * @param aTypeArray array of download parameter types.
        * @param aDesArray array of values associated with the types in the type array.
        * @return None.
        * @since 3.0
        */
        void OpenFileEmbeddedL( RArray<TUint>* aTypeArray, 
                                                  CDesCArrayFlat* aDesArray );

        /**
        * DoRequest the host applicaion to handle non-http request.
        * @since 2.8
        * @param aUrl The non-http(s) or file URL
        * @param aParamList Parameters to pass to the host application. Contain referer header. It could be NULL
        * @return ETrue is handled by the host application. EFlase if not
        */
        virtual TBool DoHandleRequestL( RArray<TUint>* aTypeArray, 
                                        CDesCArrayFlat* aDesArray );
                                        
    private:  // data members

        MApiProvider *iApiProvider;        // owner app, not owned
        CBrowserWindow *iWindow;           // owner window, not owned
        CEikDocument *iDocument;           // document, not owned
        CSchemeHandler *iSchemeHandler;    // scheme handler
        CDocumentHandler *iDocHandler;     // Owned.
        CBrowserContentView* iContentView; // content view, not owned
        TBool           iSchemeProcessing; // scheme processing
        
        /**
        * Indicated whether the Browser was started using the long zero
        * key press. This is ONLY valid at start up.
        **/
        TBool iLongZeroStartup;        
        
        /**
        * Flag to indicate whether the Browser has started the network connection
        **/
        TBool iConnectionStarted;
        TBool iConnectionCancelled;
    };

#endif      // BROWSERSPECIALLOADOBSERVER_H

// End of File
