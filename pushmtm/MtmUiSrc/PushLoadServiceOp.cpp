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
* Description:  Implementation of CPushLoadServiceOp.
*
*/



// INCLUDE FILES

#include "PushLoadServiceOp.h"
#include "PushMtmUiDef.h"
#include "PushMtmUiPanic.h"
#include "PushMtmLog.h"
#include "PushMtmUtil.h"
#include "PushMtmSettings.h"
#include "PushAuthenticationUtilities.h"
#include <push/CSIPushMsgEntry.h>
#include <push/CSLPushMsgEntry.h>
#include <eikenv.h>
#include <apmstd.h>
#include <schemehandler.h>
#include <AknNoteWrappers.h>
#include <PushEntry.h>
#include <PushMtmUi.rsg>
#include <AknQueryDialog.h>
#include <bldvariant.hrh>
#include <FeatMgr.h>
#include <Uri16.h>
#include <data_caging_path_literals.hrh>
#include <f32file.h>

// CONSTANTS

_LIT( KDefaultScheme, "http://" );

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushLoadServiceOp::NewL
// ---------------------------------------------------------
//
CPushLoadServiceOp* CPushLoadServiceOp::NewL( CMsvSession& aSession,
                                              TMsvId aEntryId,
                                              TRequestStatus& aObserverStatus )
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::NewL")

    CPushLoadServiceOp* self =
        new (ELeave) CPushLoadServiceOp( aSession, aEntryId, aObserverStatus );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::NewL")
    return self;
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::~CPushLoadServiceOp
// ---------------------------------------------------------
//
CPushLoadServiceOp::~CPushLoadServiceOp()
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::~CPushLoadServiceOp")

    Cancel();
    delete iSchemeHandler;
    FeatureManager::UnInitializeLib();

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::~CPushLoadServiceOp")
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::StartL
// ---------------------------------------------------------
//
void CPushLoadServiceOp::StartL()
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::StartL")

    Cancel();

    // Check expiration (SI specific).
    if ( IsExpiredL( iTEntry ) )
        {
        PUSHLOG_WRITE(" Msg is expired.")
        // The entry is expired.

        // Display a confirmation dialog.
        TParse* fileParser = new (ELeave) TParse;
        CleanupStack::PushL( fileParser );
        fileParser->Set( KPushMtmUiResourceFileAndDrive, &KDC_MTM_RESOURCE_DIR, NULL );
        AssureResourceL( fileParser->FullName() );
        CleanupStack::PopAndDestroy( fileParser ); // fileParser
        fileParser = NULL;

        HBufC* value = iCoeEnv.AllocReadResourceLC( R_PUSHLS_EXPIRED_NOTE );
        CAknInformationNote* note = new (ELeave) CAknInformationNote( ETrue );
        note->ExecuteLD( *value );
        CleanupStack::PopAndDestroy( value ); // value

        // Delete the expired message.
        CMsvEntry* cParent = iMsvSession.GetEntryL( iTEntry.Parent() );
        CleanupStack::PushL( cParent );
        cParent->DeleteL( iEntryId );
        CleanupStack::PopAndDestroy( cParent ); // cParent

        // Nothing to do.
        iDone = ETrue;
        }

    CPushMtmUiOperation::StartL();

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::StartL")
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::CPushLoadServiceOp
// ---------------------------------------------------------
//
CPushLoadServiceOp::CPushLoadServiceOp( CMsvSession& aSession,
                                        TMsvId aEntryId,
                                        TRequestStatus& aObserverStatus )
