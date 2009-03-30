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
* Description:  Member definitions of CPushMessageInfoDialog.
*
*/



//  INCLUDES

#include "PushMessageInfoDialog.h"
#include "PushMtmUiDef.h"
#include "PushMtmUiPanic.h"
#include "PushMtmUtil.h"
#include "PushMtmLog.h"
#include <CSIPushMsgEntry.h>
#include <CSLPushMsgEntry.h>
#include <CUnknownPushMsgEntry.h>
#include <PushMtmUi.rsg>
#include <AknLists.h>
#include <msvapi.h>
#include <e32math.h>
#include <StringLoader.h>
#include <AVKON.RSG>
#include <bldvariant.hrh>
#include <FeatMgr.h>
#include <data_caging_path_literals.hrh>
#include <f32file.h>
#include <AknUtils.h>
#include <AknBidiTextUtils.h>

// CONSTANTS

LOCAL_D const TInt KNoResource = 0;
LOCAL_D const TInt KArrayGranularity = 16;
LOCAL_D const TReal KReal1024 = 1024.0;
_LIT( KTab, "\t" );
_LIT( KSpace, " " );


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMessageInfoDialog::CPushMessageInfoDialog
// ---------------------------------------------------------
//
CPushMessageInfoDialog::CPushMessageInfoDialog()
:   CAknPopupList(), 
    iResourceLoader( *iCoeEnv )
    {
    }

// ---------------------------------------------------------
// CPushMessageInfoDialog::~CPushMessageInfoDialog
// ---------------------------------------------------------
//
CPushMessageInfoDialog::~CPushMessageInfoDialog()
    {
    iSecondColumnFont = 0;
    iResourceLoader.Close();
    delete iListBox;
    delete iListBoxModel;
    FeatureManager::UnInitializeLib();
    }

// ---------------------------------------------------------
// CPushMessageInfoDialog::ExecuteLD
// ---------------------------------------------------------
//
void CPushMessageInfoDialog::ExecuteLD( CMsvSession& aMsvSession, TMsvId aId )
    {
    ConstructL( aMsvSession, aId );
    CAknPopupList::ExecuteLD();
    }

