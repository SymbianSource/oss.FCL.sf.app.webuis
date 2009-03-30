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
* Description:  CVideoServicesContainer implementation
*
*/


// INCLUDE FILES
#include "VideoServicesContainer.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CVideoServicesContainer::NewL( const TRect& aRect )
// EPOC two phased constructor
// ---------------------------------------------------------
//
CVideoServicesContainer* CVideoServicesContainer::NewL( const TRect& aRect )
	{
    CVideoServicesContainer* self = new ( ELeave ) CVideoServicesContainer();
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    CleanupStack::Pop();
    return self;
	}

// ---------------------------------------------------------
// CVideoServicesContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CVideoServicesContainer::ConstructL( const TRect& aRect )
    {
    CreateWindowL();
    SetRect( aRect );
    ActivateL();
    }

// Destructor
CVideoServicesContainer::~CVideoServicesContainer()
    {
    }


// ---------------------------------------------------------
// CVideoServicesContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CVideoServicesContainer::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbWhite );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CVideoServicesContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CVideoServicesContainer::HandleControlEventL( 
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/ )
    {

    }

// End of File  
