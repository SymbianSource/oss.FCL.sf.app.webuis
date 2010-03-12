#!/usr/local/bin/perl -w
# ============================================================================
# Name        : buildIADsis.pl
# Description: Creates SIS installation packages for S60 Browser that
#	     meets Arrow/IAD program requirements for delivery to end-user devices.
#	    Also generates R&D signed SIS packages for Nokia R&D internal use.
#
# Copyright © 2008 Nokia.  All rights reserved.
# This material, including documentation and any related computer
# programs, is protected by copyright controlled by Nokia.  All
# rights are reserved.  Copying, including reproducing, storing,
# adapting or translating, any or all of this material requires the
# prior written consent of Nokia.  This material also contains
# confidential information which may not be disclosed to others
# without the prior written consent of Nokia.
# ============================================================================

# TODOs
#			Baseline whatlog and build whatlog need to be processed and compared 

# Notes for maintainers:
# Versioning guide:
# http://s60wiki.nokia.com/S60Wiki/How_to_guide_for_versioning
# Eclipsing ROM files using installers
# http://s60wiki.nokia.com/S60Wiki/How_to_guide_for_eclipsing_ROM_files
# Installers basics
# http://s60wiki.nokia.com/S60Wiki/How_to_guide_for_creating/signing_sis_files
# SIS file naming convention for IAD delivery
#  http://s60wiki.nokia.com/S60Wiki/How_to_guide_for_uploading_sis_packages_to_IAD_server

use strict;
use warnings;
use Getopt::Long;
use Data::Dumper;
use File::Path;
use Benchmark;

my $usage = q{
Usage: buildIADsis.pl -opmode dev -appversion 7.1.2 -binversion 10.1 -variant 01  -nocache -verbose

Mandatory arguments: 
 -opmode : either dev or integration. If integration, use filelist argument below
 -appversion: Browser version as a string e.g 7.1.420 (major.minor.build). Minor must be 1 digit.
 -platform: S60 platform. E.g 3.23 or 5.0 
 -variant: S60 platform specific variant code. E.g for 3.23: 01->English, 22->Chinese etc.

Optional arguments:  
 -platform:     S60 platform. Such as S60.323 or S60.50 
 -filelist: 		A text file containing list of files to include in the PKG 
            		File paths must be of the form \epoc32\dir\urel\file.ext
 -binversion: 	version to inject inside all EXEs and DLLs. Must be 10.0 or higher.
 -nocache: 			does everything cleanly, no old data used
 -verbose: 			Turns on debug statements
 -webkey:				Web signing key filename 
 -webcert: 			Web signing certificate filename
 -cenrepkey: 		Cenrep signing key filename
 -cenrepcert: 	Cenrep signing certificate filename
			 
  For quick primer on differences between Application version and Binary version, see: 
  http://s60wiki.nokia.com/S60Wiki/How_to_guide_for_versioning
};

my ($s60platform, $rawFilelist,$appVersion, $binVersion, $variant,$opmode,$lang_code);
my (@variants);
my $langgroup;
my $type = "rnd";
my $brExe_UID3 = "0x10008D39"; #Unique ID for Browser application
my $packageTypeMajor = "RU";  # even though we use SA,RU, mention the RU in filename since it sounds more important
#TODO: We should be able to query the S60 platform by parsing some file in epoc32. Dunno how to do this. This should NOT be hardcoded. 
# Inital value for s60Platform is S60.323
$s60platform = "S60.323";
my $cenrepPkg = ".\\pkg\\BrowserNG_Cenrep.pkg";
my $cenrepSis = "BrowserNG_Cenrep.sis";
my $finalBrowserPkg = ".\\pkg\\BrowserNG.pkg";
my $debug_printing = 1;
my $nocache = 0; 
#TODO: We might want to pass this as a command-line argument? Note that unsigned SIS is also generated and can always
# be signed separately.
my $webkey = ".\\cert\\RDTest_02.key";
my $webcert = ".\\cert\\RDTest_02.der";
my $cenrepkey = ".\\cert\\RDTest_02.key";
my $cenrepcert = ".\\cert\\RDTest_02.der";
my $buildType = "urel";

my @cleanupList = ();

GetOptions(
      "platform=s", \$s60platform,  
      "appversion=s",\$appVersion,
			"binversion=s",\$binVersion,
            "variant=s",\$variant,
			"opmode=s",\$opmode,
			"nocache",\$nocache,
            "verbose",\$debug_printing,
			"filelist=s",\$rawFilelist,
			"webkey=s", \$webkey,
			"webcert=s", \$webcert,
			"cenrepkey=s", \$cenrepkey,
			"cenrepcert=s", \$cenrepcert,
			"build=s", \$buildType
			);


#### START OF ARG CHECKS 
if ( !defined $appVersion ) { print STDERR "\nBad argument: missing appversion\n $usage"; exit; } 
# Parse major.minor.build format string and choke if not well formed 
# We will later insert the parsed values inside the PKG file.
my @parsedAppVer  = checkAppVersion($appVersion); 
if (!$parsedAppVer[3]) { print STDERR "\nBad argument: malformed appversion string ($appVersion) \n $usage"; exit; }