// ---------------------------------------------------------
// CPushMessageInfoDialog::ConstructL
// ---------------------------------------------------------
//
void CPushMessageInfoDialog::ConstructL( CMsvSession& aMsvSession, TMsvId aId )
    {
    PUSHLOG_ENTERFN("CPushMessageInfoDialog::ConstructL");

    __ASSERT_DEBUG( !iListBox && 
                    !iListBoxModel, 
                    UiPanic( EPushMtmUiPanAlreadyInitialized ) );

    FeatureManager::InitializeLibL();

    // Add resource file.
    TParse* fileParser = new (ELeave) TParse;
    // Unnecessary to call CleanupStack::PushL( fileParser );
    fileParser->Set( KPushMtmUiResourceFileAndDrive, &KDC_MTM_RESOURCE_DIR, NULL ); 
    iResourceFile = fileParser->FullName();
    PUSHLOG_WRITE_FORMAT(" iResourceFile: <%S>",&iResourceFile);
    delete fileParser;
    fileParser = NULL;
    iResourceLoader.OpenL( iResourceFile );
    PUSHLOG_WRITE(" iResourceFile opened");

    // Construct list box's model first
    // This list box will be used by an CAknPopupList object.
    __ASSERT_DEBUG( !iListBoxModel && !iListBox, 
                    UiPanic( EPushMtmUiPanAlreadyInitialized ) );
   	iListBoxModel = new (ELeave) CDesCArrayFlat( KArrayGranularity );
    // Construct list box.
    iListBox = new (ELeave) CAknSingleHeadingPopupMenuStyleListBox;

    // ========================================================
    // Layout info of CAknSingleHeadingPopupMenuStyleListBox
    TAknTextLineLayout textLayout = 
        AknLayout::List_pane_texts__menu_single_heading__Line_2(0);

    // Font of the second column of the given list box
    iSecondColumnFont = AknLayoutUtils::FontFromId( textLayout.FontId() );

    // Line width of the second column of the given list box
    TInt itemCount = 0; // No need to set
    TInt titleVisible = 1; // known value (we always have title)
    TRect main_pane = iAvkonAppUi->ClientRect();
 
    TAknLayoutRect popup_menu_graphic_window;
    popup_menu_graphic_window.LayoutRect( main_pane,
        AknLayout::popup_menu_graphic_window(itemCount) );
 
    TAknLayoutRect menu_heading_pane;
    menu_heading_pane.LayoutRect( popup_menu_graphic_window.Rect(),
        AknLayout::list_menu_heading_pane(titleVisible, itemCount) );
 
    TAknLayoutRect list_item;
    list_item.LayoutRect( menu_heading_pane.Rect(),
        AknLayout::list_single_heading_popup_menu_pane(itemCount) );
 
    TAknLayoutText text;
    text.LayoutText( list_item.Rect(), textLayout );

    iSecondColumnWidth = text.TextRect().Width();
    // ========================================================

    // Add items to the list.
    AddMessageInfoItemsL( aMsvSession, aId );

    CAknPopupList::ConstructL( iListBox, 
                               R_AVKON_SOFTKEYS_OK_EMPTY__OK, 
                               AknPopupLayouts::EMenuGraphicHeadingWindow );

	iListBox->ConstructL( this, EAknListBoxViewerFlags );
	iListBox->CreateScrollBarFrameL( ETrue );
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL
        ( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    CTextListBoxModel* listBoxModel = iListBox->Model();
	listBoxModel->SetItemTextArray( iListBoxModel );
	listBoxModel->SetOwnershipType( ELbmDoesNotOwnItemArray );

    // Get title text of popup list
    HBufC* title = iCoeEnv->AllocReadResourceLC( R_PUSHMID_TITLE );
    PUSHLOG_WRITE(" R_PUSHMID_TITLE read");
    SetTitleL( *title );
    CleanupStack::PopAndDestroy( title ); // title

    PUSHLOG_LEAVEFN("CPushMessageInfoDialog::ConstructL");
	}

// ---------------------------------------------------------
// CPushMessageInfoDialog::AddMessageInfoItemL
// ---------------------------------------------------------
//
void CPushMessageInfoDialog::AddMessageInfoItemL( TInt aHeaderResourceId, 
                                                  const TDesC& aText ) const
    {
    __ASSERT_DEBUG( iListBoxModel, UiPanic( EPushMtmUiPanNotInitialized ) );

    // Make some necessary conversion.
    HBufC* paramTextCopy = aText.AllocLC();
    TPtr paramTextCopyPtr = paramTextCopy->Des();
    // Remove tabs, because of the list box...
    AknTextUtils::StripCharacters( paramTextCopyPtr, KAknStripListControlChars );
    // Number conversion
    if ( paramTextCopyPtr.Length() > 0 )
        {
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion( paramTextCopyPtr );
        }

    HBufC* header = ( aHeaderResourceId == KNoResource ? 
        KNullDesC().AllocLC() : 
        iCoeEnv->AllocReadResourceLC( aHeaderResourceId ) );

    CArrayFix<TPtrC>* wrappedArray = 
        new ( ELeave ) CArrayFixFlat<TPtrC>( KArrayGranularity );
    CleanupStack::PushL( wrappedArray );

    AknTextUtils::WrapToArrayL
                                ( paramTextCopyPtr, iSecondColumnWidth, 
                                  *iSecondColumnFont, *wrappedArray );
    // visual is directly not used, but wrappedArray points to it

    // Add the lines in wrappedArray to the list
    const TInt lines = wrappedArray->Count();
    for ( TInt loop = 0; loop < lines; ++loop )
        {
        TPtrC current = ( *wrappedArray )[loop];
        // Allocate a buffer for the new list box item text 
        // (including header, separator and wrapped text)
        HBufC* currentFormattedLine = HBufC::NewLC
            ( ((loop==0) ? header->Length() : /*KSpace().Length()*/1) + 
              /*KTab().Length()*/1 + current.Length() );
        TPtr currentFormattedLinePtr = currentFormattedLine->Des(); // writeable

        if ( loop == 0 )
            { currentFormattedLinePtr.Copy( *header ); }
        else
            { currentFormattedLinePtr.Copy( KSpace ); }

        currentFormattedLinePtr.Append( KTab );
        currentFormattedLinePtr.Append( current );
        iListBoxModel->AppendL( currentFormattedLinePtr );

        CleanupStack::PopAndDestroy( currentFormattedLine ); // currentFormattedLine
        }

    CleanupStack::PopAndDestroy( 1, wrappedArray ); //  wrappedArray
    CleanupStack::PopAndDestroy( 2, paramTextCopy ); // header, paramTextCopy
    }

// ---------------------------------------------------------
// CPushMessageInfoDialog::ConvertUniversalToHomeTime
// ---------------------------------------------------------
//
TTime CPushMessageInfoDialog::ConvertUniversalToHomeTime
                              ( const TTime& aUniversalTime ) const
    {
    TTime time( aUniversalTime );   // time stores UTC time.

    TLocale locale;
    TTimeIntervalSeconds universalTimeOffset( locale.UniversalTimeOffset() );
    // Add locale's universal time offset to universal time.
    time += universalTimeOffset;    // time stores Local Time.

    // If home daylight saving in effect, add one hour offset.
    if ( locale.QueryHomeHasDaylightSavingOn() )
          {
          TTimeIntervalHours daylightSaving(1);
          time += daylightSaving;   // Now it is Home Time.
          }

#ifdef __TEST_LOG__
        // The following section will call LEAVEable functions, but 
        // they are only for logging purposes.
        _LIT( KDateFormat, "%E%D%X%N%Y %1 %2 %3" );
        _LIT( KTimeFormat, "%-B%:0%J%:1%T%:2%S%:3%+B" );
        TBuf<32> dateHolder;
        TBuf<32> timeHolder;
        aUniversalTime.FormatL( dateHolder, KDateFormat );
        aUniversalTime.FormatL( timeHolder, KTimeFormat );
        PUSHLOG_WRITE_FORMAT(" UTC date: <%S>",&dateHolder);
        PUSHLOG_WRITE_FORMAT(" UTC time: <%S>",&timeHolder);
        time.FormatL( dateHolder, KDateFormat );
        time.FormatL( timeHolder, KTimeFormat );
        PUSHLOG_WRITE_FORMAT(" Home date: <%S>",&dateHolder);
        PUSHLOG_WRITE_FORMAT(" Home time: <%S>",&timeHolder);
#endif // __TEST_LOG__

    return time; // Home Time.
    }

// ---------------------------------------------------------
// CPushMessageInfoDialog::AddMessageInfoItemsL
// ---------------------------------------------------------
//
void CPushMessageInfoDialog::AddMessageInfoItemsL( CMsvSession& aMsvSession, 
                                                   TMsvId aId ) const
    {
    PUSHLOG_ENTERFN("CPushMessageInfoDialog::AddMessageInfoItemsL");

    TMsvEntry tEntry;
    TMsvId dummyService;
    User::LeaveIfError( aMsvSession.GetEntry( aId, dummyService, tEntry ) );
    PUSHLOG_WRITE(" Entry got");
    __ASSERT_DEBUG
        ( tEntry.iMtm == KUidMtmWapPush, UiPanic( EPushMtmUiPanBadType ) );

    // Construct model.
    CPushMsgEntryBase* msg = NULL;
    const TInt32 bioType( tEntry.iBioType );
    if ( bioType == KUidWapPushMsgSI.iUid )
        {
        msg = CSIPushMsgEntry::NewL();
        PUSHLOG_WRITE(" SI");
        }
#ifdef __SERIES60_PUSH_SL
    else if ( bioType == KUidWapPushMsgSL.iUid )
        {
        msg = CSLPushMsgEntry::NewL();
        PUSHLOG_WRITE(" SL");
        }
#endif // __SERIES60_PUSH_SL
#ifdef __SERIES60_PUSH_SP
    else if ( bioType == KUidWapPushMsgUnknown.iUid )
        {
        msg = CUnknownPushMsgEntry::NewL();
        PUSHLOG_WRITE(" Unknown");
        }
#endif // __SERIES60_PUSH_SP
    else
        {
        PUSHLOG_WRITE(" Bad message type");
        User::Leave( KErrNotSupported );
        }

    CleanupStack::PushL( msg );
    msg->RetrieveL( aMsvSession, tEntry.Id() );
    PUSHLOG_WRITE(" RetreiveL OK");

    // ************************************************************************
    // Information item: From (push initiator).
    // This can be MSISDN number, URL, etc.
    // ************************************************************************
    TPtrC8 from8( msg->From() );
    if ( from8.Length() )
        {
        // Copy the 8-bit text into a 16-bit one and convert the string to 
        // decoded form.
        HBufC* from16 = HBufC::NewMaxLC( from8.Length() );
        from16->Des().Copy( from8 );

        HBufC* convertedFrom = 
            CPushMtmUtil::ConvertUriToDisplayFormL( *from16 );
        CleanupStack::PushL( convertedFrom );
        //
        AddMessageInfoItemL( R_PUSHMID_FROM, *convertedFrom );
        PUSHLOG_WRITE(" From added");
        //
        CleanupStack::PopAndDestroy( 2, from16 ); // convertedFrom, from16
        }

    // ************************************************************************
    // Information item: Address/URL (SI and SL specific).
    // ************************************************************************
    if ( msg->PushMsgType() == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, msg );
        const TPtrC url = si->Url();
        if ( url.Length() )
            {
            //
            HBufC* convertedUrl = 
                CPushMtmUtil::ConvertUriToDisplayFormL( url );
            CleanupStack::PushL( convertedUrl );
            //
            AddMessageInfoItemL( R_PUSHMID_URL, *convertedUrl );
            PUSHLOG_WRITE(" URL added");
            //
            CleanupStack::PopAndDestroy( convertedUrl );
            }
        }
#ifdef __SERIES60_PUSH_SL
    else if ( msg->PushMsgType() == KUidWapPushMsgSL.iUid )
        {
        CSLPushMsgEntry* sl = STATIC_CAST( CSLPushMsgEntry*, msg );
        const TPtrC url = sl->Url();
        if ( url.Length() )
            {
            //
            HBufC* convertedUrl = 
                CPushMtmUtil::ConvertUriToDisplayFormL( url );
            CleanupStack::PushL( convertedUrl );
            //
            AddMessageInfoItemL( R_PUSHMID_URL, sl->Url() );
            PUSHLOG_WRITE(" URL added");
            //
            CleanupStack::PopAndDestroy( convertedUrl );
            }
        }
#endif // __SERIES60_PUSH_SL

    // ************************************************************************
    // Information item: Message body/text (SI specific).
    // ************************************************************************
    if ( msg->PushMsgType() == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, msg );
        if ( si->Text().Length() )
            {
            TPtrC siText = si->Text();
            PUSHLOG_WRITE_FORMAT(" SI text: <%S>",&siText);
            AddMessageInfoItemL( R_PUSHMID_BODY, siText );
            PUSHLOG_WRITE(" SI text added");
            }
        }

    // ************************************************************************
    // Information item: Type.
    // ************************************************************************
    HBufC* value = iCoeEnv->AllocReadResourceLC( R_PUSHMID_TYPE_STR );
    AddMessageInfoItemL( R_PUSHMID_TYPE, *value );
    CleanupStack::PopAndDestroy( value ); // value
    PUSHLOG_WRITE(" Msg. type added");

