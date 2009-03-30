/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  A view to browse a given feed.
*
*/


#include <AknNavi.h>
#include <AknNaviDe.h>
#include <AknTabGrp.h>
#include <AknNaviLabel.h>
#include <aknutils.h>
#include <AknViewAppUi.h>
#include <BrCtlInterface.h>
#include <CharConv.H>
#include <StringLoader.h>

#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include <csxhelp/browser.hlp.hrh>
#include "BrowserApplication.h"
#endif // __SERIES60_HELP

#include "Browser.hrh"
#include <BrowserNG.rsg>
#include "FeedsFeedContainer.h"
#include "FeedsFeedView.h"
#include "XmlEncoding.h"
#include "ApiProvider.h"
#include "BrowserSpecialLoadObserver.h"
#include "Display.h"
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
#include "BrowserAppUi.h"
#include <FeedAttributes.h>
#include <FolderAttributes.h>
#include <FeedsEntity.h>

#include "eikon.hrh"


_LIT(KFeedsSchema, "feeds:");
_LIT(KFeedsNavSchema, "feedsnav:");
_LIT(KNext, "next");
_LIT(KTemplate, "feeds_view_template.html");
_LIT(KTokenTitle, "#Title#");
_LIT(KTokenWebUrl, "#WebUrl#");
_LIT(KTokenDate, "#Date#");
_LIT(KTokenDescription, "#Description#");
_LIT(KTokenEnclosure, "#Enclosure#");
_LIT(KTokenShowPrev, "#ShowPrev#");
_LIT(KTokenShowNext, "#ShowNext#");


