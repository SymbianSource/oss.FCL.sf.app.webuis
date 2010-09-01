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
*      Implementation of TCommandVisibility.
*      
*
*/


// INCLUDE FILES
#include <eikmenup.h>
#include "Browser.hrh"
#include "CommandVisibility.h"
#include "BrowserFavouritesSelectionState.h"
#include "BrowserUtil.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// TCommandVisibility::Apply
// ---------------------------------------------------------
//
void TCommandVisibility::Apply
        (
        CEikMenuPane* aMenuPane,
        TCommandVisibilityMap aCommandInfo,
        TCommandVisibility::TIndex aIndex
        )
    {
    // Dim commands using the command visibility array.
    TInt command;
    TInt i = 0;
    while ( ( command = aCommandInfo[i][ECommand] ) !=
                                                    /*end of list*/EWmlNoCmd )
        {
        if ( !(aCommandInfo[i][aIndex]) )
            {
            aMenuPane->SetItemDimmed( command, ETrue );
            }
        i++;
        }
    }

// ---------------------------------------------------------
// TCommandVisibility::Apply
// ---------------------------------------------------------
//
TBool TCommandVisibility::Apply
        (
        const TCommandVisibilityInfo& aCommandInfo,
        TIndex aIndex
        )
    {
    return aCommandInfo[aIndex];
    }

// ---------------------------------------------------------
// TCommandVisibility::Index
// ---------------------------------------------------------
//
TCommandVisibility::TIndex TCommandVisibility::Index
        (
        const TBrowserFavouritesSelectionState& aSelectionState,
        TBool aMarkedOnly
        )
    {
    TIndex index = EInvalid;

    if ( aMarkedOnly )
        {
        // Consider only marked items (OK-Options menu).
        if ( aSelectionState.IsEmpty() )
            {
            // Empty ???
            index = ENothing;
            }
        else if ( aSelectionState.MarkedCount() == 1 )
            {
            // One is marked. See if item or folder.
            index = aSelectionState.iMarkedItemCount ? EOneItem : EOneFolder;
            }
        else
            {
            // More is marked. See if there are any folders.
            index = aSelectionState.NoFoldersMarked() ?
                                                    EMoreItems : EItemsFolders;
            }
        }
    else
        {
        // All items (Normal Options menu).
        if ( aSelectionState.AnyMarked() )
            {
            if ( aSelectionState.NoFoldersMarked() )
                {
				if ( aSelectionState.MarkedCount() == 1 )
					{					
					index = EOneItem;
					}
				else
					{
					index = EMoreItems;
					}
                }
            else
                {
                // More marked, including folders.
                index = EItemsFolders;
                }
            }
        else if ( aSelectionState.CurrentIsItem() )
            {
            // A bookmark is highlighted.
            index = EOneItem;
            }
        else if ( aSelectionState.CurrentIsFolder() )
            {
            // A folder is highlighted.
            index = EOneFolder;
            }
        else
            {
            __ASSERT_DEBUG( aSelectionState.IsEmpty(), \
                Util::Panic( Util::EFavouritesInternal ) );
            // No items at all.
            index = ENothing;
            }
        }

    __ASSERT_DEBUG( index != EInvalid,
		Util::Panic( Util::EUnExpected ) );

    return index;
    }

// End of File