#ifdef __SERIES60_PUSH_SP
    // ************************************************************************
    // Information item: Content type (SP specific)
    // ************************************************************************
    if ( msg->PushMsgType() == KUidWapPushMsgUnknown.iUid )
        {
        CUnknownPushMsgEntry* sp = STATIC_CAST( CUnknownPushMsgEntry*, msg );
        TPtrC contType = sp->ContentType();
        if ( contType.Length() )
            {
            AddMessageInfoItemL( R_PUSHMID_CONTENT_TYPE, contType );
            PUSHLOG_WRITE(" Content type added ");
            }
        }
#endif // __SERIES60_PUSH_SP

    TBuf<32> time;
    TBuf<32> date;
    HBufC* dateFormatString = 
                iCoeEnv->AllocReadResourceLC( R_QTN_DATE_USUAL_WITH_ZERO );
    HBufC* timeFormatString = 
                iCoeEnv->AllocReadResourceLC( R_QTN_TIME_USUAL_WITH_ZERO );

    // ************************************************************************
    // SI specific information item: creation time and date.
    // ************************************************************************
    if ( msg->PushMsgType() == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, msg );
        const TTime created( si->Created() );
        // Check if created time was set.
        if ( created != Time::NullTTime() )
            {
            // OK. this attribute was set. Render it to the user.
            TTime creLocal = ConvertUniversalToHomeTime( created );
            creLocal.FormatL( time, *timeFormatString );
            creLocal.FormatL( date, *dateFormatString );
            AddMessageInfoItemL( R_PUSHMID_CREATED, time );
            AddMessageInfoItemL( KNoResource, date );
            PUSHLOG_WRITE(" Created added");
            }
        }

    // ************************************************************************
    // Information item: Arrival time and date.
    // ************************************************************************
    const TTime arrivalTime( msg->ReceivedDate() );
    // Arrival time is always set by the wappush framework. Render it to the user.
    TTime arrLocal = ConvertUniversalToHomeTime( arrivalTime );
    arrLocal.FormatL( time, *timeFormatString );
    arrLocal.FormatL( date, *dateFormatString );
    AddMessageInfoItemL( R_PUSHMID_RCVD, time );
    AddMessageInfoItemL( KNoResource, date );
    PUSHLOG_WRITE(" Received added");

    // ************************************************************************
    // SI specific information item: expiration time and date.
    // ************************************************************************
    if ( msg->PushMsgType() == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, msg );
        const TTime expires( si->Expires() );
        // Check expiration if expiration time was set.
        if ( expires != Time::NullTTime() )
            {
            TTime expLocal = ConvertUniversalToHomeTime( expires );
            expLocal.FormatL( time, *timeFormatString );
            expLocal.FormatL( date, *dateFormatString );
            AddMessageInfoItemL( R_PUSHMID_EXPIRES, time );
            AddMessageInfoItemL( KNoResource, date );
            PUSHLOG_WRITE(" Expires added");
            }
        }

    CleanupStack::PopAndDestroy( 2, dateFormatString ); 
    // timeFormatString, dateFormatString

    // ************************************************************************
    // Information item: Size.
    // Convert the message size given in bytes into kilobytes. Round the 
    // value up, if necessary.
    // ************************************************************************
    TReal messageSize( 0 );
    if ( msg->PushMsgType() == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, msg );
        messageSize = (TReal)si->Text().Size() / KReal1024;
        }
