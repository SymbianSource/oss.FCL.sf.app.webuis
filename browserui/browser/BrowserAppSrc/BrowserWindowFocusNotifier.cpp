/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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


// INCLUDE Files

// User includes
#include "BrowserWindowFocusNotifier.h"	// Class header
#include "logger.h"

// System includes
#include <e32std.h>

// CONSTANTS

//-----------------------------------------------------------------------------
// CBrowserWindowFocusNotifier::CBrowserWindowFocusNotifier
//-----------------------------------------------------------------------------
CBrowserWindowFocusNotifier::CBrowserWindowFocusNotifier()
	{
	}
	
//-----------------------------------------------------------------------------
// CBrowserWindowFocusNotifier::~CBrowserWindowFocusNotifier
//-----------------------------------------------------------------------------
CBrowserWindowFocusNotifier::~CBrowserWindowFocusNotifier()
	{
LOG_ENTERFN("~CBrowserWindowFocusNotifier");
	FlushAOStatusArray();
	}

//-----------------------------------------------------------------------------
//	CBrowserWindowFocusNotifier* CBrowserWindowFocusNotifier::NewL
//-----------------------------------------------------------------------------
CBrowserWindowFocusNotifier* CBrowserWindowFocusNotifier::NewL()
	{
	CBrowserWindowFocusNotifier* self = new (ELeave) 
	                                            CBrowserWindowFocusNotifier();
	        
	CleanupStack::PushL( self );
	self->ConstructL( );
	CleanupStack::Pop( self );   // self
	return self;
	}


//-----------------------------------------------------------------------------
//  CBrowserWindowFocusNotifier::ConstructL
//
//-----------------------------------------------------------------------------
void CBrowserWindowFocusNotifier::ConstructL()
	{
	}


//-----------------------------------------------------------------------------
//  CBrowserWindowFocusNotifier::Add()
//-----------------------------------------------------------------------------
TInt CBrowserWindowFocusNotifier::Add( TRequestStatus* aStatus )
    {
    return iAOStatusArray.Append( aStatus );
    }
    
   
//-----------------------------------------------------------------------------
//  CBrowserWindowFocusNotifier::OnFocusGained()
//-----------------------------------------------------------------------------
void CBrowserWindowFocusNotifier::OnFocusGained()
    {
    FlushAOStatusArray();
    }
    
    
//-----------------------------------------------------------------------------
//  CBrowserWindowFocusNotifier::FlushAOStatusArray()
//-----------------------------------------------------------------------------
void CBrowserWindowFocusNotifier::FlushAOStatusArray()
    {
LOG_ENTERFN("CBrowserWindowFocusNotifier::FlushAOStatusArray");
    TInt count = iAOStatusArray.Count();
BROWSER_LOG( ( _L( "Flush count: %d" ), count ) );
	
	// Clear all status requests and then clear array - in order
	for ( TInt i = 0; i < count; i++ )
    	{
BROWSER_LOG( ( _L( "Flushing: %d" ), i ) );
    	TRequestStatus* status = iAOStatusArray[i];
    	User::RequestComplete( status, KErrNone );
    	}
    	
	iAOStatusArray.Reset();
    }
            
//  End of File
