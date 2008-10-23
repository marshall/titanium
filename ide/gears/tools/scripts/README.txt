This directory contains developer scripts. These scripts were created to aid
project contributors with setting up and using Gears development environment.

Briefly on each script:

* gear-up.cmd -- verifies and provides options for satisfying Gears development
    environment requirements. This script is called by all other scripts, so
    it typically does not have to be launched independently

* build.cmd -- builds Gears, taking 2 arguments: MODE=(dbg|opt)
    and BROWSER=(FF|IE|NPAPI)
    
* clean.cmd -- cleans Gears project output directories

* rebuild.cmd -- successively runs clean.cmd and build.cmd, passing command
    arguments to the latter
    
* run-ide.cmd -- launches Visual Studio 2005 or Visual C++ Express and opens
    VC++ project, generated from the current working copy.
    "build.cmd", "clean.cmd", and "rebuild.cmd" are used by the generated
    project to allow building, cleaning, and respectively, rebuilding
    the project from inside the IDE.
    
* update.cmd -- updates the working copy using Subversion.

* win32-reqs.csv -- contains Win32 development environment requirements. This
    file is used by "gear-up.cmd" and should not be edited directly.