#ifdef __SERIES60_PUSH_SL
    else if ( msg->PushMsgType() == KUidWapPushMsgSL.iUid )
        {
        CSLPushMsgEntry* sl = STATIC_CAST( CSLPushMsgEntry*, msg );
        messageSize = (TReal)sl->Url().Size() / KReal1024;
        }
#endif // __SERIES60_PUSH_SL
#ifdef __SERIES60_PUSH_SP
    else if ( msg->PushMsgType() == KUidWapPushMsgUnknown.iUid )
        {
        CUnknownPushMsgEntry* sp = STATIC_CAST( CUnknownPushMsgEntry*, msg );
        messageSize = (TReal)sp->MessageData().Size() / KReal1024;
        }
#endif // __SERIES60_PUSH_SP
    TInt32 messageSizeInt;
    User::LeaveIfError( Math::Int( messageSizeInt, messageSize ) );
    TReal fraction;
    User::LeaveIfError( Math::Frac( fraction, messageSize ) );
    if ( fraction )
        {
        ++messageSizeInt;
        }
    HBufC* size = 
        StringLoader::LoadLC( R_QTN_SIZE_KB, messageSizeInt, iCoeEnv );
    // From avkon.loc:
    // #define qtn_size_kb " %N kB"
    // We don't need white spaces at the beginning of the size string, so 
    // remove them:
    size->Des().Trim();
    AddMessageInfoItemL( R_PUSHMID_SIZE, *size );
    PUSHLOG_WRITE(" Size added");
    CleanupStack::PopAndDestroy( size ); // size

    CleanupStack::PopAndDestroy( msg ); // msg

    PUSHLOG_LEAVEFN("CPushMessageInfoDialog::AddMessageInfoItemsL");
    }

// End of file.
