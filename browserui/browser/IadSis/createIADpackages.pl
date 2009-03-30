#
# Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of the License "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:  This script generate various SIS package files depeding on the 
#							 depending on the variant selection. for example variant 01 will create
#							 a package support English, French, Germany and Portuguese. 
#
use Getopt::Long;
use File::Copy;
use File::Basename;
use strict;
use warnings;
use Cwd;
use FindBin qw($Bin);
use Data::Dumper;
use fcntl; 

my $usage = q{
Description: This script generate browser SISx package file automatically. 
This script usually is called by buildIADsis.pl. However it can be used separately as well. 
	
Example: create_package.pl -v 01 -p armv5 -r urel -bmajor 7 -bminor 1 -bnumber 32  

Mandatory arguments: 
-v: S60 language variant number
-p: Build target platform. Valid values are: armv5 or winscw, only armv5 supported right now
-r: Release build. Valid values are: urel or udeb, only urel supported right now 
-bmajor: build major number. In term of Browser version 7.1.1234, 7 is the major number 
-bminor: build minor number. In term of Browser version 7.1.1234, 1 is the minor number 
-bnumber: build number. In term of Browser version 7.1.1234, 1234 is the build number  

Optional arguments: 
- None

Limitations
- This script is currently valid only for 'armv5 urel' builds on S60 3.2.3 platform.

}; 

my %num_lang = (
	"01"	=> ["western", "01", "02", "03", "14"],
	"02"	=> ["western", "01", "09", "06", "16"],
	"03"	=> ["western", "01", "37", "57", "50"],
	"04"	=> ["western", "01", "05", "27", "17"],
	"05"	=> ["western", "01", "04", "18", "13"],
	"06"	=> ["western", "01", "51", "76", "83"],
	"07"	=> ["western", "01", "67", "54", "49"],
	"08"	=> ["western", "01", "68", "78", "93"],
	"09"	=> ["western", "01", "42", "45", "79"],
	"10"	=> ["western", "01", "59", "70", "39"],
	"11"	=> ["western", "01", "15", "07", "08"],
	"12"	=> ["western", "01", "58", "94"],
	"13"	=> ["western", "01", "25", "26", "28"],
	"14"	=> ["china", "29", "30", "157"],
	"15"	=> ["china", "31", "159"],
	"16"	=> ["japan", "32", "160"],
	"17"	=> ["thai", "33", "161"],
	"18"  => ["western", "01", "96"],
	"19"  => ["china", "31", "326", "159"],
	"20"  => ["western", "05", "44", "401", "402"],
	"50" => ["western", "01", "02", "03", "14", "09", "06", "16", "37", "57", "33", "50", "05", "27", 
											"17", "04", "18", "13", "51", "76", "83", "67", "54", "49", "68", "78", "93", 
											"42", "45", "79", "59", "70", "39", "15", "07", "08", "58", "94", "25", "26", "28", 
											"96", "05", "44"],
	"51" => ["china", "29", "30", "157", "31", "159"]
	);	
# To support western-all IAD delivery we need to remove Lang 401, 402 of variant 50

die $usage unless @ARGV;

print @ARGV; 
print "\n"; 

my ($variant_num,$platform,$release,$bmajor,$bminor,$bnumber); 
#my ($test1, $test2, $test3); 
GetOptions("v=s", => \$variant_num, "p=s",=> \$platform, "r=s",=> \$release,
           "bmajor=s", => \$bmajor, "bminor=s",=> \$bminor, "bnumber=s",=> \$bnumber) or die $usage; 
print "variant $variant_num \n";
print "platform $platform \n"; 
print "release $release \n";
print "bmajor, $bmajor \n"; 
print "bminor, $bminor \n"; 
print "bnumber, $bnumber \n"; 
 
print "... package variant $variant_num\n";
print "... package platform $platform \n";
print "... release $release \n";
# print "... major mini build $bmajor $bminor $bnumber \n"); 
# exit; 

