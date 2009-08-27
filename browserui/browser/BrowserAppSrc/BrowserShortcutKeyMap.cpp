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
* Description:  Browser View
*
*/

// INCLUDE FILES
#include <aknutils.h>
#include <gulicon.h>
#include <StringLoader.h>
#include <data_caging_path_literals.hrh>
#include <eikbtgpc.h>
#include <eikapp.h>
#include <AknsUtils.h>
#include <e32math.h>
#include <browser.mbg>
#include "BrowserShortcutKeyMap.h"
#include "BrowserContentViewContainer.h"
#include "BrowserPreferences.h"
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
#include "ApiProvider.h"
#include "BrowserUIVariant.hrh"
#include "BrowserAppUi.h"

// CONSTANTS
_LIT( KKeyMapSvgFile, "browser.mif" );

// Value table for 70% Transparency
const TInt KTransparency[256] =
    {
    0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7,
    8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13,
    14, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 19,
    19, 19, 20, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 23, 24, 24,
    24, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29,
    30, 30, 30, 31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35,
    35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 38, 39, 39, 39, 40, 40, 40,
    41, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 46,
    46, 46, 47, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 50, 51, 51,
    51, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 56,
    57, 57, 57, 58, 58, 58, 59, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62,
    62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65, 65, 66, 66, 66, 67, 67, 67,
    68, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71, 71, 72, 72, 72, 73,
    73, 73, 74, 74, 74, 74, 75, 75, 75, 76, 76, 76, 77, 77
    };


// keymap is 3x4
#define KNumCellsX 3
#define KNumCellsY 4
#define KNumCells  12

// spacing in pixels for scroll bars
#define KKeymapHScrollbarSize 4
#define KKeymapVScrollbarSize 4

// sizing for entire keymap (not counting scrollbar)
// as a percentage of vertical and horizontal screen size
#define KKeymapVSize 0.90
#define KKeymapHSize 0.90

// margins for each cell
#define KCellTopMargin    2
#define KCellRightMargin  5
#define KCellBotMargin    2
#define KCellLeftMargin   5

// Create constant icon assignments for each command
// note that this follows TShortcutsCommandDecodeEntry in BrowserPreferences.h

typedef const TInt TShortcutsIconAndTextDecodeEntry[2];

#define KShortcutsIconAndTextAssignmentSize 22
#define KShortcutsIconAndTextDecodeMatrixText  0  // First entry is the Text
#define KShortcutsIconAndTextDecodeMatrixIcon  1  // Second entry is the Icon

