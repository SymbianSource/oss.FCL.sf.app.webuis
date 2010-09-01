/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <s32strm.h>
#include "BrowserBookmarksOrder.h"
#include "Commonconstants.h"

_LIT( KBrowserBMOrderDataBegin, "BMOrderDataBegin" );
_LIT( KBrowserBMOrderDataEnd, "BMOrderDataEnd" );


// ---------------------------------------------------------
// CBrowserBookmarksOrder::NewLC()
// ---------------------------------------------------------
//

CBrowserBookmarksOrder* CBrowserBookmarksOrder::NewLC()
    {
   CBrowserBookmarksOrder* order = 
        new (ELeave) CBrowserBookmarksOrder();
    CleanupStack::PushL(order);
    order->ConstructL();
    return order;
    }

// ---------------------------------------------------------
// CBrowserBookmarksOrder::ConstructL()
// ---------------------------------------------------------
//

void CBrowserBookmarksOrder::ConstructL()
    {
    iBMOrderArray = new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
    }

CBrowserBookmarksOrder::~CBrowserBookmarksOrder()
    {
    delete iBMOrderArray;
    }

// ---------------------------------------------------------
// CBrowserBookmarksOrder::InternalizeL
// ---------------------------------------------------------
//

void CBrowserBookmarksOrder::InternalizeL( RReadStream& aStream )
    {
    iBMOrderArray->Reset();
    HBufC* tag = HBufC::NewLC(sizeof (KBrowserBMOrderDataBegin));
    TInt err = 0;
    TPtr des = tag->Des();

    TRAP(err, aStream>>des );
   if ( (err == KErrNone) && (tag->Des() == KBrowserBMOrderDataBegin ) )
        {
        TInt count = aStream.ReadInt16L();
        for (TInt i=0; i<count; i++)
            {
            TInt value;
            TRAP(err, value = aStream.ReadInt16L());
            if (err == KErrNone)
                {
                iBMOrderArray->AppendL(value);
                }
            else
                if (err != KErrEof)
                {
                User::Leave(err);
                }
            }

            TRAP(err, aStream>>des );
            if (tag->Des() != KBrowserBMOrderDataEnd)
                {
                iBMOrderArray->Reset();
                }
        }
    CleanupStack::PopAndDestroy(tag);
    }

// ---------------------------------------------------------
// CBrowserBookmarksOrder::ExternalizeL
// ---------------------------------------------------------
//

void CBrowserBookmarksOrder::ExternalizeL( RWriteStream& aStream ) const
    {
    aStream<<KBrowserBMOrderDataBegin;
    aStream.WriteInt16L(iBMOrderArray->Count());
    for (TInt i=0; i<iBMOrderArray->Count(); i++)
        {
        aStream.WriteInt16L((*iBMOrderArray)[i]);
        }
    aStream<<KBrowserBMOrderDataEnd;
    }

// ---------------------------------------------------------
// CBrowserBookmarksOrder::GetBookMarksOrder()
// ---------------------------------------------------------
//

const CArrayFixFlat<TInt>& CBrowserBookmarksOrder::GetBookMarksOrder()
    {
    return *iBMOrderArray;
    }

// ---------------------------------------------------------
// CBrowserBookmarksOrder::SetBookMarksOrderL
// ---------------------------------------------------------
//

void CBrowserBookmarksOrder::SetBookMarksOrderL(const CArrayFixFlat<TInt>& aBookMarksOrder)
    {

    iBMOrderArray->Reset();
    if ( aBookMarksOrder.Count() )
        {   
        iBMOrderArray->AppendL(&(aBookMarksOrder[0]), aBookMarksOrder.Count());
        }
    }
