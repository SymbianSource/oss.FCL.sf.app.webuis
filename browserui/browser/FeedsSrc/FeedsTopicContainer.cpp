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
* Description:  A view to browse a feed's topics.
*
*/



#include <AknNavi.h>
#include <AknNaviDe.h>
#include <AknNaviLabel.h>
#include <AknTabGrp.h>
#include <AknViewAppUi.h>
#include <akniconarray.h>
#include <aknconsts.h>
#include <aknsconstants.h>
#include <aknlists.h>
#include <aknsutils.h>
#include <data_caging_path_literals.hrh>
#include <eikclbd.h>
#include <gulicon.h>
#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include <csxhelp/browser.hlp.hrh>
#include "BrowserApplication.h"
#endif // __SERIES60_HELP

#include <browser.mbg>
#include "Browser.hrh"
#include <BrowserNG.rsg>
#include <feedattributes.h>
#include <folderattributes.h>
#include <feedsentity.h>
#include "FeedsTopicContainer.h"
#include "FeedsTopicView.h"

#include "ApiProvider.h"
#include "Display.h"


const TInt KDateSize = 30;          // Size of Date strings
const TInt KTimeSize = 30;          // Size of Time strings


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsTopicContainer* CFeedsTopicContainer::NewL(CFeedsTopicView* aView,
            MApiProvider& aApiProvider, const TRect& aRect)
    {
    CFeedsTopicContainer* self = new (ELeave) CFeedsTopicContainer(aView, aApiProvider);
    
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    CleanupStack::Pop();

    return self;
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::CFeedsTopicContainer
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsTopicContainer::CFeedsTopicContainer(CFeedsTopicView* aView,
                             MApiProvider& aApiProvider ) :
    iView( aView ),
    iApiProvider( aApiProvider )
    {
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::ConstructL(const TRect& aRect)
    {
    CreateWindowL();
	SetMopParent( iView );
    InitContainerL(aRect);
    ActivateL();
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::~CFeedsTopicContainer
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsTopicContainer::~CFeedsTopicContainer()
    {
    delete iListBox;
    delete iListBoxRows;
	delete iNaviDecorator;
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::OfferKeyEventL
//
// Handles key event.
// -----------------------------------------------------------------------------
//
TKeyResponse CFeedsTopicContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    if (aType != EEventKey) 
        {
        return EKeyWasNotConsumed;
        }
        
    if (( aKeyEvent.iCode == EKeyOK || aKeyEvent.iCode == EKeyEnter ) &&
        ( aKeyEvent.iModifiers & EModifierShift ) )
        {
        return EKeyWasConsumed;
        }
    
    return iListBox->OfferKeyEventL(aKeyEvent, aType);
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::GetHelpContext
//
// Get help context for the control.
// -----------------------------------------------------------------------------
//
#ifdef __SERIES60_HELP
void CFeedsTopicContainer::GetHelpContext(TCoeHelpContext& aContext) const
    {
    // This must be the Browser's uid becasue the help texts are under Browser topics.
    aContext.iMajor = KUidBrowserApplication;
    aContext.iContext = KOSS_HLP_RSS_ARTICLELST;
    }
#endif // __SERIES60_HELP


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::SizeChanged
//
// Called by framework when the view size is changed.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::SizeChanged()
    {
    iListBox->SetRect(Rect());
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::HandleResourceChange
//
// Called by the framework when a display resource changes (i.e. skin or layout).
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);
    iListBox->HandleResourceChange(aType);
    
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect  rect;
        
        if (AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect))
            {
            SetRect(rect);
            }
        }
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::CountComponentControls
//
// Returns number of components.
// -----------------------------------------------------------------------------
//
TInt CFeedsTopicContainer::CountComponentControls() const
    {
    return 1;
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::ComponentControl
//
// Returns pointer to particular component.
// -----------------------------------------------------------------------------
//
CCoeControl* CFeedsTopicContainer::ComponentControl(TInt aIndex) const
    {
    switch (aIndex)
        {
        case 0:
            return iListBox;

        default:
            return NULL;
        }
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::HandleListBoxEventL
//
// Processes key events from the listbox.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
    {
    // An item was selected.
    if ((aEventType == MEikListBoxObserver::EEventEnterKeyPressed) ||
        (aEventType == MEikListBoxObserver::EEventItemDoubleClicked))
        {
        HandleOpenL();
        }
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::SetCurrentFeedL
//
// Sets the current feed
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::SetCurrentFeedL(const CFeedsEntity& aFeed, TInt aIndex)
    {
    // If the feed changes ignore aIndex.
    if (iFeed != &aFeed)
        {
        aIndex = 0;
        }
        
    iFeed = &aFeed;

    // Update the view.
    UpdateTitleL();
    UpdateListBoxL();
    UpdateNavigationPaneL();

    iListBox->SetCurrentItemIndex(aIndex);
    iListBox->DrawDeferred();
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::CurrentIndex
//
// Returns the current item.
// -----------------------------------------------------------------------------
TInt CFeedsTopicContainer::CurrentIndex() const
    {
    return iListBox->CurrentItemIndex();
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::ItemCount
//
// Returns the number of items.
// -----------------------------------------------------------------------------
//
TInt CFeedsTopicContainer::ItemCount() const
    {
    return iFeed->GetChildren().Count();
    }
    

// -----------------------------------------------------------------------------
// CFeedsTopicContainer::UpdateTitleL
//
// Update the view's title.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::UpdateTitleL()
    {

    // Set the view's title
    TPtrC title;
    iFeed->GetStringValue(EFeedAttributeTitle,title);
    iApiProvider.Display().SetTitleL(title);
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::HandleOpenL
//
// Handle the open command.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::HandleOpenL()
    {
    TInt index;

    // Get the index of the item to show.
    index = iListBox->CurrentItemIndex();

    // Show the FeedView.
    iApiProvider.FeedsClientUtilities().ShowFeedViewLocalL(index);
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::HandleUpdateFeedL
//
// Handles updating the current Feed.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::HandleUpdateFeedL(void)
    {
    iApiProvider.FeedsClientUtilities().UpdateFeedL(*iFeed);
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::InitContainerL
//
// Inits the container.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::InitContainerL(const TRect& aRect)
    {
    iListBox = new (ELeave) CAknSingleGraphicStyleListBox;
    //iListBox = new (ELeave) CAknDoubleGraphicStyleListBox;
    //iListBox = new (ELeave) CAknSingleStyleListBox;
    
    // below flags equals EAknListBoxMarkableList without the EAknListBoxShiftEnterMarks 
    // so the shift key press is not handled by avkon
    iListBox->ConstructL(this, (EAknGenericListBoxFlags | EAknListBoxMultipleSelection));
    iListBox->SetContainerWindowL(*this);
    iListBox->SetRect(aRect.Size());
    iListBox->SetListBoxObserver(this);

    iListBoxRows = new (ELeave) CDesCArraySeg(10);
    iListBox->Model()->SetItemTextArray(iListBoxRows);
    iListBox->Model()->SetOwnershipType(ELbmDoesNotOwnItemArray);

    iListBox->ActivateL();
    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto);

    // Add the needed icons.
    InitIconArrayL();

    // Enable marquee.
	iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
    }
    

// -----------------------------------------------------------------------------
// CFeedsTopicContainer::InitIconArrayL
//
// Inits the array of needed icons.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::InitIconArrayL()
    {
    _LIT(KDirAndFile,"z:browser.mbm");
    
    TParse*               fp = NULL;
    CArrayPtr<CGulIcon>*  icons = NULL;
    
    // Build the path to the feeds resource file.
    fp = new (ELeave) TParse();     
    CleanupStack::PushL(fp);
    
    User::LeaveIfError(fp->Set(KDirAndFile, &KDC_APP_BITMAP_DIR, NULL)); 
    TBuf<KMaxFileName> iconFile= fp->FullName();
    CleanupStack::PopAndDestroy(fp);

    // Create the icon array.
    icons = new (ELeave) CAknIconArray(1);
    CleanupStack::PushL(icons);

    // Add the unread icon
    AppendIconL(icons, KAknsIIDQgnPropFileRssUnread, iconFile,
            EMbmBrowserQgn_prop_file_rss_unread, EMbmBrowserQgn_prop_file_rss_unread_mask);

    // Add the read icon
    AppendIconL(icons, KAknsIIDQgnPropFileRssRead, iconFile,
            EMbmBrowserQgn_prop_file_rss_read, EMbmBrowserQgn_prop_file_rss_read_mask);

    // Set the icons and cleanup
    iListBox->ItemDrawer()->ColumnData()->SetIconArray(icons);
	CleanupStack::Pop(icons);
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::AppendIconL
//
// Loads and appends an icon to the icon array.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::AppendIconL(CArrayPtr<CGulIcon>* aIcons, 
        const TAknsItemID& aID, const TDesC& aFilename, const TInt aFileBitmapId, 
        const TInt aFileMaskId)
    {
	CGulIcon*    newIcon;
	CFbsBitmap*  newIconBmp;
	CFbsBitmap*  newIconMaskBmp;

    // Create the bitmaps: the bitmaps are left on the cleanup stack.
	AknsUtils::CreateIconL(AknsUtils::SkinInstance(), aID, newIconBmp, 
            newIconMaskBmp, aFilename, aFileBitmapId, aFileMaskId);

    CleanupStack::PushL(newIconBmp);
    CleanupStack::PushL(newIconMaskBmp);

	newIcon = CGulIcon::NewL(newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(newIconMaskBmp);
    CleanupStack::Pop(newIconBmp);
    CleanupStack::PushL(newIcon);

	aIcons->AppendL(newIcon);
    CleanupStack::Pop(newIcon);
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::UpdateListBoxL
//
// Sets the list-box's values.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::UpdateListBoxL()
    {
    TBuf<255>            listBoxtxt;
    
    // Clear the list.
    iListBoxRows->Reset();

    if (iFeed == NULL)
        {
        return;
        }

    // Add the items.
    for (int i = 0; i < iFeed->GetChildren().Count(); i++)
        {
        CFeedsEntity*  item = iFeed->GetChildren()[i];
        TPtrC title;
        TPtrC desc;
		TDesC* itemName;
		TInt status;	

		item->GetStringValue(EItemAttributeTitle,title);
		item->GetStringValue(EItemAttributeDescription,desc);
		item->GetIntegerValue(EItemAttributeStatus,status);
        // Get the title.
        itemName = &(title);
        
        // If the title is missing use the description instead.
        if (itemName->Length() == 0)
            {
            itemName = &(desc);
            }

        // Ensure the title isn't too long.
        TInt len = itemName->Length();
        if (len > 253)
            {
            len = 253;
            }

        // Format: icon-index\t\1st Line of text\t2nd Line of text\t0
        listBoxtxt.Zero();
        
        switch (status)
            {
            // unread and new item all share the same unread icon
            case EItemStatusNew:
                listBoxtxt.Append(_L("0\t"));
                break;

            case EItemStatusUnread:
                listBoxtxt.Append(_L("0\t"));
                break;
            
            default:
                listBoxtxt.Append(_L("1\t"));
                break;
            }
            
        listBoxtxt.Append(itemName->Ptr(), len);
        listBoxtxt.Append(KNullDesC());

        iListBoxRows->AppendL(listBoxtxt);
        }

    iListBox->SetCurrentItemIndex(0);
    iListBox->HandleItemAdditionL();
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::Clear
//
// Clears the navigation pane.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::ClearNavigationPane()
    {
    delete iNaviDecorator;
    iNaviDecorator = NULL;
    }


// -----------------------------------------------------------------------------
// CFeedsTopicContainer::UpdateNavigationPaneL
//
// Handles the changes needed to the Navigation Pane.
// -----------------------------------------------------------------------------
//
void CFeedsTopicContainer::UpdateNavigationPaneL()
    {
    CAknNavigationControlContainer*  naviPane = NULL;

    if (!iFeed)
        {
        return;
        }

    // Convert the feed's timestamp into a localized string.
    TBuf<KDateSize + KTimeSize + 1>  timestamp;
    TBuf<KTimeSize>  temp;    
    TTime  date;                       
	iFeed->GetTimeValue(EFeedAttributeTimestamp,date);

    // Translate from UTC to local time.
    TTime                 local;
    TTime                 utc;
    TTimeIntervalSeconds  delta;
    
    local.HomeTime();
    utc.UniversalTime();
    utc.SecondsFrom(local, delta);    
    date -= delta;

    //Set time
    HBufC* timeFormat = iEikonEnv->AllocReadResourceLC( R_QTN_TIME_USUAL_WITH_ZERO );
    date.FormatL( timestamp, *timeFormat );
    CleanupStack::PopAndDestroy( timeFormat );//timeFormat
    //
    timestamp.Append(_L("  "));                    
    //Set date
    HBufC* dateFormat = iEikonEnv->AllocReadResourceLC( R_QTN_DATE_USUAL_WITH_ZERO );
    date.FormatL( temp, *dateFormat );
    CleanupStack::PopAndDestroy( dateFormat );//dateFormat
    //
    timestamp.Append(temp);
    AknTextUtils::LanguageSpecificNumberConversion(timestamp);

    // Get the navigation sub-pane.
    CAknViewAppUi*  appUi;
    TUid            uid;

    // Get the title sub-pane.
    appUi = static_cast<CAknViewAppUi*>(CCoeEnv::Static()->AppUi());

    uid.iUid = EEikStatusPaneUidNavi;

    CEikStatusPaneBase::TPaneCapabilities subPane = appUi->StatusPane()->
            PaneCapabilities(uid);

    // Set the title if the pane belongs to the app.
	if (subPane.IsPresent() && subPane.IsAppOwned())
		{
		naviPane = (CAknNavigationControlContainer*) appUi->StatusPane()->ControlL(uid);
        }
    else
        {
        User::Leave(KErrNotSupported);
        }

    // Ensure the NaviDecorator was created.
    if (!iNaviDecorator)
        {
        iNaviDecorator = naviPane->CreateNavigationLabelL( timestamp );
        }


	// If not yet pushed, this will do the push; if already there, this brings
    // it to top and draws.
    naviPane->PushL(*iNaviDecorator);
    }