# print $release; print "\n"; 


# Create BrowserNG_Cenrep.pkg file 
open (CENPKGFILE, '>.\\pkg\\BrowserNG_cenrep.pkg');
print CENPKGFILE "\;Languages\n"; 
print CENPKGFILE "\&EN\n\n"; 
print CENPKGFILE "\;Header\n";
print CENPKGFILE "\#\{\"Browser CenRep INI file\"\}\, \(0x10202BE9\)\,$bmajor,$bminor,$bnumber, TYPE=SP \n\n";
print CENPKGFILE <<ENDHEADER0;
;Localised Vendor name
%{"Nokia"}

;Unique Vendor name
:"Nokia"

;Supports S60 3rd Edition
[0x102032BE], 0, 0, 0, {"Series60ProductID"}

ENDHEADER0
print CENPKGFILE "\; CenRep ini file\n";
my $SrcDir = "\\epoc32\\data\\z\\private\\10202be9\\";
my $TargetDir = "c:\\private\\10202be9\\"; 
print CENPKGFILE "\n";
my @install_files = (
	"10008d39.txt",
	"101f8731.txt"
);
for my$install_file (@install_files) {
print CENPKGFILE "\"$SrcDir$install_file\"\n - \"$TargetDir$install_file\" \n"; 
}
close (CENPKGFILE); 

########################################################################
# create BrowserNG.pkg file
open (PKGFILE, '>.\\pkg\\BrowserNG.pkg');
#print PKGFILE <<ENDHEADER;
#;Languages
print PKGFILE "\;Languages\n"; 
#&EN
print PKGFILE "\&EN\n\n"; 
#
#;Header
#{"Web Browser Update"},(0x10008D39),7,1,0, TYPE=SA,RU
print PKGFILE "\;Header\n";
print PKGFILE "\#\{\"Web Browser Update\"\}\, \(0x10008D39\)\,$bmajor,$bminor,$bnumber, TYPE=SA,RU \n\n";

print PKGFILE <<ENDHEADER;
;Localised Vendor name
%{"Nokia"}

;Unique Vendor name
:"Nokia"

;Supports S60 3rd Edition
[0x102032BE], 0, 0, 0, {"Series60ProductID"}

ENDHEADER

# MIF resource files ot install
$SrcDir = "\\epoc32\\data\\Z\\resource\\apps\\";
$TargetDir = "!:\\resource\\apps\\"; 
print PKGFILE "\n";
# print PKGFILE $SrcDir; 
# print PKGFILE $TargetDir; 
@install_files = (
	"BrowserAudioVideoPlugin_aif.mif",
	"schemeapp_aif.mif",
	"VideoServices_AIF.MIF", 
	"browser.mif", 
	"browserng_aif.mif",
	"connman.mif", 
	"downloadmgruilib.mif",
	"operatormenu_aif.mif",
	"pushmtm.mif",
	"webkiticons.mif", 
	"webkitutilsicons.mif" 
);
for my $install_file (@install_files) {
print PKGFILE "\"$SrcDir$install_file\"\n - \"$TargetDir$install_file\" \n"; 
}

# SRC resource files ot install
$SrcDir = "\\epoc32\\data\\z\\private\\10003a3f\\apps\\";
$TargetDir = "!:\\private\\10003a3f\\import\\apps\\"; 
print PKGFILE "\n";
@install_files = (
	"BrowserNG_reg.rsc",
	"CodViewer_reg.rsc",
	"DdViewer_reg.rsc", 
	"Operatormenu_reg.rsc", 
	"PushViewer_reg.rsc",
	"RoapApp_reg.rsc", 
	"SchemeApp_reg.rsc",
	"VideoServices_reg.rsc",
	"WidgetUi_reg.rsc"
);
for my$install_file (@install_files) {
print PKGFILE "\"$SrcDir$install_file\"\n - \"$TargetDir$install_file\" \n"; 
}

