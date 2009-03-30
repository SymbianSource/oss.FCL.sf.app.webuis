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


// INCLUDE FILES
#include "BrowserContentView.h"  // not used, to be removed
#include "BrowserSpecialLoadObserver.h"
#include "BrowserLoadObserver.h"
#include "logger.h"
#include "BrowserAppDocument.h"
#include "ApiProvider.h"
#include "Preferences.h"
#include "BrowserUIVariant.hrh"
#include "Display.h"
#include "BrowserAppUi.h"
#include "BrowserWindow.h"
#include "BrowserWindowManager.h"
#include "CommonConstants.h"
#ifdef __WINS__
    #include <commdb.h>
    #include <aknselectionlist.h>
    #include <AknLists.h>
#endif // __WINS__
#include <DocumentHandler.h>
#include <MConnection.h>  // for MConnection
#include <schemehandler.h>
#include <BrowserNG.rsg>
#include <FeatMgr.h>
#include <AiwGenericParam.h>

//CONSTANTS
const TInt KMaxDocHandlerUrlLen = 255;
//DestNetChange
#include <CmApplicationSettingsUi.h>


// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::NewL()
// ---------------------------------------------------------
//
CBrowserSpecialLoadObserver* CBrowserSpecialLoadObserver::NewL(
        MApiProvider& aApiProvider,
        CBrowserWindow* aWindow,
        CEikDocument* aDocument,
        CBrowserContentView* aContentView )
    {
    CBrowserSpecialLoadObserver* self = 
        new (ELeave) CBrowserSpecialLoadObserver(
            aApiProvider, aWindow, aDocument, aContentView );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop();  // self
    return self;
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::~CBrowserSpecialLoadObserver()
// ---------------------------------------------------------
//
CBrowserSpecialLoadObserver::~CBrowserSpecialLoadObserver()
    {
    delete iSchemeHandler;
    delete iDocHandler;
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::CBrowserSpecialLoadObserver()
// ---------------------------------------------------------
//
CBrowserSpecialLoadObserver::CBrowserSpecialLoadObserver(
        MApiProvider& aApiProvider,
        CBrowserWindow* aWindow,
        CEikDocument* aDocument,
        CBrowserContentView* aContentView ) : 
    iApiProvider( &aApiProvider ),
    iWindow( aWindow ),
    iDocument( aDocument ),
    iSchemeHandler( NULL ),
    iContentView( aContentView ),
    iSchemeProcessing( EFalse ),
    iLongZeroStartup( EFalse ),
    iConnectionStarted( EFalse )
    {
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::ConstructL()
// ---------------------------------------------------------
//
void CBrowserSpecialLoadObserver::ConstructL()
    {
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::NetworkConnectionNeededL()
// ---------------------------------------------------------
//
void CBrowserSpecialLoadObserver::NetworkConnectionNeededL(
        TInt* aConnectionPtr,
        TInt* aSockSvrHandle,
        TBool* aNewConn,
        TApBearerType* aBearerType )
    {
LOG_ENTERFN("CBrowserSpecialLoadObserver::NetworkConnectionNeededL");
	
	__ASSERT_DEBUG( (aConnectionPtr != NULL), Util::Panic( Util::EUninitializedData ) );
	__ASSERT_DEBUG( (aSockSvrHandle != NULL), Util::Panic( Util::EUninitializedData ) );
	__ASSERT_DEBUG( (aNewConn != NULL), Util::Panic( Util::EUninitializedData ) );
	__ASSERT_DEBUG( (aBearerType != NULL), Util::Panic( Util::EUninitializedData ) );
	
    iApiProvider->SetProgressShown( ETrue );
    TBool query( EFalse );
    if( iApiProvider->Connection().Connected() )
        {
        TInt a( iWindow->WindowId() );
		BROWSER_LOG( ( _L( "already connected, WindowId: %d" ), a ) );
        *aConnectionPtr = (TInt) &iApiProvider->Connection().Connection();
        *aSockSvrHandle = iApiProvider->Connection().SocketServer().Handle();
        *aNewConn = EFalse;
        *aBearerType = iApiProvider->Connection().CurrentBearerTypeL();
        }
    else
        {
		BROWSER_LOG((_L("not connected.")));
        *aNewConn = ETrue;
        TBool isValid( EFalse );
        TUint32 ap( 0 );
        TInt err( KErrNone );

		BROWSER_LOG((_L("AP selectionMode: %d."),
    	iApiProvider->Preferences().AccessPointSelectionMode() ));

       //determine the type of connection
        switch (iApiProvider->Preferences().AccessPointSelectionMode() )
        	{
        	case EAlwaysAsk:
        		{
	            BROWSER_LOG((_L("always ask.")));
	            // Always ask
	            isValid = ETrue;
	        	break;	
        		}
        		
        	case EDestination:
        		{
                isValid = ETrue;  // force to show LAN connection dialog

#ifdef __WINS__
                query = ETrue;  // disable connection query dialog
#endif  // __WINS__
                   
                // for bug fix MLAN-7EKFV4, if bookmark has defined its AP, use it
                if ( iApiProvider->RequestedAp() != KWmlNoDefaultAccessPoint )
                    {
                    #ifndef __WINS__
                                        ap = Util::IapIdFromWapIdL(
                                            *iApiProvider, iApiProvider->RequestedAp() );
				                        BROWSER_LOG((_L("ap: %d"), ap));
                    #else  // __WINS__
                                        // On WINS no need to convert the AP ID
                                        ap = iApiProvider->RequestedAp();
                    #endif // __WINS__

                    iApiProvider->Connection().SetRequestedAP( ap );
                    iApiProvider->Connection().SetConnectionType( EConnectionMethod );
                    }
                else
                    {
                    TUint32 snapId = iApiProvider->Preferences().DefaultSnapId();
                    //connection type is destination nw 
                    iApiProvider->Connection().SetConnectionType( EDestination );
                    //set snap id
                    iApiProvider->Connection().SetRequestedSnap( snapId );        		        		
                    }
                break;	
        		}
        		
        	case EConnectionMethod:
        		{
        		BROWSER_LOG((_L("user defined.")));
                // User defined

                TRAP( err, isValid = Util::CheckApValidityL( *iApiProvider, 
                    iApiProvider->RequestedAp() ));
			    BROWSER_LOG((_L("isValid 1: %d"), isValid));
			    BROWSER_LOG((_L("err: %d"), err));
                if( isValid )
                    {  // valid AP, get it
#ifndef __WINS__
                    ap = Util::IapIdFromWapIdL(
                        *iApiProvider, iApiProvider->RequestedAp() );
				    BROWSER_LOG((_L("ap: %d"), ap));
#else  // __WINS__
                    // On WINS no need to convert the AP ID
                    ap = iApiProvider->RequestedAp();
#endif // __WINS__
                    }
                else if( err == KErrNone )
                    {  // still try to get AP
                    TUint apUid = iApiProvider->Preferences().DefaultAccessPoint();
    				
				    BROWSER_LOG((_L("apUid: %d"), apUid));
                    
                    if ( apUid == KWmlNoDefaultAccessPoint )
                        {
#ifdef __WINS__
                        query = ETrue;  // disable connection query dialog
                        isValid = ETrue;  // force to show LAN connection dialog
#else  // __WINS__
						BROWSER_LOG((_L("StartPrefView.")));
                        //  there are no AP, show preferences view without always ask
                        isValid  = iApiProvider->StartPreferencesViewL( EShowDestinations | EShowConnectionMethods );
                        if( isValid )
                            {
                            if( iApiProvider->Preferences().AccessPointSelectionMode() == EConnectionMethod )
                                {
                       		    ap = iApiProvider->Preferences().DefaultAccessPoint();
                                }
                            // else if EDestination, user choosed SNAP in preference view
                            }
#endif  // __WINS__
                        }
                    }

                //set connection type and AP or SNAP Id                     
                if( iApiProvider->Preferences().AccessPointSelectionMode() == EDestination )
                    {
                    TUint32 snapId = iApiProvider->Preferences().DefaultSnapId();
                    iApiProvider->Connection().SetConnectionType( EDestination );
                    iApiProvider->Connection().SetRequestedSnap( snapId );        		        		
                    }
                else // EConnectionMethod
                    {
                    iApiProvider->Connection().SetRequestedAP( ap );
                    iApiProvider->Connection().SetConnectionType( EConnectionMethod );
                    }
        		break;	
        		}
 
        	default:
        		{
        		BROWSER_LOG((_L("wrong type of connection")));	
        		}
        	} //switch	
 		
		BROWSER_LOG((_L("isValid 2: %d"), isValid));
        if( isValid )
            {
            iConnectionCancelled = EFalse;
            if( !iApiProvider->Connection().Connected() )
                {
                iApiProvider->Connection().SetRequestedAP( ap );

                TBool startPage = iWindow->LoadObserver().LoadUrlType() == 
                                    CBrowserLoadObserver::ELoadUrlTypeStartPage;
                TBool noConnDlgs = !iApiProvider->Preferences().ShowConnectionDialogs();

                // Disable connection dialogs if...
                TBool disableConnDlgs = 
                        query ||
                        startPage ||    // ...the load type is StartPage
                        noConnDlgs ||   // ...connection dialogs setting = OFF
                        LongZeroStartup();  // ...Browser started with long zero keypress
                SetLongZeroStartup( EFalse );

				BROWSER_LOG((_L("StartConnection params, query: %d, temp: %d"), query, disableConnDlgs));
                iConnectionStarted = ETrue; // is true when connection request submited.
                // might leave, don't TRAP. OK.
                err = iApiProvider->Connection().StartConnectionL( disableConnDlgs );
                iConnectionStarted = EFalse; // is false when connection response completed.
                }
                
            // during StartConnectionL user pressing "Stop" can cancel
            if (iConnectionCancelled)
                {
                err = KErrCancel;
                }
            BROWSER_LOG((_L("err after StartConnection: %d"), err));
            
            if( err == KErrNone )
                {
                *aConnectionPtr = (TInt) &iApiProvider->Connection().Connection();
                *aSockSvrHandle = iApiProvider->Connection().SocketServer().Handle();
                *aBearerType = iApiProvider->Connection().CurrentBearerTypeL();
                }
            
            CBrowserAppUi::Static()->ConnNeededStatusL( err );
            if (( err == KErrCancel ) && !iApiProvider->IsPageLoaded())
                {                
	          	// Close Browser's Content View window if connection setup is cancelled to avoid a blank white screen
                //  ** Don't do this is page was previously loaded and is visible to user.
				// ALSO, if feeds are enabled, don't close the window if coming from feeds folder view
				// or in topic or feed views

				// *** Changing back to CloseContentView as a part of the new Browser exit functionality				   
#ifdef __RSS_FEEDS
 				TUid previousViewID = iApiProvider->LastActiveViewId();
                if( (previousViewID != KUidBrowserFeedsFolderViewId) &&
                	(previousViewID != KUidBrowserFeedsTopicViewId) &&
                	(previousViewID != KUidBrowserFeedsFeedViewId) )
                	{
        	        CBrowserAppUi::Static()->CloseContentViewL();
                	}
#else //__RSS_FEEDS           
                CBrowserAppUi::Static()->CloseContentViewL();
#endif //__RSS_FEEDS
                }
            else if( err == KErrGeneral )
                {
                if( !iApiProvider->Preferences().HttpSecurityWarningsStatSupressed() )
                    {
                    iWindow->Display().UpdateSecureIndicatorL(
                        EAknIndicatorStateOff );
                    }
                }
            // user might cancel, pass result to BrowserEngine
            User::LeaveIfError( err );
            iApiProvider->Display().StartProgressAnimationL();
            }
        else
            {
            User::LeaveIfError( err );
            }
        }
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::DoHandleRequestL()
// ---------------------------------------------------------
//
TBool CBrowserSpecialLoadObserver::DoHandleRequestL
    ( RArray<TUint>* aTypeArray, CDesCArrayFlat* aDesArray )
    {
    LOG_ENTERFN("CBrowserSpecialLoadObserver::HandleRequestL");

    if( !iSchemeProcessing )
        {
        iSchemeProcessing = ETrue;
        TBool paramFound( EFalse );
        TPtrC requestUrl = ExtractBrCtlParam( 
            EParamRequestUrl, 
            aTypeArray,
            aDesArray, 
            paramFound );
        __ASSERT_DEBUG( paramFound, Util::Panic( Util::EUninitializedData ) );
        BROWSER_LOG( ( _L( " EParamRequestUrl: %S" ), &requestUrl ) );

        // pass to the CSchemaHandler             
        CSchemeHandler* newSH = CSchemeHandler::NewL( requestUrl );
        delete iSchemeHandler;
        iSchemeHandler = newSH;
        BROWSER_LOG( ( _L( " iSchemeHandler OK" ) ) );

        // construct the current accesspoint
        // if it's connected, always add it 
        if ( iApiProvider->Connection().Connected() )
            {
            CAiwGenericParamList* paramsList = CAiwGenericParamList::NewL();
            CleanupStack::PushL(paramsList);
        	TInt32 ap = iApiProvider->Connection().CurrentAPId(); 
        	TAiwVariant paramVariant (ap);
        	TAiwGenericParam genericParam( EGenericParamAccessPoint, paramVariant );
        	paramsList->AppendL (genericParam );
            
        	
		    // set the parameter
     	    iSchemeHandler->SetParameterList( paramsList );
     	
         	CleanupStack::Pop();
            }

        iSchemeHandler->Observer( this );
        iSchemeHandler->HandleUrlEmbeddedL();
        }
    return ETrue;
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::HandleRequestL()
// ---------------------------------------------------------
//
TBool CBrowserSpecialLoadObserver::HandleRequestL
    ( RArray<TUint>* aTypeArray, CDesCArrayFlat* aDesArray )
    {
    TBool ret( ETrue );
    TRAPD( err, DoHandleRequestL( aTypeArray, aDesArray ) );
    if( KErrNone != err )
        {
        iSchemeProcessing = EFalse;
        User::Leave( err );
        }
    return ret;
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::HandleDownloadL()
// ---------------------------------------------------------
//
TBool CBrowserSpecialLoadObserver::HandleDownloadL
    ( RArray<TUint>* aTypeArray, CDesCArrayFlat* aDesArray )
    {
    LOG_ENTERFN("CBrowserSpecialLoadObserver::HandleDownloadL");

    TBool downloadHandled = EFalse;

#ifdef __RSS_FEEDS
	// Check to see if this is content to be handled by UI (rss or opml)
	//
    if (HandleUiContentL(aTypeArray, aDesArray))
    	{
    	return ETrue;
    	}
#endif  // __RSS_FEEDS

    CBrowserAppDocument* appDoc = STATIC_CAST( CBrowserAppDocument*, iDocument );
    
    __ASSERT_DEBUG( (appDoc != NULL), Util::Panic( Util::EUninitializedData ) );
    
    if ( appDoc->IsContentHandlerRegistered() )
        {
        BROWSER_LOG( ( _L( "Calling file handler." ) ) );

        TBool paramFound( EFalse );
        TPtrC fileName = ExtractBrCtlParam( 
                            EParamLocalFileName, 
                            aTypeArray,
                            aDesArray, 
                            paramFound );
        __ASSERT_DEBUG( paramFound, Util::Panic( Util::EUninitializedData ) );

        if ( fileName.Length() == 0 )
            {
            // May be a Rights Object download - 
            // no downloaded file, nothing to open/handle/pass to BrowserLauncher client.
            BROWSER_LOG( ( _L( " Empty fileName" ) ) );
            }
        else
            {
            CAiwGenericParamList* genericParamList = 
                BrCtlParamList2GenericParamListL( aTypeArray, aDesArray );
            CleanupStack::PushL( genericParamList );

            // Forward the file to the registered handler.
            TBool continueBrowsing( EFalse );
            downloadHandled = CBrowserAppUi::Static()->HandleContentL
                             ( fileName, *genericParamList, continueBrowsing );
            BROWSER_LOG( ( _L( " downloadHandled: %d, continueBrowsing: %d" ),
                downloadHandled, continueBrowsing ) );

            CleanupStack::PopAndDestroy( genericParamList ); // genericParamList

            if ( downloadHandled )
                {
                // We can delete the file now.
                RFs fs;
                User::LeaveIfError( fs.Connect() );
                CleanupClosePushL( fs );
                fs.Delete( fileName ); // Return value is ignored.
                CleanupStack::PopAndDestroy( &fs ); // fs
                }

            if ( !continueBrowsing )
                {
                iApiProvider->SetExitFromEmbeddedMode( ETrue );
                }
            }
        }
    else
        {
        // No handler registered.
        BROWSER_LOG( ( _L( " No handler registered." ) ) );
        }

    // If the download (file) was not handled, 
    //   the BrowserUI handles it with Document Handler.
    if ( !downloadHandled )
        {
        // This content must be handled by BrowserUI - Browser Control cannot handle it.
        OpenFileEmbeddedL( aTypeArray, aDesArray );
        downloadHandled = ETrue;
        }
        
    BROWSER_LOG( ( _L( " downloadHandled: [%d]" ), (TInt)downloadHandled ) );
    return downloadHandled;
    }

#ifdef __RSS_FEEDS
// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::HandleUiContentL()
// ---------------------------------------------------------
//
TBool CBrowserSpecialLoadObserver::HandleUiContentL( 
                         RArray<TUint>* aTypeArray, 
                         CDesCArrayFlat* aDesArray ) const
    {
    TDataType dataType;

    TBool contentTypeFound( EFalse );
    TBool urlFound( EFalse );
    TBool fileFound( EFalse );
    TBool result( EFalse );
    
    TPtrC receivedContentType = ExtractBrCtlParam( 
        EParamReceivedContentType, 
        aTypeArray,
        aDesArray, 
        contentTypeFound );
        
    TPtrC url = ExtractBrCtlParam( 
        EParamRequestUrl, 
        aTypeArray,
        aDesArray, 
        urlFound );
      
        
    TPtrC fileName = ExtractBrCtlParam( 
    	EParamLocalFileName, 
        aTypeArray,
        aDesArray, 
        fileFound );
    
    // Compare returns 0 if the strings are the same
    if ((receivedContentType.Compare(KMimeTypeAppXML) == 0) || 
          (receivedContentType.Compare(KMimeTypeTextXML) == 0) ||
          (receivedContentType.Compare(KMimeTypeRSS) == 0) ||
          (receivedContentType.Compare(KMimeTypeAtom) == 0) ||
          (receivedContentType.Compare(KMimeTypeRDF) == 0) ||
          (receivedContentType.Compare(KOPMLMimeType) == 0) ||
          //++PK added check for Text/xml content-type, only T is in caps, should make string check case insensitive
          (receivedContentType.Compare(KMimeTypeCapsTextXML) == 0)
          //++PK
          )
    	{
    	if (fileFound)
    		{
    		RFs             rfs;
    		RFile           rfile;
    	
    		// Open the file.
    		User::LeaveIfError(rfs.Connect());
    		CleanupClosePushL(rfs);

			TInt fileResult( rfile.Open( rfs, fileName, EFileRead ) );
    		// file open doesn't succeeded
    		User::LeaveIfError( fileResult );
    		if( fileResult == KErrNone )
        		{
        		CleanupClosePushL( rfile );
    		
    			TBuf<KMaxDataTypeLength> dataTypeDes;
                TBool   deleteOpml = EFalse;
    		
    			if(CBrowserAppUi::Static()->RecognizeUiFileL(rfile, dataTypeDes))
    				{
					TFileName fileNameBuf = TFileName(fileName);
					TFileName truncFileName;
					User::LeaveIfError( rfile.Name(truncFileName) );    				
    				    				
    				// Confirm with user that we want to handle it
    				if(CBrowserAppUi::Static()->ConfirmUiHandlingL(truncFileName, dataTypeDes))
    					{  					
						// Have AppUI decide what to do with it, what view, etc.
    					CBrowserAppUi::Static()->HandleUiDownloadL(fileNameBuf, dataTypeDes);
    					}
                    else
                        {
                        deleteOpml = ETrue;
                        }
    				result = ETrue;
       				}
       			CleanupStack::PopAndDestroy(); //rfile
       			
                // delete the downloaded file if it is not handled
                if(deleteOpml && (dataTypeDes.Compare(KOPMLMimeType) == 0 ))
                    {
                    rfs.Delete(fileName);
                    }       			
        		}
        	CleanupStack::PopAndDestroy(); //rfs        	
    		}
    		
    	// If open file was handled, skip sending to feeds engine
    	// otherwise, pass it on
    	if(!result)
    		{
    		// Launch the Feeds Engine
			iContentView->HandleSubscribeToWithUrlL(url);
			result = ETrue;	
    		}
    	}
    
    return result;
    }
     
#endif  // __RSS_FEEDS


// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::BrCtlParamList2GenericParamListL()
// ---------------------------------------------------------
//
CAiwGenericParamList* CBrowserSpecialLoadObserver::BrCtlParamList2GenericParamListL( 
                         RArray<TUint>* aTypeArray, 
                         CDesCArrayFlat* aDesArray ) const
    {
    LOG_ENTERFN("CBrowserSpecialLoadObserver::BrCtlParamList2GenericParamListL");

	__ASSERT_DEBUG( (aTypeArray != NULL), Util::Panic( Util::EUninitializedData ) );
	__ASSERT_DEBUG( (aDesArray != NULL), Util::Panic( Util::EUninitializedData ) );


    //Creating generic param list
    CAiwGenericParamList* genericParamList = CAiwGenericParamList::NewLC();

    for( TInt j = 0; j < aTypeArray->Count(); j++ )
        {
        BROWSER_LOG( ( _L( " iter: %d" ), j ) );
        TBool paramFound( EFalse );
        const TUint paramType = (*aTypeArray)[j];
        TPtrC param = ExtractBrCtlParam( 
                                 paramType, 
                                 aTypeArray,
                                 aDesArray, 
                                 paramFound );
        __ASSERT_DEBUG( paramFound, Util::Panic( Util::EUninitializedData ) );

        BROWSER_LOG( ( _L( " Type:  %d, Value: %S" ), paramType, &param ) );

        switch( paramType )
            {
            case EParamRequestUrl:
                {
                BROWSER_LOG( ( _L( " EParamRequestUrl" ) ) );
                // limit url to 255 char's or DocHandler panics!
                TAiwVariant paramVariant( param.Left( KMaxDocHandlerUrlLen ) );
                TAiwGenericParam genericParam( EGenericParamURL, 
                                               paramVariant );
                genericParamList->AppendL( genericParam );
                break;
                }
            case EParamRealm:
                {
                BROWSER_LOG( ( _L( " EParamRealm" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamUsername:
                {
                BROWSER_LOG( ( _L( " EParamUsername" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamPassword:
                {
                BROWSER_LOG( ( _L( " EParamPassword" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamProxyUsername:
                {
                BROWSER_LOG( ( _L( " EParamProxyUsername" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamProxyPassword:
                {
                BROWSER_LOG( ( _L( " EParamProxyPassword" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamRawRequestHeader:
                {
                BROWSER_LOG( ( _L( " EParamRawRequestHeader" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamReceivedContentType:
                {
                BROWSER_LOG( ( _L( " EParamReceivedContentType" ) ) );
                // The expected content type should override the response header's
                // content type => don't use EParamReceivedContentType if 
                // EParamExpectedContentType is set!
                TBool expectedContentTypeFound( EFalse );
                ExtractBrCtlParam( 
                                                 EParamExpectedContentType, 
                                                 aTypeArray,
                                                 aDesArray, 
                                                 expectedContentTypeFound );
                if ( !expectedContentTypeFound )
                    {
                    // Using EParamReceivedContentType
                    BROWSER_LOG( ( _L( " EParamExpectedContentType not found" ) ) );
                    TAiwVariant paramVariant( param );
                    TAiwGenericParam genericParam( EGenericParamMIMEType, 
                                                   paramVariant );
                    genericParamList->AppendL( genericParam );
                    }
                break;
                }
            case EParamExpectedContentType:
                {
                // Overrides EParamReceivedContentType!
                BROWSER_LOG( ( _L( " EParamExpectedContentType" ) ) );
                TAiwVariant paramVariant( param );
                TAiwGenericParam genericParam( EGenericParamMIMEType, 
                                               paramVariant );
                genericParamList->AppendL( genericParam );
                break;
                }
            case EParamTotalContentLength:
                {
                BROWSER_LOG( ( _L( " EParamTotalContentLength" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamReceivedContent:
                {
                BROWSER_LOG( ( _L( " EParamReceivedContent" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamRawResponseHeader:
                {
                BROWSER_LOG( ( _L( " EParamRawResponseHeader" ) ) );
                // No corresponding generic param!
                break;
                }
            case EParamLocalFileName:
                {
                BROWSER_LOG( ( _L( " EParamLocalFileName" ) ) );
                TAiwVariant paramVariant( param );
                TAiwGenericParam genericParam( EGenericParamFile, 
                                               paramVariant );
                genericParamList->AppendL( genericParam );
                break;
                }
            case EParamCharset:
                {
                BROWSER_LOG( ( _L( " EParamCharset" ) ) );
                TAiwVariant paramVariant( param );
                TAiwGenericParam genericParam( EGenericParamCharSet, 
                                               paramVariant );
                genericParamList->AppendL( genericParam );
                break;
                }
            case EParamRefererHeader:
                {
                BROWSER_LOG( ( _L( " EParamRefererHeader" ) ) );
                TAiwVariant paramVariant( param );
                TAiwGenericParam genericParam( EGenericParamReferringURI, 
                                               paramVariant );
                genericParamList->AppendL( genericParam );
                break;
                }
            default:
                {
                BROWSER_LOG( ( _L( " Unexpected argument" ) ) );
                break;
                }
            }
        }

    // Always add the access point, if currently connected
    if( iApiProvider->Connection().Connected() )
        {
        TInt32 ap = iApiProvider->Connection().CurrentAPId();
        BROWSER_LOG( ( _L( " IAP: %d" ), ap ) );
        TAiwVariant paramVariant( ap );
        TAiwGenericParam genericParam( EGenericParamAccessPoint, 
                                       paramVariant );
        genericParamList->AppendL( genericParam );
        }

    CleanupStack::Pop( genericParamList ); // genericParamList
    return genericParamList;
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::ExtractBrCtlParam()
// ---------------------------------------------------------
//
TPtrC CBrowserSpecialLoadObserver::ExtractBrCtlParam
    ( TUint aParamTypeToFind,
      RArray<TUint>* aTypeArray, 
      CDesCArrayFlat* aDesArray, 
      TBool& aParamFound ) const
    {
    LOG_ENTERFN("CBrowserSpecialLoadObserver::ExtractBrCtlParam");

	__ASSERT_DEBUG( (aTypeArray != NULL), Util::Panic( Util::EUninitializedData ) );
	__ASSERT_DEBUG( (aDesArray != NULL), Util::Panic( Util::EUninitializedData ) );

    // initialize output parameter
    aParamFound = EFalse;
    TPtrC16 retParamValue;

    for( TInt j = 0; j < aTypeArray->Count(); j++ )
        {
        BROWSER_LOG( ( _L( " iter: %d" ), j ) );
        const TUint paramType = (*aTypeArray)[j];
        if ( aParamTypeToFind == paramType )
            {
            // That's we need
            retParamValue.Set( aDesArray->MdcaPoint(j) );
            aParamFound = ETrue; // Indicate it in the out param
            BROWSER_LOG( ( _L( " Type:  %d, Value: %S" ), paramType, &retParamValue ) );
            break; // break the loop - we found it
            }
        }

    return retParamValue;
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::IsSelfDownloadContentTypeL()
// ---------------------------------------------------------
//
TBool CBrowserSpecialLoadObserver::IsSelfDownloadContentTypeL
    ( RArray<TUint>* aTypeArray, 
      CDesCArrayFlat* aDesArray ) const
    {
    LOG_ENTERFN("CBrowserSpecialLoadObserver::IsSelfDownloadContentTypeL");

    TBool isSelfDownloadContentType = EFalse;
    
    // First get the download's content type (mime type)
    TBool paramFound( EFalse );
    TPtrC contentType = ExtractBrCtlParam( 
                        EParamReceivedContentType, 
                        aTypeArray,
                        aDesArray, 
                        paramFound );
                        
    if ( !paramFound )
        {
        isSelfDownloadContentType = EFalse;
        }
    else
        {
        BROWSER_LOG( ( _L( " contentType: [%S]" ), &contentType ) );
        // Then check, if it is in SelfDownloadContentTypes
        TInt cTLength = contentType.Length();
        TPtrC selfDownloadCTs = iApiProvider->Preferences().SelfDownloadContentTypesL();
        TInt idx = KErrNotFound;
        idx = selfDownloadCTs.FindF( contentType );
        const TChar KBrowserSpecLoadObsSemicolon = ';';
        while( idx != KErrNotFound )
            {
            // check for ';' on the left KSemicolon
            if ( idx == 0 || selfDownloadCTs[ idx - 1 ] == KBrowserSpecLoadObsSemicolon )
                {
                // check for ';' on the right
                idx += cTLength;
                if ( idx == selfDownloadCTs.Length() || selfDownloadCTs[ idx ] == KBrowserSpecLoadObsSemicolon )
                    {
                    isSelfDownloadContentType = ETrue;
                    break;
                    }
                }
            selfDownloadCTs.Set( selfDownloadCTs.Mid( idx ) );
            idx = selfDownloadCTs.FindF( contentType );
            }
        }
        
    BROWSER_LOG( ( _L( " isSelfDownloadContentType: %d" ), isSelfDownloadContentType ) );
    return isSelfDownloadContentType;
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::OpenFileEmbeddedL()
// ---------------------------------------------------------
//
void CBrowserSpecialLoadObserver::OpenFileEmbeddedL
    ( RArray<TUint>* aTypeArray, 
      CDesCArrayFlat* aDesArray )
    {
    LOG_ENTERFN("CBrowserSpecialLoadObserver::OpenFileEmbeddedL");

    CDocumentHandler* newDocHandler = CDocumentHandler::NewL();
    delete iDocHandler;
    iDocHandler = newDocHandler;
    iDocHandler->SetExitObserver( this );

    TBool paramFound( EFalse );
    TPtrC fileName = ExtractBrCtlParam( 
                        EParamLocalFileName, 
                        aTypeArray,
                        aDesArray, 
                        paramFound );
    __ASSERT_DEBUG( paramFound, Util::Panic( Util::EUninitializedData ) );

    TPtrC contentType = ExtractBrCtlParam( 
                        EParamReceivedContentType, 
                        aTypeArray,
                        aDesArray, 
                        paramFound );
    HBufC8* contentType8 = 0;
    if ( !paramFound )
        {
        contentType8 = KNullDesC8().AllocLC();
        }
    else
        {
        BROWSER_LOG( ( _L( " contentType: [%S]" ), &contentType ) );
        contentType8 = HBufC8::NewLC( contentType.Length() );
        // 16 bit buffer copied into 8 bit buffer.
        contentType8->Des().Copy( contentType );
        }
    TDataType dataType( *contentType8 );

    CAiwGenericParamList* genericParamList = 
        BrCtlParamList2GenericParamListL( aTypeArray, aDesArray );
    CleanupStack::PushL( genericParamList );

    // File is not yet saved to target folder.
    TBool allowSave( ETrue );
    TAiwVariant allowSaveVariant( allowSave );
    TAiwGenericParam genericParamAllowSave
                     ( EGenericParamAllowSave, allowSaveVariant );
    genericParamList->AppendL( genericParamAllowSave );
    
    // Set EGenericParamAllowMove - we encourage handlers to apply 
    // move instead of copy for the file.
    TBool allowMove( ETrue );
    TAiwVariant allowMoveVariant( allowMove );
    TAiwGenericParam genericParamAllowMove
                     ( EGenericParamAllowMove, allowMoveVariant );
    genericParamList->AppendL( genericParamAllowMove );

    RFile tempFile; 
    iDocHandler->OpenTempFileL( fileName, tempFile );
    CleanupClosePushL( tempFile );
    /*TInt err = */iDocHandler->OpenFileEmbeddedL( tempFile, 
                                          dataType, 
                                          *genericParamList );
    //BROWSER_LOG( ( _L( " err: %d" ), err ) );
    CleanupStack::PopAndDestroy( &tempFile ); // tempFile
    
    CleanupStack::PopAndDestroy( genericParamList ); // genericParamList
    CleanupStack::PopAndDestroy( contentType8 ); // contentType8
    }

// ---------------------------------------------------------
// CBrowserSpecialLoadObserver::HandleServerAppExit()
// ---------------------------------------------------------
//
void CBrowserSpecialLoadObserver::HandleServerAppExit( TInt /*aReason*/ )
    {
    iSchemeProcessing = EFalse;
    }

#ifdef __WINS__
// ----------------------------------------------------------------------------
// CBrowserSpecialLoadObserver::AskIapIdL(TUint32& aId)
// Ask IAP id from the user
// ----------------------------------------------------------------------------
//
TInt CBrowserSpecialLoadObserver::AskIapIdL( TUint32& aId )
    {
    CCommsDatabase* TheDb;
    TInt res;
    TBuf<40> name;
    TUint32 id;
    RArray<TUint32> idArray; 
    CDesCArrayFlat* items = new (ELeave) CDesCArrayFlat(16);
    CleanupStack::PushL( items );  // 1

    // Get IAP names and ids from the database
    TheDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( TheDb );  // 2

    TheDb->ShowHiddenRecords();

    CCommsDbTableView* view = TheDb->OpenTableLC( TPtrC( IAP ) );  // 3
    res = view->GotoFirstRecord();

    while( !res )
        {
        view->ReadTextL( TPtrC( COMMDB_NAME ), name );
        view->ReadUintL( TPtrC( COMMDB_ID ), id );

        idArray.Insert( id, 0 );
        items->InsertL( 0, name );

        res = view->GotoNextRecord();
        RDebug::Print(_L("IAP name, id: %S, %d"), &name, id );
        }

    // Create listbox and PUSH it.
    CEikTextListBox* list = new (ELeave) CAknSinglePopupMenuStyleListBox;
    CleanupStack::PushL( list );  // 4

    // Create popup list and PUSH it.
    CAknPopupList* popupList = CAknPopupList::NewL(
        list, R_AVKON_SOFTKEYS_OK_EMPTY__OK,
        AknPopupLayouts::EMenuWindow);
    CleanupStack::PushL( popupList );  // 5

    // initialize listbox.
    list->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
    list->CreateScrollBarFrameL(ETrue);
    list->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff,
        CEikScrollBarFrame::EAuto);

    // Set listitems.
    CTextListBoxModel* model = list->Model();
    model->SetItemTextArray( items );
    model->SetOwnershipType( ELbmOwnsItemArray );

    // Set title
    popupList->SetTitleL( _L( "Select IAP:" ) );

    // Show popup list.
    TInt popupOk = popupList->ExecuteLD();
    if( popupOk )
        {
        TInt index = list->CurrentItemIndex();
        aId = idArray[index];
        }

    CleanupStack::Pop( popupList );       // popuplist
    CleanupStack::PopAndDestroy( list );  // list
    CleanupStack::PopAndDestroy( view );  // view
    CleanupStack::PopAndDestroy( TheDb ); // TheDb
    CleanupStack::Pop( items );           // items CTextListBoxModel* model owns it

    idArray.Close();
    items = NULL;
    return popupOk;
    }

#endif // __WINS__

// End of file
