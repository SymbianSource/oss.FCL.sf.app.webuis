/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*       Implementation of CBrowserPopupList and CBrowserAdaptiveListPopup
*
*
*/

// INCLUDE FILES
#include <e32std.h>
#include <AknPopup.h>
#include <AknListQueryDialog.h>
#include <AknIconArray.h>
#include <recenturlstore.h>
#include <AknsUtils.h>
#include <AknsListBoxBackgroundControlContext.h>
#include <AppApacLayout.cdl.h>
#include <AppLayout.cdl.h>
#include <AknLayout.cdl.h>
#include <SkinLayout.cdl.h>
#include <AknLayout2ScalableDef.h>
#include <AknLayoutScalable_Avkon.cdl.h>
#include <AknLayoutScalable_Apps.cdl.h>
#include <LayoutMetaData.cdl.h>
#include <AknEnv.h>
#include <aknsdrawutils.h>
#include <browser.mbg>
#include "BrowserContentView.h"
#include "BrowserAdaptiveListPopup.h"
#include "BrowserAppUi.h"
#include "CommonConstants.h"
#include "BrowserAppViewBase.h"
#include <data_caging_path_literals.hrh>
#include <Uri8.h>
#include <utf.h>

#include "eikon.hrh"

// CONSTANTS
const TInt KMaxIcons = 2;
const TInt KListLengthToShow = 64;//the length of the string in the list
_LIT( KItemTab, "1\t");//prefix for items
_LIT( KDirTab, "0\t");//prefix for directories
const TInt KArrayGranularity = 10;
_LIT( KProtocolIdentifier,"://" );
const TUint KSlash('/');
const TUint KPeriod('.');
const TInt KListHeight = 45;
const TInt KFontHeight = 150;

static void TextPos(TPoint *aResultArray, const TAknTextLineLayout
&aLayout, TSize aItemSize)
    {
    TRect itemRect(TPoint(0,0), aItemSize);
    TAknLayoutText textLayout;
    textLayout.LayoutText(itemRect, aLayout);
    aResultArray[0] = textLayout.TextRect().iTl;
    aResultArray[1] = textLayout.TextRect().iTl +
textLayout.TextRect().Size();
    }

static void GfxPos(TPoint *aResultArray, const TAknWindowLineLayout
&aLayout, TSize aItemSize)
    {
    TRect itemRect(TPoint(0,0), aItemSize);
    TAknLayoutRect layout;
    layout.LayoutRect(itemRect, aLayout);
    aResultArray[0] = layout.Rect().iTl;
    aResultArray[1] = layout.Rect().iTl + layout.Rect().Size();
    }