# SRC resource files ot install
$SrcDir = "\\epoc32\\data\\Z\\resource\\";
$TargetDir = "!:\\resource\\"; 
print PKGFILE "\n";
@install_files = (
	"AiwBrowserProvider.rsc"
);
for my $install_file (@install_files) {
print PKGFILE "\"$SrcDir$install_file\"\n - \"$TargetDir$install_file\" \n\n"; 
}

# Get the languages of the variant 
my $variant_lang = $num_lang{$variant_num}; 
my @variant_lang = @$variant_lang; 
shift @variant_lang; 

# generate the lang dependent files 
for my$lang (@variant_lang) {
  print PKGFILE "IF exists\( \"z:\\resource\\avkon.r$lang\" ) \n";	

	$SrcDir = "\\epoc32\\data\\z\\resource\\";
	$TargetDir = "!:\\resource\\"; 
	print PKGFILE "\n";
	@install_files = (
		"webkit",
		"BrowserAudioVideoPlugin",
		"BrowserDialogsProvider", 
		"browsertelservice", 
		"CodUi",
		"ConnectionManager", 
		"DownloadMgrUiLib",
		"WidgetInstallerUI",
		"WidgetMenu"
	);
	for my$install_file (@install_files) {
	print PKGFILE "\"$SrcDir$install_file.r$lang\"\n - \"$TargetDir$install_file.r$lang\" \n"; 
	}
	# generate more lang dependent files 
	$SrcDir = "\\epoc32\\data\\z\\resource\\";
	$TargetDir = "!:\\resource\\"; 
	print PKGFILE "\n";
	@install_files = (
		"webkitutils"
	);
	for my$install_file (@install_files) {
	print PKGFILE "\"$SrcDir$install_file.r$lang\"\n - \"$TargetDir$install_file.r$lang\" \n"; 	
	}
	# generate more lang dependent files 
	$SrcDir = "\\epoc32\\data\\z\\resource\\apps\\";
	$TargetDir = "!:\\resource\\apps\\"; 
	print PKGFILE "\n";
	@install_files = (
		"BrowserNG",
		"CodViewer",
		"DdViewer",
		"Operatormenu",
		"PushViewer",
		"RoapApp",
		"SchemeApp",
		"VideoServices",
		"WidgetUi"
	);
	for my$install_file (@install_files) {
	print PKGFILE "\"$SrcDir$install_file.r$lang\"\n - \"$TargetDir$install_file.r$lang\" \n"; 
	}
	# generate more lang dependent files 
	$SrcDir = "\\epoc32\\data\\z\\resource\\messaging\\mtm\\";
	$TargetDir = "!:\\resource\\messaging\\mtm\\"; 
	print PKGFILE "\n";
	@install_files = (
		"PushRegistry"
	);
	for my$install_file (@install_files) {
	print PKGFILE "\"$SrcDir$install_file.r$lang\"\n - \"$TargetDir$install_file.r$lang\" \n"; 
	}
	# generate more lang dependent files 
	$SrcDir = "\\epoc32\\data\\z\\resource\\messaging\\";
	$TargetDir = "!:\\resource\\messaging\\"; 
	print PKGFILE "\n";
	@install_files = (
		"PushMtmUi"
	);
	for my$install_file (@install_files) {
	print PKGFILE "\"$SrcDir$install_file.r$lang\"\n - \"$TargetDir$install_file.r$lang\" \n"; 
	}
print PKGFILE "ENDIF \n";
}
# end of generate the lang dependent files 

