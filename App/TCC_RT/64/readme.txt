                           TCC-RT v28.0

                     README.TXT -- July 2021

                          JP Software Inc.
                            P.O. Box 328
                     Chestertown, MD 21620, USA

                         https://jpsoft.com

Greetings, and thanks for trying TCC-RT!

This file contains a variety of information you should read before using
TCC-RT, including:

      TCC RT Overview
      Installation Notes
      Files Included
      Technical Support and Downloads
      Legal Stuff

TCC-RT  Overview
----------------

  TCC-RT is a free run time version of TCC (Take Command Console)
  included in our flagship product Take Command. TCC-RT allows you to run
  TCC batch files (i.e., *.btm) without having to install the full Take
  Command on target machines.
 
  Because TCC-RT is intended only for non-interactive use on systems
  without a full Take Command installation, it does not support the
  following TCC commands (the commands won't return an error; they just
  won't do anything):

    BATCOMP
    BDEBUGGER
    BREAKPOINT
    EVERYTHING
    HELP
    IDE
    OPTION dialogs
    PIPEVIEW
    STATUSBAR
    TABCOMPLETE
    TCDIALOG
    TCEDIT
    TCFILTER
    TCTOOLBAR
    VIEW

  TCC-RT does not support command dialogs, interactive command line 
  input & editing, or the PRE_INPUT / PRE_EXEC, and POST_EXEC aliases.

  To run TCC batch files, just pass the batch file name to TCC RT.  For
  example:

    tcc.exe mybatch.btm

Installation Notes
------------------

   Our products are distributed as Windows Installer files which install
   the product when run. The TCC-RT installer is a combined x86 and x64
   installer; it will install the appropriate version for the type of
   Windows you are running.

Files Included
--------------

   The following files are included with TCC-RT 28.0:

     TCC.EXE             TCC-RT program file
     BORLNDMM.DLL        Support DLL for TPIPE
     DebenuPDFLibrary64DLL1411.dll  Support DLL for TPIPE
     ENGLISH.DLL         English text for internal messages
     FRENCH.DLL          French text for internal messages
     GERMAN.DLL          German text for internal messages
     IPWORKS20.DLL       Internet functions support
     IPWORKSBT20.DLL     Bluetooth functions support
     IPWORKSSH20.DLL     SSH internet functions support
     IPWORKSZIP20.DLL    Compression functions support
     LICENSE.TXT         TCC-RT user license
     LUA54.DLL           Lua language support for TCC-RT
     NSILP_PowerShell.x64.DLL  PowerShell support DLL for TCC-RT
     ONIG.DLL            Regular Expressions support DLL for TCC
     README.TXT          This file
     regid.2002-09.com.jpsoft_TCC RT 28.0.swidtag
                         ISO/IEC 19770-2:2009 Software Identification Tag
     RUSSIAN.DLL         Russian text for internal messages
     SPANISH.DLL         Spanish text for internal messages
     TAKECMD.DLL         Shared DLL for TCC-RT
     TC-PROCESSENV64.DLL TCC-RT support DLL for SETP & UNSETP
     TEXTPIPEENGINE64.DLL  TPIPE support DLL
     TPIPE.EXE           TPIPE text processor program file
     UPDATER.EXE         Utility program for autoupdates
     UPDATER.INI         Initialization file for UPDATER.EXE

Technical Support, Downloads, and Updates
-----------------------------------------

   Complete details on technical support, including support plans and 
   terms and conditions, are on our web site.

   Standard, no-charge support is available through our online support 
   forums, where our support personnel can read and respond to your 
   messages, and other users can respond as well. The forums are
   accessible at:

      https://jpsoft.com/forums/

   To download JP Software files, including TCC-RT maintenance updates and
   trial versions of Take Command, visit our web site at:

      https://jpsoft.com

   You can use UPDATER.EXE to check for and download updates from out website.
   To check for new versions and display a dialog to optionally download a new
   version:

      updater.exe /checknow

   To check for a new version and download and install it automatically without
   displaying any dialog:

      updater.exe /silentall

LEGAL STUFF
-----------

Copyright 2021, JP Software Inc., All Rights Reserved. Published by JP
Software Inc., P.O. Box 328, Chestertown, MD 21620, USA.

Take Command is a registered trademark of JP Software Inc. TCC is JP
Software Inc.'s trademark for its character-mode command processor. JP 
Software, jpsoft.com, and all JP Software designs and logos are also 
trademarks of JP Software Inc. Other product and company names are 
trademarks of their respective owners.