static const TShortcutsIconAndTextDecodeEntry KShortcutsIconAndTextAssignmentMatrix[] =
{
    /* 0   EWmlNoCmd,                       */  { R_BROWSER_KEYMAP_TEXT_NONE,               EMbmBrowserQgn_indi_browser_tb_key_map      },
    /* 1   EWmlCmdHelp,                     */  { R_BROWSER_KEYMAP_TEXT_SHOW_HELP,          EMbmBrowserQgn_indi_browser_tb_help         },
    /* 2   EWmlCmdSwitchWindow,             */  { R_BROWSER_KEYMAP_TEXT_SWITCH_WINDOW,      EMbmBrowserQgn_indi_browser_tb_switch_win   },
    /* 3   EWmlCmdSavePage,                 */  { R_BROWSER_KEYMAP_TEXT_SAVE_PAGE,          EMbmBrowserQgn_indi_browser_tb_save_page    },
    /* 4   EWmlCmdShowImages,               */  { R_BROWSER_KEYMAP_TEXT_VIEW_IMAGES,        EMbmBrowserQgn_indi_browser_tb_view_images  },
    /* 5   EWmlCmdHistory,                  */  { R_BROWSER_KEYMAP_TEXT_VISUAL_HISTORY,     EMbmBrowserQgn_indi_browser_tb_view_pages   },
    /* 6   EWmlCmdFavourites,               */  { R_BROWSER_KEYMAP_TEXT_MANAGE_BOOKMARKS,   EMbmBrowserQgn_indi_browser_tb_bm           },
    /* 7   EWmlCmdShowSubscribeList,        */  { R_BROWSER_KEYMAP_TEXT_SUBSCRIBE_TO_FEEDS, EMbmBrowserQgn_indi_browser_tb_rss          },
    /* 8   EWmlCmdRotateDisplay,            */  { R_BROWSER_KEYMAP_TEXT_ROTATE_SCREEN,      EMbmBrowserQgn_indi_browser_tb_rotate       },
    /* 9   EWmlCmdPreferences,              */  { R_BROWSER_KEYMAP_TEXT_SETTINGS,           EMbmBrowserQgn_indi_browser_tb_settings     },
    /* 10  EWmlCmdZoomOut,                  */  { R_BROWSER_KEYMAP_TEXT_ZOOM_OUT,           EMbmBrowserQgn_indi_browser_tb_zoom_out     },
    /* 11  EWmlCmdZoomIn,                   */  { R_BROWSER_KEYMAP_TEXT_ZOOM_IN,            EMbmBrowserQgn_indi_browser_tb_zoom_in      },
    /* 12  EWmlCmdLaunchHomePage,           */  { R_BROWSER_KEYMAP_TEXT_GO_TO_HOMEPAGE,     EMbmBrowserQgn_indi_browser_tb_home         },
    /* 13  EWmlCmdOneStepBack,              */  { R_BROWSER_KEYMAP_TEXT_PREVIOUS_PAGE,      EMbmBrowserQgn_indi_browser_tb_back         },
    /* 14  EWmlCmdShowMiniature,            */  { R_BROWSER_KEYMAP_TEXT_MINIATURE_SHOW,     EMbmBrowserQgn_indi_browser_tb_page         },
    /* 15  EWmlCmdReload,                   */  { R_BROWSER_KEYMAP_TEXT_RELOAD,             EMbmBrowserQgn_indi_browser_tb_reload       },
    /* 16  EWmlCmdFindKeyword,              */  { R_BROWSER_KEYMAP_TEXT_FIND_KEYWORD,       EMbmBrowserQgn_indi_browser_tb_find_kw      },
    /* 17  EWmlCmdSaveAsBookmark,           */  { R_BROWSER_KEYMAP_TEXT_SAVE_AS_BOOKMARK,   EMbmBrowserQgn_indi_browser_tb_save_bm      },
    /* 18  EWmlCmdGoToAddress,              */  { R_BROWSER_KEYMAP_TEXT_GO_TO_WEB_ADDRESS,  EMbmBrowserQgn_indi_browser_tb_goto         },
    /* 19  EWmlCmdShowToolBar,              */  { R_BROWSER_KEYMAP_TEXT_SHOW_TOOLBAR,       EMbmBrowserQgn_indi_browser_tb_tb           },
    /* 20  EWmlCmdShowShortcutKeymap,       */  { R_BROWSER_KEYMAP_TEXT_SHOW_KEYMAP,        EMbmBrowserQgn_indi_browser_tb_key_map      },
    /* 21  EWmlCmdEnterFullScreenBrowsing,  */  { R_BROWSER_KEYMAP_TEXT_FULL_SCREEN,        EMbmBrowserQgn_indi_button_exit_fs          },
};

// key assignments
static const TInt KDialpadAssignments[] =
{
    /*  0  */ R_BROWSER_KEYMAP_KEY0,
    /*  1  */ R_BROWSER_KEYMAP_KEY1,
    /*  2  */ R_BROWSER_KEYMAP_KEY2,
    /*  3  */ R_BROWSER_KEYMAP_KEY3,
    /*  4  */ R_BROWSER_KEYMAP_KEY4,
    /*  5  */ R_BROWSER_KEYMAP_KEY5,
    /*  6  */ R_BROWSER_KEYMAP_KEY6,
    /*  7  */ R_BROWSER_KEYMAP_KEY7,
    /*  8  */ R_BROWSER_KEYMAP_KEY8,
    /*  9  */ R_BROWSER_KEYMAP_KEY9,
    /* 10  */ R_BROWSER_KEYMAP_KEY10,
    /* 11  */ R_BROWSER_KEYMAP_KEY11
};

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TKeymapCellData::TKeymapCellData
// TKeymapCellData Utility Class constructor
// -----------------------------------------------------------------------------

TKeymapCellData::TKeymapCellData() :
    keyText(NULL),
    lineText(NULL),
    cmdRscId(NULL),
    keyBitmapImage(NULL),
    keyBitmapMask(NULL)
    {
    }