# Non lang dependent rsc files 
$SrcDir = "\\epoc32\\data\\z\\resource\\plugins\\";
$TargetDir = "!:\\resource\\plugins\\"; 
print PKGFILE "\n";
@install_files = (
	"AiwBrowserProvider",
	"BrowserRec",
	"CodRecog",
	"CookieFilter",
	"DdRecog",
	"DeflateFilter",
	"HttpFilterAcceptHeader",
	"HttpFilterAuthentication",
	"HttpFilterCache",
	"HttpFilterPipeliningConfig",
	"PushMtmPushContentHandler",
	"PushMtmWhiteListAdapter",
	"SchemeDispatcher",
	"WidgetInstallerUI", 
	"widgetRecognizer",
	"httpfilterIop",
	"httpfilterconnhandler",
	"httpfilterproxy",
	"memoryplugin",
	"npBrowserAudioVideoPlugin",
	"npGpsPlugin",
	"npSystemInfoPlugin", 
	"uaproffilter",
	"widgetmemoryplugin"	
);
print PKGFILE "\n";
for my$install_file (@install_files) {
print PKGFILE "\"$SrcDir$install_file.rsc\"\n - \"$TargetDir$install_file.rsc\" \n"; 
}

# Files to install (binaries - DLL)
$SrcDir = "\\epoc32\\release\\armv5\\urel\\";
$TargetDir = "!:\\sys\\bin\\"; 
print PKGFILE "\n";
@install_files = (
	"AiwBrowserProvider",
	"BrowserCache",
	"BrowserLauncher",
	"BrowserRec",
	"BrowserTelService",
	"CodDownload",
	"CodEng",
	"CodRecog",
	"CodUi",
	"ConnectionManager",
	"DdRecog",
	"DeflateFilter",
	"DownloadMgr",
	"DownloadMgrUiLib", 
	"FavouritesEngine",
	"FeedsServerApi",
	"FeedsServerClient",
	"HttpCacheManager",
	"HttpDMServEng",
	"HttpFilterAcceptHeader",
	"HttpFilterAuthentication",
	"HttpFilterCache",
	"HttpFilterCommon", 
	"HttpFilterPipeliningConfig",
	"JavaScriptCore",
	"MemMan",
	"Multipartparser",
	"PushMtmCliSrv",
	"PushMtmPushContentHandler",
	"PushMtmUi",
	"PushMtmUtil",
	"PushMtmWhiteListAdapter",
	"RECENTURLSTORE",
	"SchemeDispatcher",
	"WidgetInstallerUI",
	"WidgetRecognizer",
	"WidgetRegistryClient",
	"browserdialogsprovider",
	"browserengine",
	"cXmlParser",
	"cookiefilter",
	"cookiemanager",
	"httpfilterIop",
	"httpfilterconnhandler",
	"httpfilterproxy",
	"memoryplugin",
	"npBrowserAudioVideoPlugin",
	"npGpsPlugin",
	"npSystemInfoPlugin",
	"pagescaler",
	"uaproffilter",
	"webkitutils",
	"webutils",
	"widgetengine",
	"WidgetInstaller",
	"widgetmemoryplugin",
	"wmlEngine"
);	
for my$install_file (@install_files) {
print PKGFILE "\"$SrcDir$install_file.dll\"\n - \"$TargetDir$install_file.dll\" \n"; 
}

# Files to install (binaries - EXE)
$SrcDir = "\\epoc32\\release\\armv5\\urel\\";
$TargetDir = "!:\\sys\\bin\\"; 
print PKGFILE "\n";
@install_files = (
	"BrowserNG",
	"CodViewer",
	"CookieServer",
	"DdViewer",
	"DownloadMgrServer",
	"FavouritesSrv",
	"FeedsServer",
	"OperatorMenu",
	"PushViewer",
	"RoapApp",
	"SchemeApp",
	"VideoServices",
	"WidgetBackupRestore",
	"widgetlauncher", 
	"WidgetRegistry",
	"WidgetUi"
);	
for my$install_file (@install_files) {
print PKGFILE "\"$SrcDir$install_file.exe\"\n - \"$TargetDir$install_file.exe\" \n"; 
}

# add the cenrep file 
print PKGFILE "\n\n";
print PKGFILE "\@\"BrowserNG_Cenrep.sisx\", (0x10202BE9)\n"; 

close (PKGFILE); 

print "end of the package" 