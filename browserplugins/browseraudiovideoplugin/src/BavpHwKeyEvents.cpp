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
* Description:  Class which implements handling Hardware Volume Key events.
*				  This class implements methods of MRemConCoreApiTargetObserver
*
*/


// INCLUDE FILES
#include <RemConCoreApiTarget.h>
#include <remconinterfaceselector.h>
#include <aknconsts.h>                      // KAknStandardKeyboardRepeatRate

#include "BavpHwKeyEvents.h"
#include "BavpLogger.h"

// CONSTANTS    
const TInt KVolumeFirstChangePeriod = KAknKeyboardRepeatInitialDelay;
const TInt KVolumeChangePeriod = KAknStandardKeyboardRepeatRate;
const TInt KVolumeChangeUp = 1;      
const TInt KVolumeChangeDown = -1;   

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpHwKeyEvents::NewL
// Two-phased constructor.
// ----------------------------------------------------------------------------
CBavpHwKeyEvents* CBavpHwKeyEvents::NewL( MBavpHwKeyEventsObserver& aObserver )
    {
	Log( EFalse, _L("CBavpHwKeyEvents::NewL") );

    CBavpHwKeyEvents* self = new ( ELeave ) CBavpHwKeyEvents( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
    
// ----------------------------------------------------------------------------
// CBavpHwKeyEvents::~CBavpHwKeyEvents
//  Destructor
// ----------------------------------------------------------------------------
CBavpHwKeyEvents::~CBavpHwKeyEvents()
    {    
	Log( EFalse, _L("CBavpHwKeyEvents::~CBavpHwKeyEvents") );

    if ( iVolumeTimer )
        {
        iVolumeTimer->Cancel();
        delete iVolumeTimer;
        }
    
    delete iInterfaceSelector;
    iCoreTarget = NULL; // For LINT. Owned by iInterfaceSelector
    }

// ----------------------------------------------------------------------------
// CBavpHwKeyEvents::CBavpHwKeyEvents
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------------------------
CBavpHwKeyEvents::CBavpHwKeyEvents( MBavpHwKeyEventsObserver& aObserver )
    : iObserver( aObserver )
    {    
    }
    
// -----------------------------------------------------------------------------
// CBavpHwKeyEvents::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpHwKeyEvents::ConstructL( )
    {
	Log( EFalse, _L("CBavpHwKeyEvents::ConstructL") );
    
    iInterfaceSelector = CRemConInterfaceSelector::NewL();

    // Owned by iInterfaceSelector
    iCoreTarget = CRemConCoreApiTarget::NewL( *iInterfaceSelector, *this ); 
    iInterfaceSelector->OpenTargetL();
    
    // Volume key pressed and held (repeat) timer.
    iVolumeTimer = CPeriodic::NewL( EPriorityNormal );
    }

// ----------------------------------------------------------------------------
// CBavpHwKeyEvents::MrccatoCommand
// Handles hardware key events. The volume up/down keys can be clicked once,
// for volume to increase/decrease one increment, or held down and the volume
// will increase/decrease gradually until the limit is reached. The transport
// keys (play, pause,..) will call the HandleCommandL method on the controller.
// ----------------------------------------------------------------------------
void CBavpHwKeyEvents::MrccatoCommand( TRemConCoreApiOperationId aOperationId, 
                                       TRemConCoreApiButtonAction aButtonAct )
    {
	Log( EFalse, _L("CBavpHwKeyEvents::MrccatoCommand, OpID"), aOperationId );

    switch ( aOperationId )
        {
        case ERemConCoreApiVolumeUp:
            {
            switch ( aButtonAct )
                {
                case ERemConCoreApiButtonPress:
                    {
                    // Volume up held down for 0.6 seconds
                    iChange = KVolumeChangeUp;
                    iVolumeTimer->Start( KVolumeFirstChangePeriod,
                                         KVolumeChangePeriod, 
                                         TCallBack( VolumeTimerCallback, this ) );
                    break;
                    }
                case ERemConCoreApiButtonClick:
                    {
                    // Volume up clicked once
                    iChange = KVolumeChangeUp;
                    DoChangeVolume();
                    break;
                    }
                case ERemConCoreApiButtonRelease:
                    {
                    // Volume up key released, stop timer
                    iVolumeTimer->Cancel();
                    break;
                    }
                default:
                    {
                    // Never hits this
                    break;
                    }
                }
            break;
            }
        case ERemConCoreApiVolumeDown:
            {
            switch ( aButtonAct )
                {
                case ERemConCoreApiButtonPress:
                    {
                    // Volume down key held for 0.6 seconds
                    iChange = KVolumeChangeDown;
                    iVolumeTimer->Start( KVolumeFirstChangePeriod,
                                         KVolumeChangePeriod, 
                                         TCallBack( VolumeTimerCallback, this ) );
                    break;
                    }
                case ERemConCoreApiButtonClick:
                    {
                    // Volume down clicked once
                    iChange = KVolumeChangeDown;
                    DoChangeVolume();
                    break;
                    }
                case ERemConCoreApiButtonRelease:
                    {
                    // Volume down key released, stop timer
                    iVolumeTimer->Cancel();
                    break;
                    }
                default:
                    {
                    // Never hits this
                    break;
                    }
                }
            break;
            }
        default:
            {
            // Don’t do anything here.
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CBavpHwKeyEvents::DoChangeVolume( )
// Change volume depending on the level of increase or decrease
// ----------------------------------------------------------------------------
void CBavpHwKeyEvents::DoChangeVolume( )
    {
	Log( EFalse, _L("CBavpHwKeyEvents::DoChangeVolume") );

    iObserver.BavpHwKeyVolumeChange( iChange );
    }

// ----------------------------------------------------------------------------
// CBavpHwKeyEvents::ChangeVolume
// Method call to change volume
// ----------------------------------------------------------------------------
TInt CBavpHwKeyEvents::VolumeTimerCallback( TAny* aObject )
    {
    // cast, and call non-static function
    static_cast<CBavpHwKeyEvents*>( aObject )->DoChangeVolume( );
    return KErrNone;
    }
        
// End of File


