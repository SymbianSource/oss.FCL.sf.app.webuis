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
*      Implementation of CLauncherServerBase
*
*
*/

// INCLUDES
#include "LauncherClientService.h"
#include "LauncherServerBase.h"
#include "LauncherServerCommands.hrh"
#include <browseroverriddensettings.h>
#include <browserlauncher.h>
#include "BrowserAppDocument.h"
#include "BrowserAppUi.h"
#include "eikenv.h"
#include "eikappui.h"
#include "logger.h"
#include <SysUtil.h>

const TInt KMinimumCDriveDiskSpace = 512 * 1024;

// -----------------------------------------------------------------------------
// CLauncherServerBase::CBrowserLauncherService()
// -----------------------------------------------------------------------------
//
EXPORT_C CLauncherServerBase::CLauncherServerBase()
    {
    LOG_ENTERFN( "CLauncherServerBase::CLauncherServerBase" );

    iClientBuffer = NULL;
    CEikonEnv* browserAppEnv = CEikonEnv::Static();
    if ( browserAppEnv )
        {
        iBrowserAppUi = ( CBrowserAppUi* ) browserAppEnv->EikAppUi();
        if ( iBrowserAppUi )
            {
            iBrowserDocument = ( CBrowserAppDocument* ) iBrowserAppUi->Document();
            }
        }
    }

// -----------------------------------------------------------------------------
// CLauncherServerBase::ServiceL()
// -----------------------------------------------------------------------------
//
EXPORT_C void CLauncherServerBase::ServiceL( const RMessage2& aMessage )
    {
    LOG_ENTERFN( "CLauncherServerBase::ServiceL" );
    LOG_WRITE_FORMAT(" Function: %d", aMessage.Function());

    switch ( aMessage.Function() )
        {
        case ELauncherSendSync :
                {
                __ASSERT_DEBUG(iMySyncMessage.Handle()==0, User::Panic(_L("Launcher"),2));
                iMySyncMessage = aMessage;
                HandleMessageSyncL();
                __ASSERT_DEBUG(iMySyncMessage.Handle()==0, User::Panic(_L("Launcher"),21));
                break;
                }
        case ELauncherSendASync :
                {
                __ASSERT_DEBUG(iMyAsyncMessage.Handle()==0, User::Panic(_L("Launcher"),3));
                iMyAsyncMessage = aMessage;
                HandleMessageASyncL();
                __ASSERT_DEBUG(iMyAsyncMessage.Handle()!=0, User::Panic(_L("Launcher"),31));
                break;
                }
        case ELauncherSendASyncListen:
                {
                __ASSERT_DEBUG(iMyAsyncMessage.Handle()==0, User::Panic(_L("Launcher"),4));
                iMyAsyncMessage = aMessage;
                // Nothing else to do. It's a reinitialization, we only wanted to know the message.
                __ASSERT_DEBUG(iMyAsyncMessage.Handle()!=0, User::Panic(_L("Launcher"),41));
                break;
                }
        case ELauncherSendSyncBuffer :
                {
                __ASSERT_DEBUG(iMySyncMessage.Handle()==0, User::Panic(_L("Launcher"),5));
                iMySyncMessage = aMessage;
                HandleMessageSyncBufferL();
                __ASSERT_DEBUG(iMySyncMessage.Handle()==0, User::Panic(_L("Launcher"),51));
                break;
                }
        case ELauncherSendSyncBools :
                {
                __ASSERT_DEBUG(iMySyncMessage.Handle()==0, User::Panic(_L("Launcher"),6));
                iMySyncMessage = aMessage;
                HandleMessageSyncBoolsL();
                __ASSERT_DEBUG(iMySyncMessage.Handle()==0, User::Panic(_L("Launcher"),61));
                break;
                }
        case ELauncherCancelReceive :
                {
                iMyAsyncMessage.Complete( KErrCancel );
                HandleBrowserExit();
                aMessage.Complete( KErrNone );
                break;
                }
        default:
                {
                CAknAppServiceBase::ServiceL( aMessage );
                break;
                }
        }
    }

