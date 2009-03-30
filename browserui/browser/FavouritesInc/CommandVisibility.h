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
*      Declaration of class TCmdVisibility.
*      
*
*/


#ifndef COMMAND_VISIBILITY_H
#define COMMAND_VISIBILITY_H

// INCLUDE FILES

#include <e32base.h>

// FORWARD DECLARATION

struct TBrowserFavouritesSelectionState;

// CLASS DECLARATION

/**
* Visibility info on one menu command. Unfortunately this cannot be put into
* a class, since it is initialized statically. Commands of menu panes are
* put into TCommandVisibilityMap-s (an array of TCommandVisibilityInfo).
* One TCommandVisibilityInfo contains the following information:
* 0. TInt The command.
* 1-5. Visibility under different selection conditions.
* TCommandVisibility::TIndex is used to index into the array.
*/
typedef const TInt TCommandVisibilityInfo[6];

/**
* Variable length array of TCommandVisibilityInfo (i.e. commands and their
* visibility); the array is terminated with -1. This array is handled by
* TCommandVisibility.
*/
typedef const TCommandVisibilityInfo* TCommandVisibilityMap;

/*
* This struct provides a way to initialize menu panes with using visibility
* info arrays.
*/
class TCommandVisibility
    {
    public:     // types

        /*
        * Indexes into the visibility info.
        */
        enum TIndex
            {
            ECommand = 0,   ///<      The command.
            ENothing,       ///< (0)  Visibility (nothing is selected).
            EOneItem,       ///< (1i) Visibility (one item is selected).
            EOneFolder,     ///< (1f) Visibility (one folder is selected).
            EMoreItems,     ///< (*i) Visibility (more selected no folders).
            EItemsFolders,  ///< (*)  Visibility (more selected, incl folders).
            EInvalid = -1   ///<      Unused (sanity checking).
            };

    public:     // new methods

        /**
        * Apply visibility to this menu pane (dim commands).
        * @param aMenuPane Menu pane to dim.
        * @param aCommandInfo Command visibility info to apply.
        * @param aIndex Index to use.
        */
        static void Apply
            (
            CEikMenuPane* aMenuPane,
            TCommandVisibilityMap aCommandInfo,
            TIndex aIndex
            );

        /**
        * Checks whether the menu item can be shown. As this method does not
        * get any menu panes, it will not dim menu items.
        * @param aCommandInfo Command visibility info to apply.
        * @param aIndex Index to use.
        * @return EFalse if the menu item for the appropriate command is not
        * to be shown. This method returns EFalse otherwise.
        */
        static TBool Apply
            (
            const TCommandVisibilityInfo& aCommandInfo,
            TIndex aIndex
            );

        /**
        * Calculate index into the visibility info, based on selection state.
        * @param aSelectionState Selection state.
        * @param aMarkedOnly If ETrue, consider only marked items.
        * @return The index.
        */
        static TIndex Index
            (
            const TBrowserFavouritesSelectionState& aSelectionState,
            TBool aMarkedOnly
            );
    };


// CONSTANTS

// Controlling the visibility.
#define SHOW ETrue      ///< Show this command (do not dim).
#define DIM EFalse      ///< Dim this command.
#define CUSTOM ETrue    ///< Custom handling (for the moment, do not dim).
#define VOID ETrue      ///< Never evaluated.

#endif

// End of file
