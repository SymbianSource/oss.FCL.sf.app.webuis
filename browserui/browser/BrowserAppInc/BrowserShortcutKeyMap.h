/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implements Browser Shortcut Key Map
*
*/


#ifndef BROWSERSHORTCUTKEYMAP_H
#define BROWSERSHORTCUTKEYMAP_H

//  INCLUDES
#include <e32base.h>

// MACROS

// FORWARD DECLARATIONS
class CBrowserContentViewContainer;
class MApiProvider;

// CLASS DECLARATION
/**
* Encapsulate Keymap Cell Info
* @since 5.0
*/
class TKeymapCellData
    {
public:
    // C++ constructor and destructor to initialize/delete pointer data members
    TKeymapCellData();
    ~TKeymapCellData();

    // data
    HBufC* keyText;
    HBufC* lineText;
    TInt   cmdRscId;
    CFbsBitmap* keyBitmapImage;
    CFbsBitmap* keyBitmapMask;
    
    TRect r; // Main rect for cell
    TRect rCell; // Cell contents rect (includes margin)
    TRect rKey;  // key indicator rect
    TRect rLine1; // text line 1 rect
    TRect rLine2; // text line 2 rect
    TRect rIcon; // icon rect        
    };

/**
* Create and manage the keymap sprite
* @since 5.0
*/

class CBrowserShortcutKeyMap: public CBase
    {
    public:  // Constructor and destructor
        /**
        * Two-phased constructor.
        */
        static CBrowserShortcutKeyMap* NewL( CBrowserContentViewContainer* aContentViewContainer, MApiProvider& aApiProvider );

        /**
        * Destructor.
        */
        virtual ~CBrowserShortcutKeyMap();
        
    private:  // Constructors
        /**
        * C++ default constructor.
        */
        CBrowserShortcutKeyMap( CBrowserContentViewContainer* aContentViewContainer, MApiProvider& aApiProvider );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
                
    private: // New functions

        /**
        * @since 5.0
        * @param
        * @return
        * Initialize Keymap properties
        */        
        void InitKeymap(); 

        /**
        * @since 5.0
        * @param
        * @return
        * Initialize Keymap data kept for each cell of the map
        */
        void InitCellDataL();
        
        /**
        * @since 5.0
        * @param
        * @return
        * Create the bitmap and context that will be used for the sprite
        */
        void CreateBitmapAndContextL();

        /**
        * @since 5.0
        * @param
        * @return
        * Calculate the layout of the sprite rects for a given cell
        */
        void CreateCellLayout(const TInt& aCellIndex, const TPoint& aUpperL, const TSize& aSize);

        /**
        * @since 5.0
        * @param
        * @return
        * Draw the sprite rects to the sprite bitmap
        */
        void CreateSpriteCellL(const TInt& aCellIndex);
               
        /**
        * @since 5.0
        * @param
        * @return
        * Construct the Sprite from the bitmap and context
        */
        void ConstructSprite();

        /**
        * @since 5.0
        * @param
        * @return
        * Utility: Get the folder where the canned image is located
        */       
        HBufC* GetCannedImageDirL();
        
        /**
        * @since 5.0
        * @param
        * @return
        * Utility: get the SVG bitmaps, etc, given its filename
        */
        void LoadSvg( TInt aImageId, const TDesC& aFileName, CFbsBitmap*& aImage, CFbsBitmap*& aImageMask );
        
        /**
        * @since 7.x
        * @param
        * @return
        * Utility: create dimmed versions of bitmaps aImage
        */
        void CreateDimBitmapL(  CFbsBitmap* aImage,
                                CFbsBitmap* aImageMask,
                                CFbsBitmap*& aDimImage,
                                CFbsBitmap*& aDimImageMask);
                                  
            
    private:    // Data
        CBrowserContentViewContainer* iContentViewContainer;  // not owned
        MApiProvider& iApiProvider;                           // not owned, used to query browser prefs
         
        TRect iTargetRect;                                    // Main rect of the keymap, sized according to parent container
        
        const CFont* iKeyFont;                                // not owned, font used for the key indicator text
        const CFont* iLineFont;                               // not owned, font used for the function description  
        RPointerArray<TKeymapCellData> iCellData;             // data structure to hold data for each key cell
        
        HBufC* iSvgMbmFile;                                   // holds icon svg

        /* sprite data */
        RWsSprite iSprite;
        CFbsBitmapDevice*        iSpriteBitmapDevice;         // owned
        CFbsBitGc*               iSpriteBitmapContext;        // owned
        CFbsBitmap*              iSpriteBitmap;               // owned
        CFbsBitmapDevice*        iSpriteMaskBitmapDevice;     // owned
        CFbsBitGc*               iSpriteMaskBitmapContext;    // owned
        CFbsBitmap*              iSpriteMaskBitmap;           // owned
    };

#endif      // BROWSERSHORTCUTKEYMAP_H

// End of File