// -----------------------------------------------------------------------------
// CLauncherServerBase::HandleMessageSyncL()
// -----------------------------------------------------------------------------
//
void CLauncherServerBase::HandleMessageSyncL()
    {
    LOG_ENTERFN( "CLauncherServerBase::HandleMessageSyncL" );

    // Client parameters. These will be filled by function
    // UnPackLauncherDataL()...
    TUid embeddingApplicationUid;
    TBrowserOverriddenSettings settings;
    TBrowserOverriddenSettings* pSettings;
    TInt folderUid;
    TPtrC seamlessParam;
    TBool isContentHandlerRegistered( EFalse );
    TBool isOverridenSettings( EFalse );

    TInt result; // The result of the processed data.

    // Read the recieved data buffer
    HBufC8* incomingDataBuffer = HBufC8::NewMaxLC( iMySyncMessage.GetDesLengthL( 0 ) );
    TPtr8 incomingDataPointer = incomingDataBuffer->Des( );
    iMySyncMessage.ReadL( 0, incomingDataPointer );
    LOG_WRITE(" ReadL OK");

    // Unpack the recieved data buffer
    pSettings = &settings;
    TLauncherPackageUtils::UnPackLauncherDataL( embeddingApplicationUid,
                                                pSettings,
                                                folderUid,
                                                seamlessParam,
                                                incomingDataPointer,
                                                isContentHandlerRegistered,
                                                isOverridenSettings );

    // Process the unpacked data.
    result = ProcessMessageSyncL( embeddingApplicationUid,
                                  settings,
                                  folderUid,
                                  seamlessParam,
                                  isContentHandlerRegistered );

    // Cleanup, and complete the client`s request.
    CleanupStack::PopAndDestroy( ); // incomingDataBuffer
    iMySyncMessage.Complete( result );
    }

// -----------------------------------------------------------------------------
// CLauncherServerBase::HandleMessageASyncL()
// -----------------------------------------------------------------------------
//
void CLauncherServerBase::HandleMessageASyncL()
    {
    LOG_ENTERFN( "CLauncherServerBase::HandleMessageASyncL" );

    // Check for ciritical disk space if starts in embeded mode
    RFs fs;
    User::LeaveIfError(fs.Connect());
    TInt drive( EDriveC );
    TBool isSpace( EFalse );
    TInt err( KErrNone );
    TRAP( err, isSpace = !SysUtil::DiskSpaceBelowCriticalLevelL(&fs, KMinimumCDriveDiskSpace, drive ));
    fs.Close();
    if (!isSpace)
        User::Leave(KErrDiskFull);

    // Client parameters. These will be filled by function
    // UnPackLauncherDataL()...
    TUid embeddingApplicationUid;
    TBrowserOverriddenSettings settings;
    TBrowserOverriddenSettings* pSettings;
    TInt folderUid;
    TPtrC seamlessParam;
    TBool isContentHandlerRegistered( EFalse );
    TBool isOverridenSettings( EFalse );

    // Read the recieved data buffer
    HBufC8* incomingDataBuffer = HBufC8::NewMaxLC( iMyAsyncMessage.GetDesLengthL( 0 ) );
    TPtr8 incomingDataPointer = incomingDataBuffer->Des( );
    iMyAsyncMessage.ReadL( 0, incomingDataPointer );
    LOG_WRITE(" ReadL OK");

    // Unpack the recieved data buffer
    pSettings = &settings;
    TLauncherPackageUtils::UnPackLauncherDataL( embeddingApplicationUid,
                                                pSettings,
                                                folderUid,
                                                seamlessParam,
                                                incomingDataPointer,
                                                isContentHandlerRegistered,
                                                isOverridenSettings );

    // Process the unpacked data.
    ProcessMessageASyncL( embeddingApplicationUid,
                          settings,
                          folderUid,
                          seamlessParam,
                          isContentHandlerRegistered,
                          isOverridenSettings );

    // Cleanup.
    CleanupStack::PopAndDestroy( ); // incomingDataBuffer
    }

void CLauncherServerBase::HandleMessageSyncBufferL()
    {
    LOG_ENTERFN( "CLauncherServerBase::HandleMessageSyncBufferL" );

    ProcessMessageSyncBufferL();
    // And complete client`s request.
    iMySyncMessage.Complete( KErrNone );
    }

void CLauncherServerBase::HandleMessageSyncBoolsL()
    {
    LOG_ENTERFN( "CLauncherServerBase::HandleMessageSyncBoolsL" );

    TBool clientWantsToContinue = iMySyncMessage.Int0();
    TBool wasContentHandled = iMySyncMessage.Int1();
    ProcessBoolsL( clientWantsToContinue, wasContentHandled );
    // And complete client`s request.
    iMySyncMessage.Complete( KErrNone );
    }

void CLauncherServerBase::HandleBrowserExit()
    {
    LOG_ENTERFN( "CLauncherServerBase::HandleBrowserExit" );
    DoBrowserExit( );
    }

/* Capability check will come later...
CLauncherServerBase::CLauncherServerBase(TCapability aClientReqs = ECapability_None):iClientReqs(aClientReqs)
    {
    }

CPolicyServer::TCustomResult SecurityCheckL( const RMessage2& aMsg, TInt& aAction, TSecurityInfo& aMissing)
    {
    switch (aMsg.Function())
        {
        case RLauncherClientService::EMessageId:
            return aMsg.HasCapability(iClientReqs) ? CPolicyServer::EPass : CPolicyServer::EFail;
            break;
        default:
            return CApaAppServiceBase::SecurityCheckL( aMsg, aAction, aMissing );
            break;
        }
    }
*/

// End of File