#By default ROM EXEs/DLLs have a version of 10.0, so you want the newDLLs to have equal or higher minor version. See IAD Eclipsing guide for details
if ( !defined $binVersion ) { print STDOUT "\n Missing binary version, using default\n"; $binVersion = "10.1"; }
my @parsedBinVer = checkBinaryVersion($binVersion);
if (!$parsedBinVer[2]) { print STDERR "\nBad argument or default: malformed binversion string ($binVersion). \n $usage"; exit; }

if ( !defined $variant ) { print STDERR "\nBad argument: missing variant\n $usage"; exit; } 

if ( !defined $opmode || !($opmode =~ /^dev/ || $opmode =~ /^int/)) { print STDERR "\nBad argument: Can't grok opmode\n $usage"; exit; } 

if (defined $rawFilelist && (! -e $rawFilelist) ) { print STDERR "\nBad argument: File is missing: $rawFilelist\n $usage"; exit; }

print "no cache flag = $nocache \n" if $debug_printing;
#print "verbose = $debug_printing \n";
#### END OF ARG CHECKS 

# IAD-compliant SIS file naming convention
# ComponentName_<package UID>_v<package version>_<package type>_<platform>_<variant>.sis
# Example: TestApplication_0x12345678_v1.2.3_SA_S60.32_Euro1.sis
# Reference: http://s60wiki.nokia.com/S60Wiki/How_to_guide_for_uploading_sis_packages_to_IAD_server
# Where:
   #package version:  major.minor.build
   #package type:     SIS package type (SA, SP,..)
   #platform:         S60 Platform (S60.xx  where xx = 32, 50,..)
   #variant:          Language/operator variant
   
my $browserSIS =  "BrowserNG_${brExe_UID3}_v${appVersion}_${packageTypeMajor}_${s60platform}_${variant}.sis";
# For 323 we limit the number of IAD to Chinese, Western and Japan 
if ( $s60platform eq "S60.323" ) 
{
	if ( $variant == 50 ) {
      $browserSIS =  "BrowserNG_${brExe_UID3}_v${appVersion}_${packageTypeMajor}_${s60platform}_western.sis"; 
  }
	elsif ( $variant == 51 ) {
      $browserSIS =  "BrowserNG_${brExe_UID3}_v${appVersion}_${packageTypeMajor}_${s60platform}_china.sis"; 
  }
	elsif ( $variant == 16 ) {
      $browserSIS =  "BrowserNG_${brExe_UID3}_v${appVersion}_${packageTypeMajor}_${s60platform}_japan.sis"; 
  }
  else {
      print STDOUT "\n build variant $variant for RnD purpose \n"; 
  }
  print STDOUT "\n=== SIS filename: $browserSIS \n" ;
}
# Handle S60.50 situations 
if ( $s60platform eq "S60.50" ) 
{
# May need special handling later 
}

### Call pkg generation perl script 
system("CreateIADpackages.pl -s $s60platform -v $variant -p armv5 -r urel -bmajor $parsedAppVer[0] -bminor $parsedAppVer[1] -bnumber $parsedAppVer[2] -build $buildType"); 

# Pick up any files from the WHAT output that match any of these regex patterns
my $filenameRegEx = join '|', qw( 
								[.]dll$ 
								[.]exe$ 
								[.]mif$ 
								[.]r..$);

# These type of files need to be stamped with a version number >= 10.0 so that the 
# original files on ROM may be eclipsed.								
my $binariesRegEx = join '|', qw( 
								[.]dll$ 
								[.]exe$);

							




# In any mode, use a $rawFilelist is passed from the command line
if (defined $rawFilelist) { 
	print STDOUT "\n=== Caller supplied list of files: $rawFilelist === \n" ;
} else {
	# If no file list passed from command-line, we know how to generate one in DEV mode only
	if ($opmode =~ /^dev/) {
		$rawFilelist = "what_$buildType.log";
		if ($nocache ||  (! -e $rawFilelist) )  { 
			print STDOUT "\n=== Generating WHAT log, may take a while.. [",scalar(localtime),"] ===\n";
			makeWhatLog($buildType, $rawFilelist);
			print STDOUT "\n=== Finished generating WHAT log ",scalar(localtime)," ===\n";
		} else { 
			print STDOUT "\n=== Will use cached WHAT log : $rawFilelist ===\n";
		}
	} else { 
		die("\nBad argument: File is missing: $rawFilelist\n $usage");
	}
	
}

#Reduce list of files to those of interest
my @rawList = getRawFileList($rawFilelist); 
my @filteredFiles = grep(/$filenameRegEx/i, @rawList);
#print Dumper(@filteredFiles) if $debug_printing;

print "Changing binary files with version\n" if $debug_printing;
my @binariesOnly = grep(/$binariesRegEx/i, @filteredFiles);
# print Dumper(@binariesOnly) if $debug_printing;
for my $binaryFile (@binariesOnly) {
	print STDOUT "Fixing version to $binVersion for file: $binaryFile..\n" if $debug_printing;
	changeBinaryVersionAndMore("elftran", $binaryFile, $binVersion);
	if ( $? == -1 || ($? >> 8) != 0 )  { cleanDeath("Elftran failed to modify : $binaryFile "); }
}


