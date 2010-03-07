#!/usr/bin/env python
# instead of non-working cpack: svn export for *nix systems
# usage: export_nix.py TARGET_DIR
# the scripts excludes windows-specific files

import os
import sys
import subprocess
import shutil
import fnmatch
from optparse import OptionParser

PATTERNS = [ "*.vcproj", "*.sln", "install_win32", "win32" ]
GPL_COMPATIBLE_LICENSES = [ "GPL", "LGPL", "Public Domain", "MPL", "PPL", "MIT", "BSD" ]

parser = OptionParser()
parser.add_option("-g", "--gpl",
                  action="store_true", dest="gpl",
                  help="export only GPL files (mostly icons)")
parser.set_defaults( gpl = False ) # should be True 
(options, args) = parser.parse_args()
target_dir = args[0]

print "Exporting to target directory '%s'" % target_dir
subprocess.check_call( ["svn", "export", os.curdir, target_dir ] )

def remove_path( path, message ):
    if os.path.isdir( relname ):
        print "[%s] Deleting dir '%s'" % ( message, relname )
        shutil.rmtree( relname )
    else:
        print "[%s] Deleting '%s'" % ( message, relname )
        os.remove( relname )

def filter_path( path ):
    if not options.gpl:
        return True
    else:
        license_path = None
        if os.path.exists( path + ".license" ):
            license_path = path + ".license"
        elif os.path.exists( os.path.join( path, "license" ) ):
            license_path = os.path.join( path, "license" )
        if not license_path:
            return True
        else:
            license_line = open( license_path ).readlines()[0].strip()
            if license_line in GPL_COMPATIBLE_LICENSES:
                return True
            return False

for root, dirs, files in os.walk( target_dir ):
    for relname in [ os.path.join( root, name ) for name in dirs + files ]:
        for pattern in PATTERNS:
            if not fnmatch.fnmatch( os.path.basename( relname ), pattern ):
                continue
            remove_path( relname, "pattern" )
        if not filter_path( relname ):
            remove_path( relname, "license" )

print "Done exporting to target directory '%s'" % target_dir

basename = os.path.basename( target_dir )
archivename = basename + ".tar.bz2"
archivedir = os.path.dirname( os.path.abspath( target_dir ) )
archivepath = os.path.join( archivedir, archivename )

print "Packing '%s' to '%s'" % ( target_dir, archivepath )
subprocess.check_call( ["tar", "--bzip2", "-c", "-f", archivename, basename ], cwd = archivedir )
print "Done"

