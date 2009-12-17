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
#include <e32base.h>
#include <favouritesitemdata.h>

class CBrowserBookmarksOrder : public MFavouritesItemData,
                               public CBase
    {
    public:

    static CBrowserBookmarksOrder* NewLC();

    void ConstructL();

    /**
    * Destructor.
    */
    virtual ~CBrowserBookmarksOrder();
    
    const CArrayFixFlat<TInt>& GetBookMarksOrder();

    void SetBookMarksOrderL(const CArrayFixFlat<TInt>& aBookMarksOrder);

    public: //From MFavouritesItemData

        void ExternalizeL( RWriteStream& aStream ) const;

        void InternalizeL( RReadStream& aStream );

    private: //Data members

        CArrayFixFlat<TInt>* iBMOrderArray;
    };