print STDOUT "Creating CenRep installer\n" if $debug_printing;
# No need to cleanup the sis file since it might be certificated differently 
# push(@cleanupList, "$cenrepSis");
system("makesis $cenrepPkg $cenrepSis");
if ( $? == -1 || ($? >> 8) != 0 )  { cleanDeath("Makesis on CenRep installer failed"); } else {
	#printf "\nOK %d\n", $? >> 8; 
}
push(@cleanupList, "${cenrepSis}x");
print STDOUT "\nSigning CenRep installer\n" if $debug_printing;
system("signsis -v $cenrepSis ${cenrepSis}x  $cenrepcert $cenrepkey");
if ( $? == -1 || ($? >> 8) != 0 )  { cleanDeath("Signsis on CenRep installer failed"); } else { 
	#printf "\nOK %d\n", $? >> 8; 
}

print STDOUT "Creating main Browser installer with filename: $browserSIS\n" if $debug_printing;
system("makesis $finalBrowserPkg $browserSIS");
if ( $? == -1 || ($? >> 8) != 0 ) { cleanDeath("Makesis on main Browser installer failed"); } else { 
	#printf "\nOK %d\n", $? >> 8; 
}

print STDOUT "\nSigning main Browser installer\n" if $debug_printing;
system("signsis $browserSIS ${browserSIS}x $webcert $webkey");
if ( $? == -1 || ($? >> 8) != 0 )  { cleanDeath("Signsis on main Browser installer failed"); } else { 
	#printf "OK %d\n", $? >> 8; 
}
	
#cleanup all transient files
cleanup();

	
# Helps us generate list of DLLs EXEs and resource files to be packaged in SIS package	
sub makeWhatLog
{
	my $buildType = shift;
	my $file_whatLog = shift;

	system("\\gb_cmds\\gb_aurora_32_what.cmd armv5 $buildType > $file_whatLog"); 
}

# Returns an in-memory list of all binary and resource files that the build process generates (WHAT output)
sub getRawFileList
{
	my $filename = shift;
	my @array = 0;
	#print "\n == Loading file with list of all assets : $filename == \n" if $debug_printing;
	open(HANDLE, $filename) || cleanDeath("Failed to open WHAT command output: $filename\n");
	@array=<HANDLE>;
	close (HANDLE);
	#print Dumper(@rawList) if $debug_printing;

	return @array;
}

#
sub changeBinaryVersionAndMore
{
	my $elfCmd = shift;
    my $file = shift;
	my $binaryVersion = shift;
	
    system("$elfCmd -version $binaryVersion -compressionmethod bytepair $file");
}
# Dies after doing cleanup
sub cleanDeath { 
	my $reason = shift;
	cleanup();
	die($reason);
}

# Removes unwanted files from disk
sub cleanup { 
	for my $file (@cleanupList) {
		print "Cleaning up: $file\n";
		unlink($file);
	}
}

# Checks for a well formed Binary Version string in major.minor format and that it is >= 10.0 
# 10.0 is the default version of in-ROM binaries that must be eclipsed by our installation. 
sub checkBinaryVersion {
    my ($ver) = shift;
    
    my ($valid, $major, $minor) = (0, 0, 0);
    if ($ver =~ /^(\d+)\.(\d+)$/) {
        $valid = 1;
        ($major, $minor) = ($1, $2);
        # {=leading-zeros-ignored}
        $major =~ s/^0+//;
        $minor =~ s/^0+//;
		
    }

	my $intMajor = int($major || 0); 
	my $intMinor = int($minor || 0); 
	
	$valid = 0; 
	if ( $intMajor >= 10 && $intMinor >=0 ) { 
	 $valid = 1; # enforce 10.0 or higher rule for IAD
	} 
	 
	my @binVerArray = ( 
				  $intMajor,
                  $intMinor,
                  $valid
				  ); 
    
	#print Dumper(@binVerArray) if $debug_printing;
    return @binVerArray;
}

# Checks for a well formed Application Version string in major.minor.build format
# See the IAD versioning HOWTO (link above) for acceptable format
sub checkAppVersion {
    my ($ver) = shift;
    
    my ($valid, $major, $minor, $build) = (0, 0, 0, 0);
    if ($ver =~ /^(\d+)\.(\d+).(\d+)$/) {
        $valid = 1;
        ($major, $minor, $build) = ($1, $2, $3);
        # {=leading-zeros-ignored}
        $major =~ s/^0+//;
        $minor =~ s/^0+//;
		$build =~ s/^0+//;
    }

	my $intMajor = int($major || 0); 
	my $intMinor = int($minor || 0); 
	my $intBuild = int($build || 0);
	
	if ($intMinor < 0 || $intMinor > 9)  { 
		$valid = 0; # enforce single digit minor rule for IAD
	}
	
	my @appVerArray = ( 
				  $intMajor,
                  $intMinor,
				  $intBuild,
                  $valid
				  ); 
	#print Dumper(@appVerArray) if $debug_printing;
    return @appVerArray;
}