const TInt KDateSize = 30;          // Size of Date strings
const TInt KTimeSize = 30;          // Size of Time strings


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsFeedContainer* CFeedsFeedContainer::NewL(CFeedsFeedView* aView,
            MApiProvider& aApiProvider)
    {
    CFeedsFeedContainer* self = new (ELeave) CFeedsFeedContainer(aView, aApiProvider);

    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::CFeedsFeedContainer
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsFeedContainer::CFeedsFeedContainer(CFeedsFeedView* aView,
                             MApiProvider& aApiProvider ) :
    iView( aView ),
    iApiProvider( aApiProvider ),
    iBrowserControl(0)
    {
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::ConstructL()
    {
    EnsureTemplateL(KTemplate);
    LoadTemplateL(KTemplate);

    CreateWindowL();
    SetMopParent( iView );
    //SetRect(aRect);
    ActivateL();

    TRect rect(Position(), Size());

    iBrowserControl = CreateBrowserControlL(this, rect,
            TBrCtlDefs::ECapabilityDisplayScrollBar | TBrCtlDefs::ECapabilityLoadHttpFw,
            TBrCtlDefs::ECommandIdBase, NULL,  NULL, this, NULL, NULL, NULL, NULL);
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::~CFeedsFeedContainer
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsFeedContainer::~CFeedsFeedContainer()
    {
    delete iTemplate;
    delete iNaviPaneTabsGroup;
    delete iBrowserControl;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::OfferKeyEventL
//
// Handles key event.
// -----------------------------------------------------------------------------
//
TKeyResponse CFeedsFeedContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {

    TKeyResponse  response = EKeyWasConsumed;

    if (aType == EEventKey)
        {
        switch (aKeyEvent.iCode)
            {
            case EKeyLeftUpArrow:     // Northwest
            case EStdKeyDevice10:     //   : Extra KeyEvent supports diagonal event simulator wedge
            case EKeyLeftArrow:       // West
            case EKeyLeftDownArrow:   // Southwest
            case EStdKeyDevice13:     //   : Extra KeyEvent supports diagonal event simulator wedge
                    {
                    ShowPrevItemL();
                    }
                return response;

            case EKeyRightUpArrow:    // Northeast
            case EStdKeyDevice11:     //   : Extra KeyEvent supports diagonal event simulator wedge
            case EKeyRightArrow:       // East
            case EKeyRightDownArrow:  // Southeast
            case EStdKeyDevice12:     //   : Extra KeyEvent supports diagonal event simulator wedge
                    {
                    ShowNextItemL();
                    }
                return response;

            }
        }

    response = iBrowserControl->OfferKeyEventL(aKeyEvent, aType);

    // now "simulate" another key event for proper handling of middle-softkey
    if ( (aKeyEvent.iScanCode == EStdKeyDevice3) && (aType == EEventKeyDown) )
        {
        TKeyEvent keyEvent;
        keyEvent.iCode = 0xf845;
        keyEvent.iModifiers = 1;
        keyEvent.iRepeats = 0;
        keyEvent.iCode = EKeyDevice3;
        keyEvent.iScanCode = EStdKeyDevice3;
        response = iBrowserControl->OfferKeyEventL( keyEvent, EEventKey );
    }
    return response;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::GetHelpContext
//
// Get help context for the control.
// -----------------------------------------------------------------------------
//
#ifdef __SERIES60_HELP
void CFeedsFeedContainer::GetHelpContext(TCoeHelpContext& aContext) const
    {
    // This must be the Browser's uid becasue the help texts are under Browser topics.
    aContext.iMajor = KUidBrowserApplication;
    aContext.iContext = KOSS_HLP_RSS_ARTICLE;
    }
#endif // __SERIES60_HELP


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::MakeVisible
//
// Sets this control as visible or invisible.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::MakeVisible(TBool aVisible)
    {
    if (iBrowserControl)
        {
        iBrowserControl->MakeVisible(aVisible);
        }

    CCoeControl::MakeVisible(aVisible);
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::SizeChanged
//
// Called by framework when the view size is changed.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::SizeChanged()
    {
    if (iBrowserControl)
        {
        iBrowserControl->SetRect(Rect());
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::HandleResourceChange
//
// Called by the framework when a display resource changes (i.e. skin or layout).
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);
    iBrowserControl->HandleResourceChange(aType);

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
// CFeedsFeedContainer::CountComponentControls
//
// Returns number of components.
// -----------------------------------------------------------------------------
//
TInt CFeedsFeedContainer::CountComponentControls() const
    {
    TInt ctrls = 0;
    if ( iBrowserControl )
        {
        ctrls++;// Brctl
        }
    return ctrls;
    }

// -----------------------------------------------------------------------------
// CFeedsFeedContainer::NetworkConnectionNeededL
//
// Request to create a network connection.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::NetworkConnectionNeededL(TInt* aConnectionPtr,
        TInt* aSockSvrHandle, TBool* aNewConn, TApBearerType* aBearerType)
    {
    iApiProvider.SpecialLoadObserver().NetworkConnectionNeededL(aConnectionPtr, aSockSvrHandle,
            aNewConn, aBearerType);
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::HandleRequestL
//
// Request the host applicaion to handle non-http request.
// -----------------------------------------------------------------------------
//
TBool CFeedsFeedContainer::HandleRequestL(RArray<TUint>* aTypeArray, CDesCArrayFlat* aDesArray)
    {
    HBufC*  url = NULL;
    TBool   handled = ETrue;

    // The 2 arrays must be in sync. Each element in iTypeArray
    // identifies the type of the corresponding element in iDesArray.
    if (aTypeArray->Count() != aDesArray->Count())
        {
        User::Leave(KErrArgument);
        }

    // Get the url.
    for (TInt i = 0; i < aTypeArray->Count(); i++)
        {
        if ((*aTypeArray)[i] == EParamRequestUrl)
            {
            url = HBufC::NewLC((*aDesArray)[i].Length());
            url->Des().Copy((*aDesArray)[i]);
            break;
            }
        }

    // Leave if the url wasn't found.
    if (url == NULL)
        {
        User::Leave(KErrArgument);
        }

    // Handle the "feeds:" schema
    if (url->Find(KFeedsSchema) == 0)
        {
        // Extract the real url from the "feed" url.
        TPtrC  loadUrl(url->Ptr() + KFeedsSchema().Length(),
                url->Length() - KFeedsSchema().Length());

        // Handle the "feeds_nav:" schema.
        if (loadUrl.Find(KFeedsNavSchema) == 0)
            {
            // Get the direction
            TPtrC  dir(loadUrl.Ptr() + KFeedsNavSchema().Length(),
                    loadUrl.Length() - KFeedsNavSchema().Length());

            if (dir.FindF(KNext) == 0)
                {
                ShowNextItemL();
                }
            else
                {
                ShowPrevItemL();
                }
            }
        else
            {
            // Otherwise dispatch the url to the client.
            iApiProvider.FeedsClientUtilities().LoadUrlL(loadUrl);
            }
        }
    // Otherwise, send request to SchemeHandler through default BrCtlSpecialLoadObserver
    else
        {
        handled = iApiProvider.SpecialLoadObserver().HandleRequestL( aTypeArray, aDesArray );
        }

    CleanupStack::PopAndDestroy(url);
    return handled;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::HandleDownloadL
//
// Called when the browser control wants the host application (us) to handle
// downloaded content
// -----------------------------------------------------------------------------
//
TBool CFeedsFeedContainer::HandleDownloadL(RArray<TUint>* aTypeArray,
        CDesCArrayFlat* aDesArray)
    {
    HBufC*  url = NULL;
    TBool   handled = EFalse;

    //
    // Pass first to the main special load observer to be handled.
    // If it's not handled, continue here
    //
    handled = iApiProvider.SpecialLoadObserver().HandleDownloadL( aTypeArray, aDesArray );

    if (!handled)
        {
        // The 2 arrays must be in sync. Each element in iTypeArray
        // identifies the type of the corresponding element in iDesArray.
        if (aTypeArray->Count() != aDesArray->Count())
            {
            User::Leave(KErrArgument);
            }

        // Get the url.
        for (TInt i = 0; i < aTypeArray->Count(); i++)
            {
            if ((*aTypeArray)[i] == EParamRequestUrl)
                {
                url = HBufC::NewLC((*aDesArray)[i].Length());
                url->Des().Copy((*aDesArray)[i]);
                break;
                }
            }

        TPtrC pUrl(url->Des());

        // Leave if the url wasn't found.
        if (url == NULL)
            {
            User::Leave(KErrArgument);
            }
        else
            {
            // Otherwise dispatch the url to the client.
            iApiProvider.FeedsClientUtilities().SubscribeToL(KNullDesC, pUrl);
            handled = ETrue;
            }

        CleanupStack::PopAndDestroy(url);
        }
    return handled;
    }

// -----------------------------------------------------------------------------
// CFeedsFeedContainer::ComponentControl
//
// Returns pointer to particular component.
// -----------------------------------------------------------------------------
//
CCoeControl* CFeedsFeedContainer::ComponentControl(TInt aIndex) const
    {
    CCoeControl *ctrl = NULL;

    switch (aIndex)
        {
        case 0:
            {
            ctrl = iBrowserControl;
            break;
            }

        default:
            break;
        }

    return ctrl;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::SetCurrentFeedL
//
// Sets the current feed
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::SetCurrentFeedL(CFeedsEntity& aFeed, TInt aInitialItem)
    {
    iFeed = &aFeed;
    iCurrentItem = aInitialItem;

    // Update the view.
    if (iView->iContainerOnStack)
        {
        // Only do this if it's view is active.
        TPtrC title;
        iFeed->GetStringValue(EFeedAttributeTitle,title);
        if (title.Length() > 0)
            {
            iApiProvider.Display().SetTitleL( title );
            }
        else
            {
            iApiProvider.Display().SetTitleL( KNullDesC );
            }
        }

    ShowFeedItemL();
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::GetItemUrl
//
// Returns the current item's url.
// -----------------------------------------------------------------------------
//
const TDesC& CFeedsFeedContainer::GetItemUrl()
    {
    iFeed->GetChildren()[iCurrentItem]->GetStringValue(EItemAttributeLink,iUrl);
    return iUrl;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::CurrentItem
//
// Returns the index of the current item.
// -----------------------------------------------------------------------------
//
TInt CFeedsFeedContainer::CurrentItem()
    {
    return iCurrentItem;
    }

// -----------------------------------------------------------------------------
// CFeedsFeedContainer::ItemCount
//
// Returns the number of items.
// -----------------------------------------------------------------------------
TInt CFeedsFeedContainer::ItemCount() const
    {
    return iFeed->GetChildren().Count();
    }

// -----------------------------------------------------------------------------
// CFeedsFeedContainer::ShowNextItemL
//
// Shows the next item if possible.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::ShowNextItemL()
    {
    if (iFeed->GetChildren().Count() <= 1)
        {
        return;
        }

    if ((iCurrentItem + 1) < iFeed->GetChildren().Count())
        {
        iCurrentItem++;
        }
    else
        {
        iCurrentItem = 0;
        }

    ShowFeedItemL();
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::ShowPrevItemL
//
// Shows the prev item if possible.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::ShowPrevItemL()
    {
    if (iFeed->GetChildren().Count() <= 1)
        {
        return;
        }

    if ((iCurrentItem - 1) >= 0)
        {
        iCurrentItem--;
        }
    else
        {
        iCurrentItem = iFeed->GetChildren().Count() - 1;
        }

    ShowFeedItemL();
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::Clear
//
// Clears the navigation pane.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::ClearNavigationPane()
    {
    delete iNaviPaneTabsGroup;
    iNaviPaneTabsGroup = NULL;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::UpdateNavigationPaneL
//
// Handles the changes needed to the Navigation Pane.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::UpdateNavigationPaneL()
    {
    const TInt KTabId = 88888;
    const TInt KMaxNaviText = 25;   // format is "<int>/<int>".
    _LIT(KFormat, "%d/%d");

    CAknNavigationControlContainer*  naviPane = NULL;
    TBuf<KMaxNaviText>               buf;
    CAknTabGroup*                    tabGroup = NULL;
    TInt                             itemCount = 0;

    if (iFeed)
        {
        itemCount = iFeed->GetChildren().Count();
        }

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

    // Ensure the tab group was created.
    if (!iNaviPaneTabsGroup)
        {
        iNaviPaneTabsGroup = naviPane->CreateTabGroupL();
        }

    // Format Navi Pane text "index/count" style.
    buf.Format(KFormat, iCurrentItem + 1, itemCount);

    // Update the tab-group.
    tabGroup = static_cast<CAknTabGroup*>(iNaviPaneTabsGroup->DecoratedControl());

    // Already created, replacd the tab.
    if (tabGroup->TabCount() != NULL)
        {
        tabGroup->ReplaceTabL(KTabId, buf);
        }

    // Otherwise add the tab.
    else
        {
        tabGroup->AddTabL(KTabId, buf);
        }

    tabGroup->SetTabFixedWidthL(EAknTabWidthWithOneTab);
    tabGroup->SetActiveTabById(KTabId);

    // If not yet pushed, this will do the push; if already there, this brings
    // it to top and draws.
    naviPane->PushL(*iNaviPaneTabsGroup);
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::ShowFeedItemL
//
// Shows the given feed item.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::ShowFeedItemL()
    {
    _LIT(KSchema, "data:");
    _LIT8(KType, "text/html");

    const TInt   KInt64Length = 25;

    HBufC*        link = NULL;
    HBufC*        htmlTemplate = NULL;
    HBufC*        enclosureStr = NULL;
    TDataType     datatype(KType);
    TUid          uid;
    CFeedsEntity*  item = iFeed->GetChildren()[iCurrentItem];
    HBufC*        enclosure = NULL;
    TInt          enclosureLen = 0;

    // Mark the item as read.
    iApiProvider.FeedsClientUtilities().SetItemStatusL(iFeed->GetChildren()[iCurrentItem], EItemStatusRead);

    // Load the localized strings.
    enclosureStr = StringLoader::LoadLC(R_FEEDS_ENCLOSURE);

    // Convert the item's UTC timestamp into a localized string.
    TBuf<KDateSize + KTimeSize + 1>  timestamp;
    TBuf<KTimeSize>  temp;
    TTime ts;

    item->GetTimeValue(EItemAttributeTimestamp,ts);

    // Translate from UTC to local time.
    TTime                 local;
    TTime                 utc;
    TTimeIntervalSeconds  delta;

    local.HomeTime();
    utc.UniversalTime();
    utc.SecondsFrom(local, delta);
    ts -= delta;

    // Create the localized time string.
    //Set time
    HBufC* timeFormat = iEikonEnv->AllocReadResourceLC( R_QTN_TIME_USUAL_WITH_ZERO );
    ts.FormatL( timestamp, *timeFormat );
    CleanupStack::PopAndDestroy( timeFormat );//timeFormat
    //
    timestamp.Append(_L("  "));
    //Set date
    HBufC* dateFormat = iEikonEnv->AllocReadResourceLC( R_QTN_DATE_USUAL_WITH_ZERO );
    ts.FormatL( temp, *dateFormat );
    CleanupStack::PopAndDestroy( dateFormat );//dateFormat
    //
    timestamp.Append(temp);

    // Create the enclosure string, which is a a series of the following string.
    // <div><a href="theUrl">Enclosure[theContentType]: theSize MB</a></div>
    _LIT(KEnclosureMarkup, "<div><a href=\"%S\">%S[%S MB]: %S</a></div>");

    TBuf16<KInt64Length>  size;
    TInt64                sizeInt;
    TReal64               sizeReal;
    TRealFormat           format;
    TPtrC                 url;
    TPtrC                 contentType;
    TPtrC                 len;
    TPtrC                 title;
    TPtrC                 desc;

    format.iType = KRealFormatFixed;
    format.iPlaces = 2;

    // Determine how large the enclosure string needs to be.
    for (TInt i = 0; i < item->GetChildren().Count(); i++)
        {
        CFeedsEntity* en = item->GetChildren()[i];

        en->GetStringValue(EEnclosureAttributeContentType,contentType);
        en->GetStringValue(EEnclosureAttributeSize,len);
        en->GetStringValue(EEnclosureAttributeLink,url);
        en->GetStringValue(EEnclosureAttributeTitle,title);
        TLex16 lex(len);
        // Convert the enclosure size to mega-bytes.
        lex.Val(sizeInt);
        sizeReal = sizeInt / 1000000.0;

        size.Zero();
        size.AppendNum(sizeReal, format);

        enclosureLen += KEnclosureMarkup().Length();
        enclosureLen += url.Length();
        enclosureLen += enclosureStr->Length();
        enclosureLen += size.Length();
        enclosureLen += contentType.Length();
        }

    // Allocate the enclosure string.
    enclosure = HBufC::NewLC(enclosureLen);

    // Construct the enclosure string.
    for (TInt i = 0; i < item->GetChildren().Count(); i++)
        {
        CFeedsEntity* en = item->GetChildren()[i];

        en->GetStringValue(EEnclosureAttributeContentType,contentType);
        en->GetStringValue(EEnclosureAttributeSize,len);
        en->GetStringValue(EEnclosureAttributeLink,url);
        en->GetStringValue(EEnclosureAttributeTitle,title);
        TLex16             lex(len);

        // Convert the enclosure size to mega-bytes.
        lex.Val(sizeInt);
        sizeReal = sizeInt / 1000000.0;

        size.Zero();
        size.AppendNum(sizeReal, format);

        enclosure->Des().AppendFormat(KEnclosureMarkup, &(url), enclosureStr,
                &size, &(contentType));
        }

    // Load and prepare the html template.
    item->GetStringValue(EItemAttributeTitle,title);
    item->GetStringValue(EItemAttributeDescription,desc);
    item->GetStringValue(EItemAttributeLink,url);

    htmlTemplate = ResolveTemplateL(title, timestamp, desc,
            url, *enclosure);


    CleanupStack::PushL(htmlTemplate);
    iFeed->GetStringValue(EFeedAttributeTitle,title);
    // Load the htmlTemplate in the browser control.
    uid.iUid = KCharacterSetIdentifierUcs2;

    link = HBufC::NewLC(KSchema().Length() + title.Length());
    link->Des().Copy(KSchema);
    link->Des().Append(title);

    TPtrC8  ptr((const TUint8*) htmlTemplate->Ptr(), htmlTemplate->Size());
    iBrowserControl->LoadDataL(*link, ptr, datatype, uid);

    CleanupStack::PopAndDestroy(link);
    CleanupStack::PopAndDestroy(htmlTemplate);
    CleanupStack::PopAndDestroy(enclosure);
    CleanupStack::PopAndDestroy(enclosureStr);

    // Update the nav-pane.
    UpdateNavigationPaneL();
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::LoadTemplateL
//
// Loads the template html file.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::LoadTemplateL(const TDesC& aTemplateName)
    {
    RFs                 rfs;
    RFile               file;
    TInt                size;
    TBuf<KMaxFileName>  path;
    TUint               encoding;
    HBufC8*             buff;
    CXmlEncoding*       xmlEncoding = NULL;
    TInt                loc;

    // Build the path to the file and open the file.
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs);

    path.Append(_L("\\"));
    path.Append(aTemplateName);

    User::LeaveIfError(file.Open(rfs, path, EFileRead));
    CleanupClosePushL(file);

    // Read the file into the buffer.
    User::LeaveIfError(file.Size(size));

    buff = HBufC8::NewL(size);
    CleanupStack::PushL(buff);

    TPtr8  ptr((TUint8*) buff->Ptr(), size);
    User::LeaveIfError(file.Read(ptr, size));

    // Convert the buffer to ucs2 and clean up.
    xmlEncoding = CXmlEncoding::NewL();
    CleanupStack::PushL(xmlEncoding);

    if (!xmlEncoding->DetermineCharEncodingL(ptr, KNullDesC, encoding))
        {
        User::Leave(KErrCorrupt);
        }

    iTemplate = xmlEncoding->ConvertToUcs2L(encoding, ptr);

    CleanupStack::PopAndDestroy(xmlEncoding);
    CleanupStack::PopAndDestroy(buff);
    CleanupStack::PopAndDestroy(/*file*/);
    CleanupStack::PopAndDestroy(/*rfs*/);

    // Count the number of token.  These counts are used in ResolveTemplate
    // to determine how big the resolved buffer should be.
    TPtrC  findPtr;

    iTitleCount = 0;
    iWebUrlCount = 0;
    iDateCount = 0;
    iDescriptionCount = 0;
    iEnclosureCount = 0;
    iShowPrevCount = 0;
    iShowNextCount = 0;

    findPtr.Set(*iTemplate);
    while ((loc = findPtr.Find(KTokenTitle())) != KErrNotFound)
        {
        findPtr.Set(findPtr.Mid(loc + KTokenTitle().Length()));
        iTitleCount++;
        }

    findPtr.Set(*iTemplate);
    while ((loc = findPtr.Find(KTokenWebUrl())) != KErrNotFound)
        {
        findPtr.Set(findPtr.Mid(loc + KTokenWebUrl().Length()));
        iWebUrlCount++;
        }

    findPtr.Set(*iTemplate);
    while ((loc = findPtr.Find(KTokenDate())) != KErrNotFound)
        {
        findPtr.Set(findPtr.Mid(loc + KTokenDate().Length()));
        iDateCount++;
        }

    findPtr.Set(*iTemplate);
    while ((loc = findPtr.Find(KTokenDescription())) != KErrNotFound)
        {
        findPtr.Set(findPtr.Mid(loc + KTokenDescription().Length()));
        iDescriptionCount++;
        }

    findPtr.Set(*iTemplate);
    while ((loc = findPtr.Find((KTokenEnclosure))) != KErrNotFound)
        {
        findPtr.Set(findPtr.Mid(loc + KTokenEnclosure().Length()));
        iEnclosureCount++;
        }

    findPtr.Set(*iTemplate);
    while ((loc = findPtr.Find((KTokenShowPrev))) != KErrNotFound)
        {
        findPtr.Set(findPtr.Mid(loc + KTokenShowPrev().Length()));
        iShowPrevCount++;
        }

    findPtr.Set(*iTemplate);
    while ((loc = findPtr.Find((KTokenShowNext))) != KErrNotFound)
        {
        findPtr.Set(findPtr.Mid(loc + KTokenShowNext().Length()));
        iShowNextCount++;
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::ResolveTemplateL
//
// Loads and resolves the tokens in the template html file.
// -----------------------------------------------------------------------------
//
HBufC* CFeedsFeedContainer::ResolveTemplateL(const TDesC& aTitle,
        const TDesC& aTimestamp, const TDesC& aDescription, const TDesC& aUrl,
        const TDesC& aEnclosure)
    {
    HBufC*  ucs2Buff;
    TInt    loc;

    // Create and init the resolved buffer.
    ucs2Buff = HBufC::NewLC(iTemplate->Length() + (aTitle.Length() * iTitleCount) +
            (aTimestamp.Length() * iDateCount) + (aDescription.Length() * iDescriptionCount) +
            (aUrl.Length() * iWebUrlCount) +
            (aEnclosure.Length() * iEnclosureCount) + iShowPrevCount + iShowNextCount);

    ucs2Buff->Des().Copy(*iTemplate);

    // Resolve the tokens.
    TPtr  ucs2Ptr(ucs2Buff->Des());

    // Replace the title tokens.
    while ((loc = ucs2Ptr.Find(KTokenTitle())) != KErrNotFound)
        {
        ucs2Ptr.Replace(loc, KTokenTitle().Length(), aTitle);
        }

    // Replace the url tokens.
    while ((loc = ucs2Ptr.Find(KTokenWebUrl())) != KErrNotFound)
        {
        ucs2Ptr.Replace(loc, KTokenWebUrl().Length(), aUrl);
        }

    // Replace the date tokens.
    while ((loc = ucs2Ptr.Find(KTokenDate())) != KErrNotFound)
        {
        ucs2Ptr.Replace(loc, KTokenDate().Length(), aTimestamp);
        }

    // Replace the description tokens.
    while ((loc = ucs2Ptr.Find(KTokenDescription())) != KErrNotFound)
        {
        ucs2Ptr.Replace(loc, KTokenDescription().Length(), aDescription);
        }

    // Replace the enclosure tokens.
    while ((loc = ucs2Ptr.Find(KTokenEnclosure())) != KErrNotFound)
        {
        ucs2Ptr.Replace(loc, KTokenEnclosure().Length(), aEnclosure);
        }

    CleanupStack::Pop(ucs2Buff);
    return ucs2Buff;
    }


// -----------------------------------------------------------------------------
// CFeedsFeedContainer::EnsureTemplateL
//
// If need be copy the template from ROM.
// -----------------------------------------------------------------------------
//
void CFeedsFeedContainer::EnsureTemplateL(const TDesC& aName)
    {
    TInt                err;
    RFs                 defaultRfs;
    TUint               attValue = 0;
    TBuf<KMaxFileName>  path;

    // Open a connection to the working drive.
    User::LeaveIfError(defaultRfs.Connect());
    CleanupClosePushL(defaultRfs);
    User::LeaveIfError(defaultRfs.SetSessionPath(_L("c:\\")));

    // Build the path to the file.
    path.Append(_L("\\"));
    path.Append(aName);

    // Test whether or not the folder file is present.
    err = defaultRfs.Att(path, attValue);

    // The file is there, just return.
    if (err == KErrNone)
        {
        CleanupStack::PopAndDestroy(/*defaultRfs*/);
        return;
        }

    // If the file is missing copy it from ROM.
    if ((err == KErrNotFound) || (err == KErrPathNotFound))
        {
        RFs      romRfs;
        RFile    file;
        RFile    romFile;
        TInt     size;
        HBufC8*  buffer = NULL;

        // Open an rfs for the z drive.
        User::LeaveIfError(romRfs.Connect());
        CleanupClosePushL(romRfs);
        User::LeaveIfError(romRfs.SetSessionPath(_L("z:\\")));

        // Create the destination file.
        User::LeaveIfError(file.Create(defaultRfs, path, EFileWrite));
        CleanupClosePushL(file);

        // Open the source file.
        User::LeaveIfError(romFile.Open(romRfs, path, EFileRead));
        CleanupClosePushL(romFile);

        // Copy the file.
        User::LeaveIfError(romFile.Size(size));
        buffer = HBufC8::NewLC(size);
        TPtr8 bufferPtr(buffer->Des());

        User::LeaveIfError(romFile.Read(bufferPtr, size));
        User::LeaveIfError(file.Write(bufferPtr, size));

        // Clean up
        CleanupStack::PopAndDestroy(buffer);
        CleanupStack::PopAndDestroy(/*romFile*/);
        CleanupStack::PopAndDestroy(/*file*/);
        CleanupStack::PopAndDestroy(/*romRfs*/);
        CleanupStack::PopAndDestroy(/*defaultRfs*/);
        }
    }

// ---------------------------------------------------------
// CFeedsFeedContainer::ExtractBrCtlParam()
// ---------------------------------------------------------
//
TPtrC CFeedsFeedContainer::ExtractBrCtlParam
    ( TUint aParamTypeToFind,
      RArray<TUint>* aTypeArray,
      CDesCArrayFlat* aDesArray,
      TBool& aParamFound ) const
    {
    // initialize output parameter
    aParamFound = EFalse;
    TPtrC16 retParamValue;

    for ( TInt j = 0; j < aTypeArray->Count(); j++ )
        {
        const TUint paramType = (*aTypeArray)[j];
        if ( aParamTypeToFind == paramType )
            {
            // That's we need
            retParamValue.Set( aDesArray->MdcaPoint(j) );
            aParamFound = ETrue; // Indicate it in the out param
            break; // break the loop - we found it
            }
        }
    return retParamValue;
    }
