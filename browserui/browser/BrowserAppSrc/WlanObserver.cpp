/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <e32cmn.h>
#include <E32HAL.H>
#include <wlaninternalpskeys.h> // For WLAN state checking


#include "ApiProvider.h"
#include "WlanObserver.h"
#include "Display.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWlanObserver::CWlanObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CWlanObserver::CWlanObserver(MApiProvider& aApiProvider)
    : CActive( EPriorityStandard ),
    iApiProvider( aApiProvider )
    {       
    CActiveScheduler::Add( this );
    iWlanProperty.Attach( KPSUidWlan, KPSWlanIndicator );  
    iWlanProperty.Subscribe( iStatus ); 
    SetActive();
    }
    
// -----------------------------------------------------------------------------
// CWlanObserver::~CWlanObserver()
// Destructor
// -----------------------------------------------------------------------------
CWlanObserver::~CWlanObserver()
    {        
    Cancel();    
    iWlanProperty.Close(); 
    }

 
// -----------------------------------------------------------------------------
// CWlanObserver::GetCurrentState
//
// Retrieves current state of wireless lan connection
// @param aWlanStateValue reference to current Wlan state value which is updated here
// @return: EFalse if error condition (ignore aWlanStateValue in this case)
//          Etrue if no error (get updated value from aWlanStateValue)
// -----------------------------------------------------------------------------
TBool CWlanObserver::GetCurrentState(TInt &aWlanStateValue)
    {
    TInt value;
    TInt errorval = iWlanProperty.Get( value );          
    if (!errorval )
        {
        aWlanStateValue = value;
        return( ETrue );
        }
    else
        {
        return( EFalse );
        }
    }

// -----------------------------------------------------------------------------
// CWlanObserver::RunL
// -----------------------------------------------------------------------------
void CWlanObserver::RunL()
    {
    if( iStatus==KErrNone )
       {        
        iWlanProperty.Subscribe( iStatus ); 
        SetActive();

        TInt value;   
        if ( GetCurrentState(value) )
            {
            iApiProvider.Display().UpdateWlanIndicator( value );
            }
       }    
    }

// -----------------------------------------------------------------------------
// CWlanObserver::RunError
// -----------------------------------------------------------------------------
TInt CWlanObserver::RunError( TInt /*aError*/ )
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWlanObserver::DoCancel
// -----------------------------------------------------------------------------
void CWlanObserver::DoCancel()
    {    
    iWlanProperty.Cancel();
    }

//  End of File