// -----------------------------------------------------------------------------
// TKeymapCellData::TKeymapCellData
// TKeymapCellData Utility Class destructor
// -----------------------------------------------------------------------------
TKeymapCellData::~TKeymapCellData()
    {
    if(keyText) delete keyText;
    if(lineText) delete lineText;
    if(keyBitmapImage) delete keyBitmapImage;
    if(keyBitmapMask) delete keyBitmapMask;
    }

// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::NewL
// The two-phase Symbian constructor
// -----------------------------------------------------------------------------
//
CBrowserShortcutKeyMap* CBrowserShortcutKeyMap::NewL( CBrowserContentViewContainer* aContentViewContainer, MApiProvider& aApiProvider )
    {
    CBrowserShortcutKeyMap* self = new (ELeave) CBrowserShortcutKeyMap( aContentViewContainer, aApiProvider );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(); //self
    return self;
    }



// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
CBrowserShortcutKeyMap::~CBrowserShortcutKeyMap()
    {
    // Close out array and delete data members
    iCellData.Close();

    // cleanup svg mbm file name
    if(iSvgMbmFile)
        {
        delete iSvgMbmFile;
        iSvgMbmFile = NULL;
        }

    // Cleanup Sprite data
    iSprite.Close();
    if(iSpriteBitmapDevice)
        {
        delete iSpriteBitmapDevice;
        iSpriteBitmapDevice = NULL;
        }
    if(iSpriteBitmapContext)
        {
        delete iSpriteBitmapContext;
        iSpriteBitmapContext = NULL;
        }
    if(iSpriteBitmap)
        {
        delete iSpriteBitmap;
        iSpriteBitmap = NULL;
        }

    if(iSpriteMaskBitmapDevice)
        {
        delete iSpriteMaskBitmapDevice;
        iSpriteMaskBitmapDevice = NULL;
        }
    if(iSpriteMaskBitmapContext)
        {
        delete iSpriteMaskBitmapContext;
        iSpriteMaskBitmapContext = NULL;
        }
    if(iSpriteMaskBitmap)
        {
        delete iSpriteMaskBitmap;
        iSpriteMaskBitmap = NULL;
        }

    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::CBrowserShortcutKeyMap
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBrowserShortcutKeyMap::CBrowserShortcutKeyMap( CBrowserContentViewContainer* aContentViewContainer, MApiProvider& aApiProvider )
    : iContentViewContainer( aContentViewContainer ), iApiProvider( aApiProvider )
    {
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::ConstructL
// The constructor that can contain code that might leave.
// -----------------------------------------------------------------------------
//
void CBrowserShortcutKeyMap::ConstructL()
    {
    //LoadResourceFileL();
    iSvgMbmFile = GetCannedImageDirL();
    iSvgMbmFile->Des().Append( KKeyMapSvgFile );

    InitKeymap();
    CreateBitmapAndContextL();
    ConstructSprite();
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::InitKeymap
// Initialize Keymap properties
// -----------------------------------------------------------------------------
//
void CBrowserShortcutKeyMap::InitKeymap()
    {
    // derive keymap's size from the container's size
    iTargetRect = iContentViewContainer->Rect();

    // resize it to account for scrollbar width/height
    iTargetRect.Resize(-TSize(KKeymapHScrollbarSize,KKeymapVScrollbarSize));
    
    TBool landscape = iTargetRect.Size().iWidth > iTargetRect.Size().iHeight ; 

    //
    // derive shrink amounts based on keymap size constants and shrink the rect
    //
    TInt newHeight = (KKeymapVSize * iTargetRect.Size().iHeight);
    TInt newWidth  = (KKeymapHSize * iTargetRect.Size().iWidth);
    
    if(newWidth > KKeymapHSize * iTargetRect.Size().iWidth)
        {
        newWidth = (KKeymapHSize * iTargetRect.Size().iWidth);
        }

    // shrink the height and width slightly
    // as necessary to be evenly divisible by the rows/cols later on
    TReal remainder;
    TInt result;
    result = Math::Mod(remainder,newWidth,KNumCellsX);
    if((result == KErrNone) && (remainder != 0))
        {
        newWidth -= remainder;
        }
    result = Math::Mod(remainder,newHeight,KNumCellsY);
    if((result == KErrNone) && (remainder != 0))
        {
        newHeight -= remainder;
        }
    TInt hShrink = (iTargetRect.Size().iWidth - newWidth)/2;
    TInt vShrink = (iTargetRect.Size().iHeight - newHeight)/2;
    iTargetRect.Shrink(TSize(hShrink,vShrink));

    // Set font types
    iLineFont = LatinPlain12();
    
    if (landscape)
        { // in landscape need to shrink the font or else it gets clipped
        iKeyFont = LatinBold13();
        }
    else
        {
        iKeyFont = LatinBold16();
        }

    // init cell data
    InitCellDataL();
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::InitCellDataL
// Initialize Keymap data kept for each cell of the map
// -----------------------------------------------------------------------------

void CBrowserShortcutKeyMap::InitCellDataL()
    {
    for(TInt i = 0; i < KNumCells; i++)
        {
        TKeymapCellData* cellData = new (ELeave) TKeymapCellData();

        TInt shortcutAssign;

        // get the shortcut assignment from Browser Prefs
        switch(i)
            {
            case 0:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey1Cmd();
                break;
            case 1:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey2Cmd();
                break;
            case 2:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey3Cmd();
                break;
            case 3:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey4Cmd();
                break;
            case 4:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey5Cmd();
                break;
            case 5:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey6Cmd();
                break;
            case 6:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey7Cmd();
                break;
            case 7:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey8Cmd();
                break;
            case 8:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey9Cmd();
                break;
            case 9:
                shortcutAssign = iApiProvider.Preferences().ShortcutKeyStarCmd();
                break;
            case 10:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey0Cmd();
                break;
            case 11:
                shortcutAssign = iApiProvider.Preferences().ShortcutKeyHashCmd();
                break;
            default:
                shortcutAssign = iApiProvider.Preferences().ShortcutKey1Cmd();
                break;
            }

        cellData->cmdRscId = KShortcutsIconAndTextAssignmentMatrix[shortcutAssign][KShortcutsIconAndTextDecodeMatrixText];
        cellData->lineText = StringLoader::LoadLC(cellData->cmdRscId);

        // get shortcut key assignments and set dialpad assignments
        cellData->keyText = StringLoader::LoadLC(KDialpadAssignments[i]);

        // load the icon its mask (use a default if we're out of range)
        TInt iconId = EMbmBrowserQgn_indi_browser_tb_key_map;
        if(shortcutAssign < KShortcutsIconAndTextAssignmentSize)
            {
            iconId = KShortcutsIconAndTextAssignmentMatrix[shortcutAssign][KShortcutsIconAndTextDecodeMatrixIcon];
            }

        if(shortcutAssign == NULL)
            {
            cellData->keyBitmapImage = NULL;
            cellData->keyBitmapMask = NULL;
            }
        else
            {
            LoadSvg(iconId,
                    *iSvgMbmFile,
                    cellData->keyBitmapImage,
                    cellData->keyBitmapMask);
            }

        // add to the data array
        iCellData.Append(cellData);

        // remove keytext, linetext from stack
        CleanupStack::Pop(2);
        }
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::CreateBitmapAndContextL
// Create the bitmap and context that will be used for the sprite
// -----------------------------------------------------------------------------
//
void CBrowserShortcutKeyMap::CreateBitmapAndContextL()
    {
    // Create Sprite Bitmap
    iSpriteBitmap = new (ELeave) CFbsBitmap;
    User::LeaveIfError( iSpriteBitmap->Create( iTargetRect.Size(), EColor16MA ) );

    iSpriteBitmapDevice = CFbsBitmapDevice::NewL( iSpriteBitmap );
    User::LeaveIfError( iSpriteBitmapDevice->CreateContext( iSpriteBitmapContext ) );

    // Create Mask Bitmap
    iSpriteMaskBitmap = new (ELeave) CFbsBitmap;
    User::LeaveIfError( iSpriteMaskBitmap->Create( iTargetRect.Size(), EColor16MA ) );
    iSpriteMaskBitmapDevice = CFbsBitmapDevice::NewL( iSpriteMaskBitmap );
    User::LeaveIfError( iSpriteMaskBitmapDevice->CreateContext( iSpriteMaskBitmapContext ) );

    TRect r( TPoint( 0, 0 ), iSpriteMaskBitmap->SizeInPixels());

    iSpriteMaskBitmapContext->SetBrushStyle( CGraphicsContext::ESolidBrush );
    iSpriteMaskBitmapContext->SetPenStyle( CGraphicsContext::ESolidPen );
    iSpriteMaskBitmapContext->SetBrushColor( TRgb( 0, 0, 0) );
    iSpriteMaskBitmapContext->SetPenColor( TRgb( 0, 0, 0) );
    iSpriteMaskBitmapContext->DrawRect(r);

    // Draw the rectangular outline
    iSpriteBitmapContext->SetBrushStyle( CGraphicsContext::ESolidBrush );
    iSpriteBitmapContext->SetPenStyle( CGraphicsContext::ESolidPen );
    iSpriteBitmapContext->SetBrushColor( TRgb( 242, 244, 242) );
    iSpriteBitmapContext->SetPenColor( TRgb( 100, 100, 0) );
    iSpriteBitmapContext->SetPenSize( TSize(2,2) );
    iSpriteBitmapContext->DrawRect(r);

    TSize cellSize(r.Width()/KNumCellsX,r.Height()/KNumCellsY);

    for(TInt i = 0; i < KNumCellsY; i++)
        {
        for(TInt j = 0; j < KNumCellsX; j++)
            {
            TInt cellIndex(i*KNumCellsX + j);
            CreateCellLayout( cellIndex, TPoint(j*cellSize.iWidth,i*cellSize.iHeight),cellSize);
            CreateSpriteCellL( cellIndex );
            }
        }
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::CreateCellLayout
// Calculate the layout of the sprite rects for a given cell
// -----------------------------------------------------------------------------

void CBrowserShortcutKeyMap::CreateCellLayout(const TInt& aCellIndex, const TPoint& aUpperL, const TSize& aSize)
    {
    // Create pointer for easy access curr Cell
    TKeymapCellData* pCurrCell = iCellData[aCellIndex];

    // Create rect for Cell
    pCurrCell->r.SetRect(aUpperL, aSize);

    // Internally, Our Cell is sized accounting for margin
    // Setup this
    TPoint marginOffset(KCellLeftMargin, KCellTopMargin);
    TSize marginSize(KCellLeftMargin + KCellRightMargin, KCellTopMargin + KCellBotMargin);
    pCurrCell->rCell.SetRect( aUpperL + marginOffset, aSize - marginSize );

    // Upper LH key indicator
    TInt keyFontHeight = iKeyFont->HeightInPixels() + iKeyFont->DescentInPixels();
    pCurrCell->rKey.SetRect( pCurrCell->rCell.iTl, TSize(keyFontHeight, iKeyFont->MaxCharWidthInPixels()) );

    // Prep the line rects, calculating the position based upon the font size
    TInt lineFontHeight = iLineFont->HeightInPixels() + iLineFont->DescentInPixels();
    TSize lineSize(pCurrCell->rCell.Width(),lineFontHeight);
    TPoint lineAreaUpperLH(pCurrCell->rCell.iTl.iX, pCurrCell->rCell.iBr.iY - (2*lineSize.iHeight));

    // Set this point to be the upper left of the line(s) of text.
    // If we have two lines, this will be the upperLH of Line1
    // If we have one line, this will be the upperLH of Line2
    // This, we will use to set the rect size of the icon later
    pCurrCell->rLine1.SetRect(lineAreaUpperLH,lineSize);
    pCurrCell->rLine2.SetRect(lineAreaUpperLH + TPoint(0,lineSize.iHeight),lineSize);
    TPoint linesUpperLH = pCurrCell->rLine1.iTl;

    // CREATE the icon area from what is left over
    TPoint iconUpperLH(pCurrCell->rCell.iTl + TPoint(pCurrCell->rKey.Size().iWidth,0));
    TPoint iconLowerRH(pCurrCell->rCell.iBr.iX,linesUpperLH.iY);
    pCurrCell->rIcon.SetRect(iconUpperLH,iconLowerRH);

    // resize the icon rect to be square if its not already... according to its smallest dimension
    if(pCurrCell->rIcon.Height() > pCurrCell->rIcon.Width())
        {
        TSize sizeChange(0,pCurrCell->rIcon.Width() - pCurrCell->rIcon.Height());
        pCurrCell->rIcon.Resize(sizeChange);
        pCurrCell->rIcon.Move(TPoint(0,-sizeChange.iHeight/2));
        }
    else if(pCurrCell->rIcon.Width() > pCurrCell->rIcon.Height())
        {
        TSize sizeChange(pCurrCell->rIcon.Height() - pCurrCell->rIcon.Width(),0);
        pCurrCell->rIcon.Resize(sizeChange);
        pCurrCell->rIcon.Move(TPoint(-sizeChange.iWidth/2,0));
        }
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::CreateSpriteCellL
// Draw the sprite rects to the sprite bitmap
// ----------------------------------------------------------------------------

void CBrowserShortcutKeyMap::CreateSpriteCellL(const TInt& aCellIndex)
    {
    // Create pointer for easy access curr Cell
    TKeymapCellData* pCurrCell = iCellData[aCellIndex];

    TBool ShowDim = EFalse;


    // Draw rect for Cell
    iSpriteBitmapContext->SetPenSize( TSize(1,1) );
    iSpriteBitmapContext->SetPenStyle( CGraphicsContext::ESolidPen );
    iSpriteBitmapContext->SetPenColor( TRgb( 0, 0, 0) );
    iSpriteBitmapContext->DrawRect(pCurrCell->r);



    // DRAW TEXT
    iSpriteBitmapContext->SetPenColor( TRgb( 0, 0, 0) );

    // Draw key text
    if(pCurrCell->keyText)
    {
        iSpriteBitmapContext->UseFont( iKeyFont );
        iSpriteBitmapContext->DrawText(pCurrCell->keyText->Des(),
            pCurrCell->rKey, iKeyFont->FontMaxHeight(), CGraphicsContext::ECenter, 0);
    }

    // For line1, line2 text, we need to first wrap them
    // based upon the width of their enclosing rectangles
    if(pCurrCell->lineText)
        {
        CArrayFixFlat<TInt> *lineWidthArray = new( ELeave ) CArrayFixFlat<TInt>(2);
        lineWidthArray->AppendL(pCurrCell->rLine1.Width());
        lineWidthArray->AppendL(pCurrCell->rLine2.Width());

        CArrayFixFlat<TPtrC> *WrappedArray = new( ELeave ) CArrayFixFlat<TPtrC>(3);
        AknTextUtils::WrapToArrayL( *(pCurrCell->lineText), *lineWidthArray, *iLineFont, *WrappedArray );

        iSpriteBitmapContext->UseFont( iLineFont );
        iSpriteBitmapContext->DrawText(WrappedArray->At(0),
            pCurrCell->rLine2, iLineFont->FontMaxHeight(), CGraphicsContext::ECenter, 0);

        if(WrappedArray->Count() >= 2)
            {
            iSpriteBitmapContext->DrawText(WrappedArray->At(0),
                pCurrCell->rLine1, iLineFont->FontMaxHeight(), CGraphicsContext::ECenter, 0);
            iSpriteBitmapContext->DrawText(WrappedArray->At(1),
                pCurrCell->rLine2, iLineFont->FontMaxHeight(), CGraphicsContext::ECenter, 0);
            }


        // Set dim flag if needed
        switch (pCurrCell->cmdRscId)
            {
            case R_BROWSER_KEYMAP_TEXT_SWITCH_WINDOW:
                { //EWmlCmdSwitchWindow;
                ShowDim = ( !iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) ||
                            iApiProvider.IsEmbeddedModeOn() ||
                            iApiProvider.WindowMgr().WindowCount() < 2);
                break;
                }

            case R_BROWSER_KEYMAP_TEXT_VIEW_IMAGES:
                { //EWmlCmdShowImages;
                ShowDim = ( !iApiProvider.BrCtlInterface().ImageCountL() );
                break;
                }

            case R_BROWSER_KEYMAP_TEXT_SUBSCRIBE_TO_FEEDS:
                { // EWmlCmdShowSubscribeList
                CBrCtlInterface& brctl = iApiProvider.BrCtlInterface();
                TBool subscribeToItems(EFalse);
                TRAPD(err, const RPointerArray<TBrCtlSubscribeTo>& items = brctl.SubscribeToMenuItemsL();
                subscribeToItems = (err == KErrNone && items.Count() > 0));
                ShowDim = !subscribeToItems;
                break;
                }

            case R_BROWSER_KEYMAP_TEXT_GO_TO_HOMEPAGE:
                { // EWmlCmdLaunchHomePage
                ShowDim = iApiProvider.IsLaunchHomePageDimmedL();
                break;
                }

            case R_BROWSER_KEYMAP_TEXT_PREVIOUS_PAGE:
                { // EWmlCmdOneStepBack
                ShowDim = (!iApiProvider.BrCtlInterface().NavigationAvailable(TBrCtlDefs::ENavigationBack ) );
                break;
                }

            case R_BROWSER_KEYMAP_TEXT_MINIATURE_SHOW:
                { //EWmlCmdShowMiniature;
                ShowDim = ( !iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserGraphicalPage ) ||
                            iApiProvider.WindowMgr().CurrentWindow()->HasWMLContent(ETrue) );
                break;
                }

            case R_BROWSER_KEYMAP_TEXT_SHOW_TOOLBAR:
                { //EWmlCmdShowToolBar;
                if ( !AknLayoutUtils::PenEnabled() && iApiProvider.Preferences().ShowToolbarOnOff() )
                    {
                    ShowDim = iApiProvider.WindowMgr().CurrentWindow()->WMLMode();
                    }
                else
                    {
                    ShowDim = ETrue;
                    }
                break;
                }

            case R_BROWSER_KEYMAP_TEXT_SAVE_AS_BOOKMARK:
                { //EWmlCmdSaveAsBookmark;
                if ( iApiProvider.IsEmbeddedModeOn() && CBrowserAppUi::Static()->IsEmbeddedInOperatorMenu() )
                    {
                    ShowDim = ETrue;
                    }
                else
                    {
                    HBufC *pageUrl = iApiProvider.BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
                    if( ( pageUrl == NULL ) || ( pageUrl->Length() == 0 ) )
                        {
                        ShowDim = ETrue;
                        }
                    CleanupStack::PopAndDestroy( pageUrl );
                    }
                break;
                }

            /* Zoom Mode currently disabled
            case R_BROWSER_KEYMAP_TEXT_ZOOM_MODE:
                { //EWmlCmdZoomMode;

                ShowDim = ( iApiProvider.WindowMgr().CurrentWindow()->WMLMode() ||
                            !iApiProvider.ContentDisplayed() ||
                            AknLayoutUtils::PenEnabled() );
                break;
                }
            */

            default:
            break;
            }
        }

    // DRAW the icon,
    // setting the icon size according to iconRect
    if(pCurrCell->keyBitmapImage != NULL && pCurrCell->keyBitmapMask != NULL)
        {
        AknIconUtils::SetSize(pCurrCell->keyBitmapImage, pCurrCell->rIcon.Size() );
        AknIconUtils::SetSize(pCurrCell->keyBitmapMask, pCurrCell->rIcon.Size() );

        if (ShowDim)
            {
            CFbsBitmap* keyBitmapDimImage;
            CFbsBitmap* keyBitmapDimMask;

            CreateDimBitmapL(pCurrCell->keyBitmapImage,
                             pCurrCell->keyBitmapMask,
                             keyBitmapDimImage,
                             keyBitmapDimMask);

            iSpriteBitmapContext->BitBltMasked(pCurrCell->rIcon.iTl,
                keyBitmapDimImage,
                TRect(TPoint(0, 0),pCurrCell->rIcon.Size() ),
                keyBitmapDimMask,
                ETrue);

            delete keyBitmapDimImage;
            delete keyBitmapDimMask;
            }
        else
            {
            iSpriteBitmapContext->BitBltMasked(pCurrCell->rIcon.iTl,
                pCurrCell->keyBitmapImage,
                TRect(TPoint(0, 0),pCurrCell->rIcon.Size() ),
                pCurrCell->keyBitmapMask,
                ETrue);
            }
        }
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::ConstructSprite
// Construct the Sprite from the bitmap and context
// -----------------------------------------------------------------------------
//
void CBrowserShortcutKeyMap::ConstructSprite()
    {
    iSprite = RWsSprite( CEikonEnv::Static()->WsSession() );
    RWindowTreeNode *window = iContentViewContainer->DrawableWindow();
    iSprite.Construct(*window,iTargetRect.iTl,ESpriteNoChildClip);

    TSpriteMember spriteMem;
    spriteMem.iBitmap = iSpriteBitmap;
    spriteMem.iMaskBitmap = iSpriteMaskBitmap;
    spriteMem.iInvertMask = ETrue;
    iSprite.AppendMember(spriteMem);

    iSprite.SetPosition(iTargetRect.iTl);
    iSprite.Activate();
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::GetCannedImageDirL
// Utility: Get the folder where the canned image is located
// -----------------------------------------------------------------------------
//
HBufC* CBrowserShortcutKeyMap::GetCannedImageDirL( )
    {
    HBufC* mbmFile = NULL;
    TParse parse;

    // Get the drive letter where mbm resides.. it will be the same
    // as this AppUi (browser ng)
    CEikAppUi* pAppUI = (CEikAppUi*)(CCoeEnv::Static()->AppUi());
    TFileName mbmDrive = pAppUI->Application()->DllName();

    parse.Set( mbmDrive, NULL, NULL );
    mbmDrive = parse.Drive();

    // allocate memory for MBM file name
    mbmFile = HBufC::NewL( KMaxFileName );

    // assemble the MBM file name
    TPtr ptr( mbmFile->Des() );
    ptr.SetLength( 0 );
    ptr.Append( mbmDrive );
    ptr.Append( KDC_APP_BITMAP_DIR );
    return mbmFile;
    }



// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::LoadSvg
// Utility: get the SVG bitmaps, etc, given its filename
// -----------------------------------------------------------------------------
//
void CBrowserShortcutKeyMap::LoadSvg(
    TInt aImageId,
    const TDesC& aFileName,
    CFbsBitmap*& aImage ,
    CFbsBitmap*& aImageMask )
    {
    // Don't skin icons--at least for now.
    TRAPD(error, AknIconUtils::CreateIconL( aImage,
                                            aImageMask,
                                            aFileName,
                                            aImageId,
                                            aImageId+1));

    if (error!=KErrNone)
        {
        return;
        }
    }

// -----------------------------------------------------------------------------
// CBrowserShortcutKeyMap::CreateDimBitmap
// Utility: Create Dimmed version of Bitmap
// -----------------------------------------------------------------------------
//
void CBrowserShortcutKeyMap::CreateDimBitmapL(
    CFbsBitmap* aImage,
    CFbsBitmap* aImageMask,
    CFbsBitmap*& aDimImage,
    CFbsBitmap*& aDimImageMask)
    {

    aDimImage = new ( ELeave ) CFbsBitmap;
    CleanupStack::PushL( aDimImage );

    aDimImage->Duplicate( aImage->Handle() );

    aDimImageMask = new ( ELeave ) CFbsBitmap;

    User::LeaveIfError( aDimImageMask->Create(aDimImage->SizeInPixels(),
                                        EGray256 ) );

    CleanupStack::PushL( aDimImageMask );

    CFbsBitmapDevice* bitmapDevice = CFbsBitmapDevice::NewL( aDimImageMask );
    CleanupStack::PushL( bitmapDevice );

    CFbsBitGc* bitGc( NULL );
    User::LeaveIfError( bitmapDevice->CreateContext( bitGc ) );
    CleanupStack::PushL( bitGc );

    bitGc->SetPenStyle( CGraphicsContext::ESolidPen );
    bitGc->BitBlt( TPoint( 0, 0 ), aImageMask );


    aDimImageMask->LockHeap();
    TInt w = aImageMask->SizeInPixels().iWidth;
    TInt h = aImageMask->SizeInPixels().iHeight;
    TInt dataStride = aImageMask->DataStride() - w;
    unsigned char* address = (unsigned char *)aDimImageMask->DataAddress();

    for ( TInt i = 0; i < h; ++i )
        {
        for ( TInt j = 0; j < w; ++j )
            {
            *address = KTransparency[*address];
            ++address;
            }
        address += dataStride;
        }

    aDimImageMask->UnlockHeap();

    CleanupStack::PopAndDestroy( 2 ); // bitmapDevice, bitGc
    CleanupStack::Pop( 2 ); // aDimImage, aDimImageMask


    }

//  End of File