:   CPushMtmUiOperation( aSession, aEntryId, aObserverStatus ),
    iDone( EFalse )
    {
    PUSHLOG_WRITE("CPushLoadServiceOp constructing")
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::ConstructL
// ---------------------------------------------------------
//
void CPushLoadServiceOp::ConstructL()
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::ConstructL")

    FeatureManager::InitializeLibL();

    CPushMtmUiOperation::ConstructL();

    // Initialize iTEntry.
    TMsvId service;
    User::LeaveIfError( iMsvSession.GetEntry( iEntryId, service, iTEntry ) );
    __ASSERT_ALWAYS( iTEntry.iMtm == KUidMtmWapPush,
                     UiPanic( EPushMtmUiPanBadMtm ) );

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::ConstructL")
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::IsExpiredL
// ---------------------------------------------------------
//
TBool CPushLoadServiceOp::IsExpiredL( const TMsvEntry& aTEntry ) const
    {
    TBool ret( EFalse );

    if ( aTEntry.iBioType == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = CSIPushMsgEntry::NewL();
        CleanupStack::PushL( si );
        si->RetrieveL( iMsvSession, iEntryId );
        // Check expiration if expiration time was set.
        if ( si->Expires() != Time::NullTTime() )
            {
    	    TTime today;
	        today.UniversalTime();
            if ( si->Expires() < today )
		        {
                ret = ETrue;
                }
            }
        CleanupStack::PopAndDestroy( si ); // si
        }

    return ret;
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::RetrieveContextAndGetUrlLC
// ---------------------------------------------------------
//
CPushMsgEntryBase* CPushLoadServiceOp::RetrieveContextAndGetUrlLC
                                       ( TPtrC& aUrl ) const
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::RetrieveContextAndGetUrlLC");

    CPushMsgEntryBase* context = NULL;
    const TInt32 bioType( iTEntry.iBioType );

    if ( bioType == KUidWapPushMsgSI.iUid )
        {
        context = CSIPushMsgEntry::NewL();
        }
    else if ( bioType == KUidWapPushMsgSL.iUid &&
              FeatureManager::FeatureSupported( KFeatureIdPushSL ) )
        {
        context = CSLPushMsgEntry::NewL();
        }
    else
        {
        __ASSERT_DEBUG( EFalse, UiPanic( EPushMtmUiPanBadBioType ) );
        User::Leave( KErrNotSupported );
        }

    CleanupStack::PushL( context );
    context->RetrieveL( iMsvSession, iEntryId );

    if ( bioType == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, context );
        aUrl.Set( si->Url() );
        }
    else if ( bioType == KUidWapPushMsgSL.iUid )
        {
        CSLPushMsgEntry* sl = STATIC_CAST( CSLPushMsgEntry*, context );
        aUrl.Set( sl->Url() );
        }
    else
        {
        __ASSERT_DEBUG( EFalse, UiPanic( EPushMtmUiPanBadBioType ) );
        }

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::RetrieveContextAndGetUrlLC");
    return context;
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::AuthenticateL
// ---------------------------------------------------------
//
TBool CPushLoadServiceOp::AuthenticateL( const CPushMsgEntryBase& aPushMsg,
                                         const CPushMtmSettings& aMtmSettings ) const
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::AuthenticateL");

    TBool isAuthenticated = ETrue; // by default.

    if ( iTEntry.iBioType != KUidWapPushMsgSL.iUid )
        {
        PUSHLOG_WRITE(" Not SL");
        isAuthenticated = ETrue; // No authentication needed.
        }
    else
        {
        // SL cache & execute-high message requires additional authentication.
        CSLPushMsgEntry& sl = (CSLPushMsgEntry&)aPushMsg;

        if ( sl.Action() == CSLPushMsgEntry::ESLPushMsgExecuteCache )
            {
            PUSHLOG_WRITE(" SL cache");
            isAuthenticated = TPushAuthenticationUtil::
                              AuthenticateMsgL( aMtmSettings, aPushMsg );
            if ( !isAuthenticated )
                {
                // Ask the user...
                isAuthenticated = ExecuteUserQueryL( R_PUSHLS_SL_CACHE_NOT_AUTH );
                }
            }
        else if
           (
             sl.Action() == CSLPushMsgEntry::ESLPushMsgExecuteHigh &&
             aMtmSettings.ServiceLoadingType() == CPushMtmSettings::EAutomatic
           )
            {
            PUSHLOG_WRITE(" SL high + auto");
            isAuthenticated = TPushAuthenticationUtil::
                              AuthenticateMsgL( aMtmSettings, aPushMsg );
            if ( !isAuthenticated )
                {
                // Ask the user...
                isAuthenticated = ExecuteUserQueryL( R_PUSHLS_SL_NOT_AUTH );
                }
            }
        else
            {
            PUSHLOG_WRITE(" Other SL");
            isAuthenticated = ETrue; // No authentication needed.
            }
        }

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::AuthenticateL");
    return isAuthenticated;
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::ExecuteUserQueryL
// ---------------------------------------------------------
//
TBool CPushLoadServiceOp::ExecuteUserQueryL( TInt aResId ) const
    {
    CAknQueryDialog* queryDlg = CAknQueryDialog::NewL();
    TInt userResponse = queryDlg->ExecuteLD( aResId );
    TBool userWantsToDownload = ( userResponse == EAknSoftkeyYes );

    return userWantsToDownload;
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::ValidateLC
// ---------------------------------------------------------
//
HBufC* CPushLoadServiceOp::ValidateLC( TDesC& aUrl ) const
    {
    TUriParser uriParser;
    TInt err = uriParser.Parse( aUrl );
    User::LeaveIfError( err );
    HBufC* validated = NULL;

    if ( uriParser.IsPresent( EUriScheme ) )
        {
        // Just copy the original URL.
        validated = aUrl.AllocLC();
        }
    else
        {
        // Add default scheme.
        validated = HBufC::NewLC( aUrl.Length() + KDefaultScheme().Length() );
        validated->Des().Copy( KDefaultScheme );
        validated->Des().Append( aUrl );
        }

    return validated;
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::RunL
// ---------------------------------------------------------
//
void CPushLoadServiceOp::RunL()
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::RunL")

    if ( iDone )
        {
        PUSHLOG_WRITE(" Done")
        SignalObserver( KErrNone );
        }
    else
        {
        TPtrC url;
        CPushMsgEntryBase* context = RetrieveContextAndGetUrlLC( url );

        CPushMtmSettings* mtmSettings = CPushMtmSettings::NewLC();
        TBool msgAuthenticated = AuthenticateL( *context, *mtmSettings );
		CleanupStack::PopAndDestroy( mtmSettings ); // mtmSettings

        if ( !msgAuthenticated )
            {
            // Ready.
            SignalObserver( KErrNone );
            }
        else
            {
            // OK, authenticated. Can be downloaded.

            // Mark service read.
            iMtmUtil->MarkServiceUnreadL( iEntryId, EFalse );

            // Download the URL.
            delete iSchemeHandler;
            iSchemeHandler = NULL;
            HBufC* validatedUrl = ValidateLC( url );
            iSchemeHandler = CSchemeHandler::NewL( *validatedUrl );

            if( (url.Mid(0,7).Compare(_L("http://")) == 0) || (url.Mid(0,8).Compare(_L("https://")) == 0))
            {
            	TRAPD( err, iSchemeHandler->HandleUrlStandaloneL());
            	CleanupStack::PopAndDestroy( validatedUrl ); // validatedUrl
            	SignalObserver( err );
            }
            else
            {
           		iSchemeHandler->Observer( this );
				TRAPD(err, iSchemeHandler->HandleUrlEmbeddedL());
				CleanupStack::PopAndDestroy( validatedUrl ); // validatedUrl
			    if( err != KErrNone)
				    SignalObserver( err );
            }

          }

        CleanupStack::PopAndDestroy( context ); // context, needed for 'url'
        }

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::RunL")
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::DoCancel
// ---------------------------------------------------------
//
void CPushLoadServiceOp::DoCancel()
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::DoCancel")
    SignalObserver( KErrCancel );
    PUSHLOG_LEAVEFN("CPushLoadServiceOp::DoCancel")
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::RunError
// ---------------------------------------------------------
//
TInt CPushLoadServiceOp::RunError( TInt aError )
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::RunError")
    PUSHLOG_WRITE_FORMAT(" CPushLoadServiceOp::RunError <%d>",aError)

    TRAP_IGNORE( ShowGlobalErrorNoteL( aError ) );
    return CPushMtmUiOperation::RunError( aError );

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::RunError")
    }

// ---------------------------------------------------------
// CPushLoadServiceOp::HandleServerAppExit
// ---------------------------------------------------------
//
void CPushLoadServiceOp::HandleServerAppExit(TInt LOG_ONLY(aReason))
    {
    PUSHLOG_ENTERFN("CPushLoadServiceOp::HandleServerAppExit")
    PUSHLOG_WRITE_FORMAT(" CPushLoadServiceOp::HandleServerAppExit <%d>",aReason)

    // Ready.
    SignalObserver( KErrNone );

    PUSHLOG_LEAVEFN("CPushLoadServiceOp::HandleServerAppExit")
    }

// End of file.