TSize ListBoxItemSize(CEikListBox &aListBox, CFormattedCellListBoxItemDrawer *aItemDrawer)
    {
    TSize size = TSize( aItemDrawer->LafItemSize().iWidth, aListBox.View()->ItemSize().iHeight );
    return size;
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserPopupList::CBrowserPopupList()
//
// ---------------------------------------------------------
//
CBrowserPopupList::CBrowserPopupList(TParentType aParentType):CAknSingleGraphicPopupMenuStyleListBox()
    {
    iParentType = aParentType;
    }

// ---------------------------------------------------------
// CBrowserPopupList::~CBrowserPopupList()
//
// ---------------------------------------------------------
//
CBrowserPopupList::~CBrowserPopupList()
    {
    }

// ---------------------------------------------------------
// CBrowserPopupList::ConstructL
//
// ---------------------------------------------------------
//
void CBrowserPopupList::ConstructL(const CCoeControl* aParent, TInt aFlags)
    {
    CAknSingleGraphicPopupMenuStyleListBox::ConstructL( aParent, aFlags);
    }

// ---------------------------------------------------------
// CBrowserPopupList::SetHighLight
// Sets the highlight of the list
// ---------------------------------------------------------
//
void CBrowserPopupList::SetHighLight(TBool aHighLight)
    {
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF
    iItemDrawer->SetFlags( CListItemDrawer::ESingleClickEnabled );
#endif    
    iListBoxFlags = ( aHighLight ?  iListBoxFlags &~ CEikListBox::EDisableHighlight : iListBoxFlags | CEikListBox::EDisableHighlight );
    if ( aHighLight )
        {
        iItemDrawer->ClearFlags( CListItemDrawer::EDisableHighlight );
        }
    else
        {
        iItemDrawer->SetFlags( CListItemDrawer::EDisableHighlight );
        }
    }

// ---------------------------------------------------------
// CBrowserPopupList::HighLightEnabled()
// Enables the highlight in the list
// ---------------------------------------------------------
//
TBool CBrowserPopupList::HighLightEnabled()
    {
    return !( iItemDrawer->Flags() & CListItemDrawer::EDisableHighlight );
    }

// ---------------------------------------------------------
// CBrowserPopupList::SizeChanged()
// sets the inner layout
// ---------------------------------------------------------
//
void CBrowserPopupList::SizeChanged()
    {
    CAknSingleGraphicPopupMenuStyleListBox::SizeChanged();
    TAknWindowLineLayout windowLineLayout;
    TAknWindowLineLayout windowLineLayoutTmp;
    TAknWindowLineLayout windowLineLayoutTmp2;
    TAknLayoutRect layoutRect;
    TPoint area[2] ;
    TRgb color;
    CEikFormattedCellListBox &aListBox = *this;
    CFormattedCellListBoxItemDrawer *itemDrawer = aListBox.ItemDrawer();
    AknListBoxLayouts::SetupStandardListBox(aListBox);

    AknListBoxLayouts::SetupListboxPos( aListBox, AppLayout::list_single_graphic_popup_wml_pane( 0 ));

    GfxPos(area,
    AknLayoutScalable_Apps::list_single_graphic_popup_wml_pane_g1(),
    ListBoxItemSize(*this, ItemDrawer()) );

    AknListBoxLayouts::SetupFormGfxCell(aListBox,
                                        itemDrawer,
                                        0,
                                        AknLayoutScalable_Apps::list_single_graphic_popup_wml_pane_g2(),
                                        area[0],
                                        area[1]);

    //sets the rectangle of the text
    TextPos(area,
    AknLayoutScalable_Apps::list_single_graphic_popup_wml_pane_t1( ),
    ListBoxItemSize(*this, ItemDrawer()) );
    AknListBoxLayouts::SetupFormAntiFlickerTextCell(aListBox,
                                                    itemDrawer,
                                                    1,
                                                    AknLayoutScalable_Apps::list_single_graphic_popup_wml_pane_t1( ),
                                                    area[0],
                                                    area[1]);
    }

// ---------------------------------------------------------
// CBrowserPopupList::Draw()
// ---------------------------------------------------------
//
void CBrowserPopupList::Draw(const TRect& /*aRect*/) const
    {
    TRect windowRect = this->Rect();
    CEikFormattedCellListBox::Draw( windowRect );
    return;
    }


// ================= CBrowserAdaptiveListPopup =======================


// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::CBrowserAdaptiveListPopup()
// ---------------------------------------------------------
//
CBrowserAdaptiveListPopup::CBrowserAdaptiveListPopup( CEikEdwin* aGotoPaneEdit, 
        CCoeControl* aParent, TParentType aParentType, TBool aSearchFeature): 
        iEditor( aGotoPaneEdit ), iParent( aParent ),
        iParentType( aParentType ), iSearchFeature( aSearchFeature )
    {
    iTouchSupported = AknLayoutUtils::PenEnabled();
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
void CBrowserAdaptiveListPopup::ConstructL( )
    {
    CreateWindowL();
//-------- list ------
    iList = new(ELeave) CBrowserPopupList( iParentType );
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF
    iList->ConstructL( this , CEikListBox::EDisableItemSpecificMenu );
#else
    iList->ConstructL( this , CEikListBox::ELeftDownInViewRect );
#endif
    iList->SetContainerWindowL( *this );
    if (iTouchSupported)
        {
        iList->SetListBoxObserver( this );
        }
    iList->CreateScrollBarFrameL( ETrue );
    iList->SetMopParent(iParent);
    iList->ScrollBarFrame()->SetScrollBarVisibilityL(
                                CEikScrollBarFrame::EOff,
                                CEikScrollBarFrame::EAuto);
    iList->ScrollBarFrame()->SetTypeOfVScrollBar( CEikScrollBarFrame::EDoubleSpan );
    iList->UpdateScrollBarsL();
    iList->SetBorder(TGulBorder::ESingleBlack);
    iList->ItemDrawer()->FormattedCellData()->EnableMarqueeL( ETrue );
//---------- items storing arrays ---------
    iItems = new(ELeave) CDesCArrayFlat(KArrayGranularity);
    iItemNames = new(ELeave) CDesCArrayFlat(KArrayGranularity);
    iItemNamesToShow = new(ELeave) CDesCArrayFlat(KArrayGranularity);
//---------- model ----------
    CTextListBoxModel* model = iList->Model();
    model->SetItemTextArray( iItemNamesToShow );
    model->SetOwnershipType( ELbmDoesNotOwnItemArray );
//---------- icons -----------
    SetIconsL();
//--------- other --------
    iFirstGotoContent = KNullDesC().AllocL();
    iPrevGotoContent = KNullDesC().AllocL();
    iPoppedUp = EFalse;
    iDirectoryMode = ETrue;
    iOpenCBA=EFalse;
    iItemDrawer = iList->ItemDrawer();
    iPrevItem = -2;
    iUrlCompletionMode = EFalse;
    iRecentUrlStore = CBrowserAppUi::Static()->RecentUrlStore();
    if (!iRecentUrlStore)
        {
        User::Leave(KErrCouldNotConnect);
        }

    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::~CBrowserAdaptiveListPopup()
// Destructor
// ---------------------------------------------------------
CBrowserAdaptiveListPopup::~CBrowserAdaptiveListPopup()
    {
    delete iList;
    delete iItems;
    delete iItemNames;
    delete iItemNamesToShow;
    delete iPrevGotoContent;
    delete iFirstGotoContent;
}

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::SetOrdinalPosition
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::SetOrdinalPosition( TInt aPos )
    {
    Window().SetOrdinalPosition( aPos );
    }

void CBrowserAdaptiveListPopup::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if (iList && iTouchSupported)
        {
        //using ComponentControl(0) instead of iList because need access to a
        //protected function
        ComponentControl(0)->HandlePointerEventL( aPointerEvent );
        }
    }

void CBrowserAdaptiveListPopup::HandleListBoxEventL(CEikListBox* aListBox,TListBoxEvent aEventType)
    {
    if (iList && aListBox == iList && iTouchSupported)
        {
        switch (aEventType)
            {
            case MEikListBoxObserver::EEventItemDoubleClicked:
                {
                TKeyEvent keyEvent;
                keyEvent.iModifiers = 0;
                keyEvent.iRepeats = 0;
                keyEvent.iCode = EKeyDevice3;
                keyEvent.iScanCode = EStdKeyDevice3;
                if ( iParentType == EBookmarksGotoPane )
                    {
                    CCoeEnv::Static()->SimulateKeyEventL( keyEvent, EEventKeyDown );
                    }
                CCoeEnv::Static()->SimulateKeyEventL( keyEvent, EEventKey );
                if ( iParentType == EBookmarksGotoPane )
                    {
                    CCoeEnv::Static()->SimulateKeyEventL( keyEvent, EEventKeyUp );
                    }
                }
                break;
            case  MEikListBoxObserver::EEventItemClicked:
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF                
            case  MEikListBoxObserver::EEventItemSingleClicked:      
#endif                
                HandleItemClickedL( aListBox );
                break;
            default:
                break;
            }
        }
    }

void CBrowserAdaptiveListPopup::HandleItemClickedL( CEikListBox* aListBox )
    {
    //the first time
   if ( !iList->HighLightEnabled() )
       {
       iList->SetHighLight( ETrue );

       //save the original content
       if ( !iFirstGotoContent )
           {
           delete iFirstGotoContent;
           iFirstGotoContent = NULL;
           }
       iFirstGotoContent = iEditor->GetTextInHBufL();
       if ( !iFirstGotoContent )
           {
           iFirstGotoContent = KNullDesC().AllocL();
           }
       }
    TInt index = aListBox->CurrentItemIndex();
    index = iItems->Count() - index - 1;
    TPtrC16 item((*iItems)[ index ] );
    TPtrC rightSide;

    //cut the slash from the end if needed
    //permanent sollution until AHLE does not
    //solve this problem related to ID: TMCN-5TTRXV error
    if ( item[item.Length() - 1] != KSlash )
      {
      rightSide.Set( item.Left( item.Length() ) );
      }
    else
      {
      rightSide.Set( item.Left( item.Length() - 1 ) );
      }
    
    //if true then append the suffix(rightSide), to the
    //user-typed data
    //else just use the recent URLs text(rightSide) only
    if ( iUrlCompletionMode )
      {
      HBufC* currentCompletion = HBufC::NewLC(
          iFirstGotoContent->Des().Length() + rightSide.Length() + 1 );
      TPtr currentCompletionPtr = currentCompletion->Des();
    
      currentCompletionPtr.Copy( iFirstGotoContent->Des() );
      currentCompletionPtr.Append( rightSide );
      currentCompletionPtr.ZeroTerminate();
      iEditor->SetTextL( currentCompletion );
      iEditor->SetCursorPosL( currentCompletionPtr.Length(), EFalse );
      CleanupStack::PopAndDestroy( currentCompletion );
      }
    else
      {
      iEditor->SetTextL( &rightSide );
      iEditor->SetCursorPosL( rightSide.Length(), EFalse );
      }
    iEditor->HandleTextChangedL();
    
    delete iPrevGotoContent;
    iPrevGotoContent = NULL;
    iPrevGotoContent = item.AllocL();
    iEditor->DrawNow();
    //change the CBA depending on whether the highlight is
    //on url or domain
    CBrowserAppUi::Static()->UpdateCbaL();
    }


//------------------------------------------------------------------
// CBrowserAdaptiveListPopup::ShowSingleItemPopupListWithGraphicsL(TBool aShowTitle)
// shows "List pane for single item with graphic"
//------------------------------------------------------------------
void CBrowserAdaptiveListPopup::ShowPopupListL(TBool aRelayout)
    {
    TAknWindowLineLayout windowLineLayout;
    TAknWindowLineLayout windowLineLayoutTmp;
    TAknWindowLineLayout windowLineLayoutMain;
    TAknWindowLineLayout windowLineLayoutMainTmp;
    TAknWindowLineLayout windowLineLayoutLW;
    TAknWindowLineLayout windowLineLayoutLWTmp;
    TAknWindowLineLayout windowLineLayoutSLW;
    TAknWindowLineLayout windowLineLayoutSLWTmp;
    TAknLayoutRect layoutRect;
    TAknLayoutRect layoutMainRect;
    TAknLayoutRect layoutLWRect;
    TAknLayoutRect layoutSLWRect;

    //turn of the highlight
    iList->SetHighLight( EFalse );
    //clear arrays
    iItems->Reset();
    iItemNames->Reset();
    iItemNamesToShow->Reset();

    TInt offset;
    HBufC* newText;
    //get create and get the string from the edit field
    if ( iEditor->TextLength() > 0 )
        {
        newText = iEditor->GetTextInHBufL();
        CleanupStack::PushL( newText );
        //cut the protokolidentifier from the string
        offset = newText->Find( KProtocolIdentifier );
        if ( offset == KErrNotFound )
            {
            offset = newText->Length();
            }
        else
            {
            offset = newText->Length() - offset - 3;
            }
        }
    else
        {
             offset = 0;
             newText = KNullDesC().AllocLC();
        }
    TPtrC trimmedContent( newText->Right( offset ) );
    TBuf<KListLengthToShow> buf;

    //find out if the user typed a ".", if so, bring up the url completion list
    TInt periodPos = trimmedContent.LocateReverse( TChar(KPeriod) );
    if ((periodPos != KErrNotFound) && ((periodPos + 1) == trimmedContent.Length()))
        {
        //add the url completion entries to the popup list
        iUrlCompletionMode = ETrue;
        CreateURLCompletionListL();
        }
    else
        {
        iUrlCompletionMode = EFalse;
        //only include recent URLs if not including the url completion items
        iRecentUrlStore->GetData( *iItems, *iItemNames, trimmedContent );
        
        //sort alphabetically and...
        for(TInt i=0; i<iItems->Count(); i++)
            {
            for(TInt j=0; j<iItems->Count()-1; j++)
                {
                    if ((*iItems)[j].CompareF((*iItems)[j+1]) > 0)
                    {  
                       RBuf tmp;
                       CleanupClosePushL(tmp);
                       tmp.Create((*iItems)[j]);
                       iItems->Delete(j);
                       iItems->InsertL( j+1, tmp );
                       tmp.Close();
                       tmp.Create((*iItemNames)[j]);
                       iItemNames->Delete(j);
                       iItemNames->InsertL( j+1, tmp );
                       CleanupStack::PopAndDestroy( &tmp );                   
                    }          
                }
            }  
        //and move url with params i.e google.fi/m=q?xyz do the back of list
        TInt i = 0;                 //iterator
        TInt j = 0;                 //counter
        while (i < (iItems->Count()-1) && j < iItems->Count())
            {
                TUriParser8 parser;
                RBuf8 out;
                CleanupClosePushL( out );
                out.Create( (*iItems)[i].Length() );
                RBuf in;
                CleanupClosePushL( in );              
                in.Create( (*iItems)[i] );
                
                CnvUtfConverter::ConvertFromUnicodeToUtf8(out, in );
                
                if (( parser.Parse( out ) == KErrNone ) &&       //if parse ok and 
                    ( parser.Extract( EUriPath ).Length() > 0 || //url contains path
                      parser.Extract( EUriQuery ).Length() > 0 )) //or query item
                    {
                    iItems->AppendL( (*iItems)[i] );//move item to back of the list
                    iItems->Delete( i );
                    iItemNames->AppendL( (*iItemNames)[i] );
                    iItemNames->Delete( i );
                    }
                else
                    {
                    i++;
                    }
                j++;
                
                CleanupStack::PopAndDestroy( &in );
                CleanupStack::PopAndDestroy( &out );
            };   
            

        }
    iDirectoryMode = EFalse;
    CleanupStack::PopAndDestroy();//newText
    TInt k;
    k = iItems->Count();
    iItemNo = k;
    TInt length = KListLengthToShow - 2;
    if ( k > 0 )
        {
        TInt j;
        TPtrC textToShow;
        TPtrC itemJ;
        //changeing the order
        for (j=k-1; j>-1; j--)
            {
            itemJ.Set( (*iItems)[j] );
            textToShow.Set( itemJ.Left( length ) );
            if ( iDirectoryMode )
                {
            //directories and items
                if ( itemJ[ itemJ.Length() - 1 ] == KSlash )
                    {
                //item
                    buf.Format( _L( "%S%S" ), &KItemTab, &textToShow );
                    }
                else
                    {
                    //directory
                    buf.Format( _L( "%S%S" ), &KDirTab, &textToShow );
                    }
                }
            else
                {
            //items only
                buf.Format( _L( "%S%S" ), &KItemTab, &textToShow );
                }
            iItemNamesToShow->AppendL( buf );
            }
        //set how many item will be shown
        TInt itemstoshow;

        if ( k >  iMaxRecentUrlsToShow)
            {
            itemstoshow = iMaxRecentUrlsToShow;
            }
        else
            {
            itemstoshow = k;
            }

        //set the main rect of the window
        // fixing bug RFON-7E2PPV, don't use ApplicationRect()
        TRect  rect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
        windowLineLayoutMain = AknLayout::main_pane( rect, 0, 2, 1 );
        windowLineLayoutMainTmp = windowLineLayoutMain;
        layoutMainRect.LayoutRect( rect, windowLineLayoutMainTmp );

      
        windowLineLayoutLW = AppLayout::popup_wml_address_window( 1, itemstoshow );
      
        windowLineLayoutLWTmp = windowLineLayoutLW;
        layoutLWRect.LayoutRect( layoutMainRect.Rect(), windowLineLayoutLWTmp );
        windowLineLayoutSLW = AknLayoutScalable_Apps::listscroll_popup_wml_pane();
        windowLineLayoutSLWTmp = windowLineLayoutSLW;
        layoutSLWRect.LayoutRect( layoutLWRect.Rect(), windowLineLayoutSLWTmp );
        windowLineLayout = AppLayout::list_wml_pane( itemstoshow - 1 );
        windowLineLayoutTmp = windowLineLayout;
        layoutRect.LayoutRect( layoutLWRect.Rect(), windowLineLayoutTmp );

        TRect rectTemp(layoutSLWRect.Rect());
        // force listbox on top of goto pane
        rectTemp.iBr.iY = iParent->PositionRelativeToScreen().iY;
        rectTemp.iTl.iX = iParent->PositionRelativeToScreen().iX;
        // shrink list box to size of list  
#ifdef BRDO_TOUCH_ENABLED_FF
        TInt listH = KListHeight;
#else
        TInt listH = AppLayout::list_single_graphic_popup_wml_pane( 0 ).iH;
#endif
        rectTemp.iTl.iY =  rectTemp.iBr.iY - (listH * itemstoshow);
        // set bottom right x axis to full width
        rectTemp.iBr.iX = rect.iBr.iX;
        
        // we only want to call SetRect() when we have a relayout, otherwise just
        // scroll the list to the correct position. Doing this will reduce the amount
        // of flicker seen while typing into the gotopane
        if ((rectTemp.Size() != Rect().Size()) || aRelayout)
            {
            //set the window rect
            SetRect( rectTemp );
            }
        rectTemp = TRect( 0, 0, Rect().Width(), Rect().Height() );
        if ( rectTemp!=iList->Rect() )
            {
            //set the rect of the list
            iList->SetRect( rectTemp );
            }
        else
            {
            // this scrolls list to the correct position, probably still does too much work
            AknListBoxLayouts::SetupListboxPos( *iList, AppLayout::list_single_graphic_popup_wml_pane( 0 ));
            }
        //the last item is visible
        iList->ScrollToMakeItemVisible( iList->Model()->NumberOfItems()-1 );
#ifdef BRDO_TOUCH_ENABLED_FF
        const CFont* pFont = AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont);
        TFontSpec fontSpec = pFont->FontSpecInTwips();
        fontSpec.iHeight =KFontHeight; 
        
        CFont* fontNew;
        CEikonEnv::Static()->ScreenDevice()->GetNearestFontInTwips( ( CFont*&)fontNew, fontSpec );
        CFormattedCellListBoxData *pLBData = iList->ItemDrawer()->ColumnData();
        iList->SetItemHeightL(KListHeight);
        
        if(pLBData && pFont)
        {
            pLBData->SetSubCellFontL(1,fontNew);
        }        
        
        CEikonEnv::Static()->ScreenDevice()->ReleaseFont(fontNew);
#endif        
        
        MakeVisible( ETrue );
        iPoppedUp = ETrue;
        DrawNow();
        iList->Draw(rectTemp);
        }
    else
        {
        MakeVisible( EFalse );
        iPoppedUp = EFalse;
        CBrowserAppUi::Static()->UpdateCbaL();
        }
    if (!IsActivated())
        {
        ActivateL();
        }
    iPrevItem = -2;
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::CreateURLCompletionListL
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::CreateURLCompletionListL()
    {
    _LIT(KSeparator, ",");

    if (iUrlSuffixBuf)
        {
        TPtrC urlSuffix(iUrlSuffixBuf->Des());
        TInt index(urlSuffix.FindF(KSeparator));
        TInt offset = 0;

        while (index != KErrNotFound)
            {
            // Omit empty string
            if (index > 0)
                {
                iItems->AppendL( urlSuffix.Mid(offset,index) );
                }

            offset += (index + 1);
            TPtrC remainder(urlSuffix.Mid(offset));
            index = remainder.FindF(KSeparator);
            }

        // Cover the cases where we only have one suffix in the list and/or
        // the last suffix in the list does not have a comma
        if (offset < urlSuffix.Length())
            {
            TPtrC remainderB(urlSuffix.Mid(offset, urlSuffix.Length() - offset));
            iItems->AppendL( remainderB );
            }
        }
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::HandleControlEventL
// if the content of the edit box changes
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::HandleControlEventL(CCoeControl* aControl,enum MCoeControlObserver::TCoeEvent aEventType)
    {
    CEikEdwin* edwin = ( CEikEdwin* )aControl;
    // the content of the edit box changes
    if ( MCoeControlObserver::EEventStateChanged == aEventType )
        {
        HBufC* newText = edwin->GetTextInHBufL();
        if (!newText)
            {
            newText = KNullDesC().AllocL();
            }
        CleanupStack::PushL( newText );
        //check whether the last content is different to the one in the editbox
        if ( !iPrevGotoContent || iPrevGotoContent->Compare( *newText ) )
            {
            ShowPopupListL();
            delete iPrevGotoContent;
            iPrevGotoContent = NULL;
            iPrevGotoContent = newText;
            CleanupStack::Pop( newText );//ownership is taken
            }
        else
            {
            if ( !iPrevGotoContent )
                {
                CleanupStack::PopAndDestroy( newText );
                }
            else
                {
                ShowPopupListL();
                delete iPrevGotoContent;
                iPrevGotoContent = NULL;
                iPrevGotoContent = newText;
                CleanupStack::Pop( newText );//ownership is taken
                }
            }
        //change the CBA
        if ( iOpenCBA )
            {
            iOpenCBA = EFalse;
            CBrowserAppUi::Static()->UpdateCbaL();
            }
        }
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::CountComponentControls
//
// ---------------------------------------------------------
//
TInt CBrowserAdaptiveListPopup::CountComponentControls() const
    {
    if ( iList == NULL )
        {
        return 0;
        }
    return 1;
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::ComponentControl
//
// ---------------------------------------------------------
//
CCoeControl* CBrowserAdaptiveListPopup::ComponentControl(TInt aIndex) const
    {
    if (aIndex==0)
        {
        return iList;
        }
    else
        {
        return NULL;
        }
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::OfferKeyEventL
// Handles key events
// ---------------------------------------------------------
//
TKeyResponse CBrowserAdaptiveListPopup::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse resp = EKeyWasNotConsumed;
    if (( iPoppedUp &&
    !CBrowserAppUi::Static()->ContentView()->MenuBar()->MenuPane()->IsVisible() &&
    (CBrowserAppUi::Static()->LastActiveViewId() == KUidBrowserContentViewId) ) || (( iPoppedUp ) &&
    (CBrowserAppUi::Static()->LastActiveViewId() != KUidBrowserContentViewId)))
        {
        TInt currIndex = iItems->Count() - iList->CurrentItemIndex() - 1;

        //the joystick is pressed
        if ( aKeyEvent.iCode == EKeyOK )
            {
            if ( ( iList->HighLightEnabled() )
              && ( iDirectoryMode )
              && ( (*iItems)[ currIndex ][(*iItems)[ currIndex ].Length() - 1] != KSlash ) )
                {
                SetDirectoryModeL( EFalse );
                resp = EKeyWasConsumed;
                }
            else
                {
                if (iEditor->TextLength()!=0)
                    {
                    HidePopupL();
                    }
                }
            }

        //the joystick is pressed up
        else if (    aKeyEvent.iCode == EKeyLeftUpArrow      // Northwest
                  || aKeyEvent.iCode == EStdKeyDevice10      //   : Extra KeyEvent supports diagonal event simulator wedge
                  || aKeyEvent.iCode == EKeyUpArrow          // North
                  || aKeyEvent.iCode == EKeyRightUpArrow     // Northeast
                  || aKeyEvent.iCode == EStdKeyDevice11 )    //   : Extra KeyEvent supports diagonal event simulator wedge
            {
            resp = EKeyWasConsumed;
            //the first time
            if ( !iList->HighLightEnabled() )
                {
                iList->SetHighLight( ETrue );
                currIndex = iList->Model()->NumberOfItems()-1;
                iList->ScrollToMakeItemVisible( currIndex );
                iList->SetCurrentItemIndex( currIndex );
                DrawNow();
                //save the original content
                if ( iFirstGotoContent != NULL )
                    {
                    delete iFirstGotoContent;
                    iFirstGotoContent = NULL;
                    }
                iFirstGotoContent = iEditor->GetTextInHBufL();
                if (iFirstGotoContent == NULL)
                    {
                    iFirstGotoContent = KNullDesC().AllocL();
                    }
                }
            else
                {
                if(  iSearchFeature )
                    {
                    if( iList->CurrentItemIndex() == 0 )
                        {
                        resp = EKeyWasConsumed;
                        return resp;
                        }
                    }
                resp=iList->OfferKeyEventL( aKeyEvent, aType );
                }
            if ( currIndex > -1 )
                {
                //copy the selected item to edit field
                TPtrC16 item;
                TPtrC rightSide;
                TInt newIndex;
                newIndex = iItems->Count() - iList->CurrentItemIndex() - 1;
                item.Set( (*iItems)[ newIndex ] );
                //cut the slash from the end if needed
                //permanent sollution until AHLE does not
                //solve this problem related to ID: TMCN-5TTRXV error

                //if URL completion, then we want to append the item to the item in the edit field
                if (iUrlCompletionMode)
                    {
                    TPtr16 ptr = iFirstGotoContent->Des();

                    HBufC* buf = HBufC::NewLC(ptr.Length() + item.Length() + 1);

                    buf->Des().Copy(ptr);
                    buf->Des().Append(item);
                    buf->Des().ZeroTerminate();


                    rightSide.Set(buf->Des());

                    iEditor->SetTextL( &rightSide );
                    iEditor->SetCursorPosL( rightSide.Length(), EFalse );

                    CleanupStack::PopAndDestroy(); //buf
                    }
                else
                    {
                    if ( (*iItems)[ newIndex ][(*iItems)[ newIndex ].Length() - 1] != KSlash )
                        {
                        rightSide.Set( item.Left( item.Length() ) );
                        }
                    else
                        {
                        rightSide.Set( item.Left( item.Length() - 1 ) );
                        }
                    iEditor->SetTextL( &rightSide );
                    iEditor->SetCursorPosL( rightSide.Length(), EFalse );
                    }
                delete iPrevGotoContent;
                iPrevGotoContent = NULL;
                iPrevGotoContent = item.AllocL();
                iEditor->DrawNow();
                //change the CBA depending on whether the highlight is
                //on url or domain
                if ( ( (*iItems)[ newIndex ][(*iItems)[ newIndex ].Length() - 1] != KSlash ) && iDirectoryMode )
                    {
                    iOpenCBA = ETrue;
                    }
                else
                    {
                    iOpenCBA = EFalse;
                    }
                CBrowserAppUi::Static()->UpdateCbaL();
                }
            }

        //the joystisk is pressed down
        else if (    aKeyEvent.iCode == EKeyLeftDownArrow    // Southwest
                  || aKeyEvent.iCode == EStdKeyDevice13      //   : Extra KeyEvent supports diagonal event simulator wedge
                  || aKeyEvent.iCode == EKeyDownArrow        // South
                  || aKeyEvent.iCode == EKeyRightDownArrow   // Southeast
                  || aKeyEvent.iCode == EStdKeyDevice12 )    //   : Extra KeyEvent supports diagonal event simulator wedge
            {
            resp = EKeyWasConsumed;
            if(  iSearchFeature )
                {
                if ( iPrevItem <= 0 )
                    {
                    if(! iList->HighLightEnabled() )
                        {
                        resp = EKeyWasNotConsumed;
                        return resp;
                        }
                    }
                }
            //it was the last item
            if ( iPrevItem == 0 )
                {
                //write back the original string
                iList->SetHighLight( EFalse );
                iEditor->SetTextL( iFirstGotoContent );
                iEditor->HandleTextChangedL();
                iEditor->SetCursorPosL( iFirstGotoContent->Length(), EFalse );
                delete iPrevGotoContent;
                iPrevGotoContent = NULL;
                iPrevGotoContent = iFirstGotoContent->AllocL();
                iEditor->DrawNow();
                DrawNow();
                //change the CBA
                if ( iOpenCBA )
                    {
                    iOpenCBA = EFalse;
                    CBrowserAppUi::Static()->UpdateCbaL();
                    }
                return resp;
                }
            else if ( ( currIndex > -1 ) && iList->HighLightEnabled() )
                {
                //write the selected items string to the editor
                TPtrC16 item;
                TPtrC rightSide;
                TInt newIndex;
                resp = iList->OfferKeyEventL( aKeyEvent, aType );
                newIndex = iItems->Count() - iList->CurrentItemIndex() - 1;
                item.Set( (*iItems)[ newIndex ] );
                //cut the slash from the end if needed
                //permanent sollution until AHLE does not
                //solve this problem related to ID: TMCN-5TTRXV error
                if ( (*iItems)[ newIndex ][(*iItems)[ newIndex ].Length() - 1] != KSlash )
                    {
                    rightSide.Set( item.Left( item.Length() ) );
                    }
                else
                    {
                    rightSide.Set( item.Left( item.Length() - 1 ) );
                    }

                //if true then append the suffix (rightSide), to the
                //user-typed data
                //else just use the recent URLs text(rightSide) only
                if ( iUrlCompletionMode )
                    {
                    HBufC* currentCompletion = HBufC::NewLC(
                        iFirstGotoContent->Des().Length() + rightSide.Length() + 1 );
                    TPtr currentCompletionPtr = currentCompletion->Des();

                    currentCompletionPtr.Copy( iFirstGotoContent->Des() );
                    currentCompletionPtr.Append( rightSide );
                    currentCompletionPtr.ZeroTerminate();
                    iEditor->SetTextL( currentCompletion );
                    iEditor->SetCursorPosL( currentCompletionPtr.Length(), EFalse );
                    CleanupStack::PopAndDestroy( currentCompletion );
                    }
                else
                    {
                    iEditor->SetTextL( &rightSide );
                    iEditor->SetCursorPosL( rightSide.Length(), EFalse );
                    }
                iEditor->HandleTextChangedL();

                if ( iPrevGotoContent != NULL )
                    {
                    delete iPrevGotoContent;
                    iPrevGotoContent = NULL;
                    }
                iPrevGotoContent = item.AllocL();
                iEditor->DrawNow();
                //change the CBA depending on whether the highlight is
                //on url or domain
                if ( ( (*iItems)[ newIndex ][(*iItems)[ newIndex ].Length() - 1] != KSlash ) && iDirectoryMode )
                    {
                    iOpenCBA = ETrue;
                    }
                else
                    {
                    iOpenCBA = EFalse;
                    }
                CBrowserAppUi::Static()->UpdateCbaL();
                }
            }

        if ( iList->HighLightEnabled() )
            {
            iPrevItem = currIndex;
            }
        }
    return resp;
    }


// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::HidePopupL
// Hides the popup list
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::HidePopupL()
    {
    //clear the lists
    if ( iPoppedUp )
        {
        HBufC* newText = HBufC::NewLC( iEditor->TextLength() );
        TPtr newTextDes = newText->Des();
        iEditor->GetText( newTextDes );
        iEditor->ClearSelectionL();
        iEditor->SetTextL( newText );
        iEditor->SetCursorPosL( iEditor->TextLength(), EFalse );
        CleanupStack::PopAndDestroy( newText );
        iItemNames->Reset();
        iList->Reset();
        if (!IsActivated())
            {
            ActivateL();
            }
        MakeVisible(EFalse);
        DrawNow();
        iPrevItem = -2;
        iPoppedUp = EFalse;
        // highlight a directory, cancel goto pane
        // activate goto pane again -> softkey problem
        iOpenCBA = EFalse;
        CBrowserAppUi::Static()->UpdateCbaL();
        }
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::IsInDirectoryMode
// gives back the directory mode
// ---------------------------------------------------------
//
TBool CBrowserAdaptiveListPopup::IsInDirectoryMode()
    {
    return iDirectoryMode;
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::SetDirectoryMode
// Sets the directory mode
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::SetDirectoryModeL(TBool aDirMode)
    {
    iDirectoryMode = aDirMode;
    if ( aDirMode == EFalse )
        {
        //put the slash to the back of the editor fileds string
        //it will show the directory mode
        TInt length;
        length = iEditor->TextLength() + 1;
        HBufC* newText = HBufC::NewLC( length );
        TPtr newTextDes = newText->Des();
        iEditor->GetText( newTextDes );
        newTextDes.Append( TChar(KSlash) );
        iEditor->SetTextL( newText );
        iEditor->SetCursorPosL( length, EFalse );
        CleanupStack::PopAndDestroy( newText );
        iEditor->DrawNow();
        ShowPopupListL();
        //change the CBA
        if ( iOpenCBA )
            {
            iOpenCBA = EFalse;
            CBrowserAppUi::Static()->UpdateCbaL();
            }
        }
    return;
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::IsOpenDirToShow
// Show which CBA is about to show
// ---------------------------------------------------------
//
TBool CBrowserAdaptiveListPopup::IsOpenDirToShow()
{
    return iOpenCBA;
}

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::IsPoppedUp
// Show whether the list is popped up or not
// ---------------------------------------------------------
//
TBool CBrowserAdaptiveListPopup::IsPoppedUp()
{
    return iPoppedUp;
}


// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::HandleResourceChange
// If new skin is set the icons will be reloaded
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::HandleResourceChange( TInt aType )
    {
    TRAP_IGNORE( HandleResourceChangeL( aType ) );
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::HandleResourceChangeL
// If new skin is set the icons will be reloaded
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::HandleResourceChangeL( TInt aType )
    {
    if (( aType == KAknsMessageSkinChange )||
       ( aType == KEikDynamicLayoutVariantSwitch ) )
        {
        CArrayPtr<CGulIcon>* array =
                            iList->ItemDrawer()->ColumnData()->IconArray();

        array->ResetAndDestroy();
        delete array;
        SetIconsL();
        }

    CCoeControl::HandleResourceChange(aType);
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect  rect;
        if (AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect))
            {
            SetRect(rect);
            }
        }

    if ( ( aType == KEikDynamicLayoutVariantSwitch ) && ( iPoppedUp ) )
        {

        if ( AknLayoutUtils::PenEnabled() )
            {
            SetOrdinalPosition( 0 );
            }

        TInt selectionIndex = -1;
        HBufC* originalText = NULL;
        TInt popitem( 0 );
        if ( iList->HighLightEnabled() )
            {
            selectionIndex = iList->CurrentItemIndex();
            //save the editors content
            originalText = iEditor->GetTextInHBufL();
            if (originalText == NULL)
                {
                originalText = KNullDesC().AllocL();
                }
            CleanupStack::PushL( originalText );
            ++popitem;
            //write back the first string
            iEditor->SetTextL( iFirstGotoContent );
            iEditor->HandleTextChangedL();
            }
        ShowPopupListL(ETrue);
        if ( selectionIndex > -1)
            {
            iEditor->SetTextL( originalText );
            iEditor->SetCursorPosL( originalText->Length(), EFalse );
            iEditor->HandleTextChangedL();
            iList->SetHighLight( ETrue );
            iList->ScrollToMakeItemVisible( selectionIndex );
            iList->SetCurrentItemIndex( selectionIndex );
            DrawNow();
            iEditor->DrawNow();
            }
        if ( popitem )
            {
            CleanupStack::PopAndDestroy( popitem );  // originalText
            }
        }
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::SetIconsL()
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::SetIconsL()
{
    CArrayPtr<CGulIcon>* icons =
    new(ELeave) CAknIconArray( KMaxIcons );
    CleanupStack::PushL(icons); // PUSH Icon
    CGulIcon* newIcon;
    CFbsBitmap *newIconBmp;
    CFbsBitmap *newIconMaskBmp;

    TParse* fp = new(ELeave) TParse();
    CleanupStack::PushL(fp);
    TInt err = fp->Set(KBrowserDirAndFile, &KDC_APP_BITMAP_DIR, NULL);
    if (err != KErrNone)
        {
        User::Leave(err);
        }
    TBuf<KMaxFileName> iconFile= fp->FullName();
    CleanupStack::PopAndDestroy(fp);

    MAknsSkinInstance* skinInstance = AknsUtils::SkinInstance();
    AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlFolderAdap,
                                           newIconBmp,
                                           newIconMaskBmp,
                                           iconFile,
                                           EMbmBrowserQgn_prop_wml_folder_adap,
                                           EMbmBrowserQgn_prop_wml_folder_adap_mask);
    newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
    CleanupStack::PushL(newIcon);
    icons->AppendL( newIcon );
    CleanupStack::Pop(newIcon);
    AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlBmAdap,
                                           newIconBmp,
                                           newIconMaskBmp,
                                           iconFile,
                                           EMbmBrowserQgn_prop_wml_bm_adap,
                                           EMbmBrowserQgn_prop_wml_bm_adap_mask);
    newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
    CleanupStack::PushL(newIcon);
    icons->AppendL( newIcon );
    CleanupStack::Pop(newIcon);
    iList->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
    CleanupStack::Pop();    // POP Icon
}

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::SetUrlSuffixList()
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::SetUrlSuffixList(HBufC* urlSuffixList)
    {
    iUrlSuffixBuf = urlSuffixList;
    }

// ---------------------------------------------------------
// CBrowserAdaptiveListPopup::SetMaxRecentUrls()
// ---------------------------------------------------------
//
void CBrowserAdaptiveListPopup::SetMaxRecentUrls (TInt maxRecentUrls)
    {
    iMaxRecentUrlsToShow = maxRecentUrls;
    }


// End of File
