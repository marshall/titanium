#!/usr/bin/python2.4
#
# Copyright 2007 Google Inc. All Rights Reserved.

"""Script to auto-generate a VS 2005 project file from the Gears source tree.

Project file is generated in gears/tools/gears.vcproj. Gears is built from a
Makefile so this project can't be used to build Gears directly (to do so, setup
make.bat as an external tool in your VS environment).

This script is Windows only and must reside in the gears/tools directory, it 
will not run if this is not the case.

Assumptions:
* All paths in this script are specified relative to the svn root directory. 
On output, all paths are converted to be absolute.
* bin-* & third_party directories are not added to the project file.

  Templatizer: A bare-bones templating engine.
  DevEnvSettings: Encapsulate the settings for a build target.
  CreateTargets(): Construct DevEnvSettings for all build targets.
  LoadFolderContents(): Collect source files and output xml for their inclusion.
  CreateConfigurationXML(): Main entry point to create project file xml.
"""

__author__ = 'playmobil@google.com (Jeremy Moskovich)'

import os
import re
import stat
import sys

# Module Constants

OUTPUT_FILE_NAME = 'gears.vcproj'

# VS Project template for use with Templatizer.

VS_PROJECT_TEMPLATE_NAME = 'vs_project_template.py'

# Template Identifiers - see description inside template file.

TARGET_SECTION_TEMPLATE = 'TARGET_SECTION'
TARGET_NAME_TAG = 'TARGET_NAME'
BROWSER = 'BROWSER'
MODE = 'MODE'
INCLUDE_DIRS_TAG = 'INCLUDE_DIRECTORIES'
PREPROCESSOR_DEFINES_TAG = 'PREPROCESSOR_DEFINES'

MAIN_VS_PROJECT_SKELETON_TEMPLATE = 'MAIN_VS_PROJECT_SKELETON'
TARGET_SECTIONS_TAG = 'TARGET_SECTIONS'
FILES_SECTION_TAG = 'FILES_SECTION'

FILE_GROUP = 'FILE_GROUP'
FILE_GROUP_NAME_TAG = 'FILE_GROUP_NAME'
FILE_LIST_TAG = 'FILE_LIST'

FILE_DEFINITION_GROUP = 'FILE_DEFINITION'
REL_PATH_TO_FILE_TAG = 'REL_PATH_TO_FILE'
FILE_EXCLUSIONS_TAG = 'FILE_EXCLUSIONS'

BUILD_EXCLUDE_GROUP_TAG = 'BUILD_EXCLUDE_GROUP'
EXCLUDE_TARGET_NAME_TAG = 'EXCLUDE_TARGET_NAME'


def _ParentDir(the_dir):
  """Return parent of directory in an OS independent manner.
  
  If the root directory is passed in then this function will return a
  normalised version of it's input e.g. an input of c:\\ gives c:\ as output.
  
  Args:
    the_dir: a string whose parent we want to retrieve.
    
  Returns:
    A string representing the absolute path of the parent directory.
  """
  the_dir = os.path.abspath(the_dir)
  the_dir = os.path.normpath(the_dir)
  
  if the_dir == os.sep or re.match(r'\[a-zA-Z]:\\*', the_dir):
    return the_dir
  else:
    return os.sep.join(the_dir.split(os.sep)[:-1])
    
# Absolute path to tools directory

TOOLS_DIR_PATH = os.path.abspath(os.path.dirname(__file__))

# Absolute path to svn_root/gears/ directory

GEARS_DIR_PATH = _ParentDir(TOOLS_DIR_PATH)

# Parent of gears/ directory (corresponds to svn root directory).

REPO_ROOT_PATH = _ParentDir(GEARS_DIR_PATH)


class Templatizer(object):
  """Simple templating engine.
  
  Templates are simply Python files which are evaled. All local variables in
  these .py files are assumed to be strings. The template's name is just the
  name of the string in the .py file.
  Within each string %(IDENTIFIER)s tags can be replaced with content.
  """
  
  class InvalidTempateNameException(Exception):
    """Raised when attempting to access a non-existent template."""
    pass
  
  def __init__(self, data_file_name):
    """Loads & parses template file.
    
    Args:
      data_file_name - A string representing the filename of a .py file to load.
    """
    
    # Templates are loaded into this dictionary - the keys are template names 
    # and the values are the strings associated with them.
    
    self.templates = {}
    
    # Load templates
    execfile(data_file_name, {}, self.templates)
    
  def SubstituteIntoTemplate(self, template_name, vars):
    """Performs template substitution.
    
    Templates are strings in the format %(IDENTIFIER)s. Here we substitute
    each IDENTIFIER with a value.
    
    Note: all identifiers in a template must have entries in vars, otherwise
    an exception will be thrown.
    
    Args:
      template_name: A string representing the name of the template we wish to
        evaluate. This is simply the name of a local variable in the .py 
        template file.
      vars: Dictionary of parameters to substitute into the template. keys are 
        strings representing the placeholders in the given template. values are
        strings that we substitute in place.
                      
    Returns:
      A string representing the instantiated template.
    """
    
    if not self.templates.has_key(template_name):
      raise InvalidTempateNameException(
          "Attempt to access invalid template: %s" % template_name)
      
    ret = self.templates[template_name] % vars
      
    return ret
    
    
class DevEnvSettings(object):
  """Class to encapsulate compilation target.
  
  Instances of this class have target names, include paths & preprocessor
  symbols.
  
  Instances of this class can be composed together using the constructor
  so we can create an object containing for example the DEBUG define and an
  include path for a debug library and then incorporate that into multiple build
  targets by composing it in. If an extra define is added to all the debug
  targets, we need only change that one object and all other debug targets
  will inherit from it.
  """
  
  def __init__(self, target_name=None, browser=None, mode=None, settings_objects_to_inherit_from=None):
    """Create an object with a given name composed of 0 or more other targets.
    
    Args:
      target_name: String representing the name for this target, None is useful
        when we are just creating an anonymous target for use in composition.
      browser: String representing the targeted browser (IE|FF)
      mode: String, targeted mode (opt|dbg)
      settings_objects_to_inherit_from: List of zero or more DevEnvSettings 
        objects to compose this object from. Composition is performed by
        copying the contents of the include dirs & preprocessor defines from
        each of these objects in order. So the order of this array matters!           
    """
    
    self.target_name = target_name
    self.browser = browser
    self.mode = mode
    self.includes = []
    self.preprocessor_symbols = []

    if settings_objects_to_inherit_from:
      self._CopyFrom(settings_objects_to_inherit_from)

  def _CopyFrom(self, settings_objects_to_inherit_from):
    """Implement inheritance by copying values from sibling objects.
    
    Args:
      settings_objects_to_inherit_from: List of 0 or more DevEnvSettings objects
        to inherit from.
    """

    assert(isinstance(settings_objects_to_inherit_from, list))
      
    for parent in settings_objects_to_inherit_from:
      self.includes.extend(parent.includes)
      self.preprocessor_symbols.extend(parent.preprocessor_symbols)
      
  def AddPreprocesorSymbol(self, symbol):
    """Add a preprocessor symbol to this target.
    
    Args:
      symbol: A string containing the item to add to the end of the list of
        preprocessor symbols.
    """
    
    assert(isinstance(symbol, str))
    
    self.preprocessor_symbols.append(symbol)
    
  def AppendIncludes(self, include_paths):
    """Append an include path to the end of the list of include directories.
    
    Path must be relative to the svn root directory e.g. 'gears/..'.
    
    Args:
      include_paths: A string containing the include path to add to this target.
    """  
    
    assert(isinstance(include_paths, list))
      
    self.includes.extend(include_paths)
      
  def GetFlattenedIncludeDirs(self):
    """Get include path string suitable for including in VS XML.
    
    Returns:
      Retrieve a string containing a ; delimited concatenation of all include 
      paths contained in the object.
    """
    
    absolute_include_paths = []
    for include_path in self.includes:
      abs_path = os.path.join(REPO_ROOT_PATH, include_path)
      absolute_include_paths.append(abs_path)
    
    return ';'.join(absolute_include_paths)
    
  def GetFlattenedPreprocessorDefines(self):
    """Get Preprocessor define string suitable for inclusion in VS XML."""
    
    return ';'.join(self.preprocessor_symbols)


def CreateTargets():
  """Creates all the compilation target objects.
  
  Returns:
   List of DevEnvSettings objects that should be included in the VS Project 
   file.
  """
  
  # TODO(playmobil): Add support for platforms other than Win32.
  
  common_settings = DevEnvSettings()

  # Settings common to all Configurations
  
  common_settings.AppendIncludes(
      [r'gears\..',  # root of source directory a.k.a. svn root.
       r'gears\bin-dbg\win32-i386', 
       r'gears\third_party\zlib', 
       r'gears\third_party\breakpad\src'])
                            
  common_settings.preprocessor_symbols = ['PNG_USER_CONFIG',
                                          'NO_GZIP',
                                          'NO_GZCOMPRESS',
                                          'WIN32',
                                          'UNICODE',
                                          'NOMINMAX' 
                                          'BREAKPAD_AVOID_STREAMS']

  # Settings For Debug & Release Builds
                            
  debug_settings = DevEnvSettings()
  debug_settings.AddPreprocesorSymbol('DEBUG')

  release_settings = DevEnvSettings()
  release_settings.AddPreprocesorSymbol('NDEBUG')
                                       
  # Settings Common to all IE Targets
  
  common_ie_settings = DevEnvSettings(
      target_name=None,
      settings_objects_to_inherit_from=[common_settings])
  common_ie_settings.AddPreprocesorSymbol('BROWSER_IE')

  # Settings Common to all FF Targets
  
  common_ff_settings = DevEnvSettings(
      target_name=None,
      settings_objects_to_inherit_from=[common_settings])
  common_ff_settings.AddPreprocesorSymbol('BROWSER_FF')
  common_ff_settings.AddPreprocesorSymbol('MOZILLA_STRICT_API')
  common_ff_settings.AppendIncludes(
      [r'gears\third_party\gecko_1.8\win32'
       r'gears\third_party\gecko_1.8\win32\gecko_sdk\include',
       r'gears\third_party\gecko_1.8'])
      
  # Settings Common to all NPAPI Targets
  
  common_npapi_settings = DevEnvSettings(
      target_name=None, settings_objects_to_inherit_from=[common_settings])
  common_npapi_settings.AddPreprocesorSymbol('BROWSER_NPAPI')
  common_npapi_settings.AppendIncludes(
      [r'gears\third_party\gecko_1.8\win32'
       r'gears\third_party\gecko_1.8\win32\gecko_sdk\include'])

  # IE Targets
  
  ie_release_target = DevEnvSettings(
      target_name='IE Release',
      browser='IE',
      mode='opt',
      settings_objects_to_inherit_from=[common_ie_settings, release_settings])
  ie_debug_target = DevEnvSettings(
      target_name='IE Debug',
      browser='IE',
      mode='dbg',
      settings_objects_to_inherit_from=[common_ie_settings, debug_settings])

  # FF Targets
  
  ff_release_target = DevEnvSettings(
      target_name='FF Release',
      browser='FF',
      mode='opt',
      settings_objects_to_inherit_from=[common_ff_settings, release_settings])
  ff_debug_target = DevEnvSettings(
      target_name='FF Debug',
      browser='FF',
      mode='dbg',
      settings_objects_to_inherit_from=[common_ff_settings, debug_settings])
      
  # NPAPI Targets
  
  npapi_release_target = DevEnvSettings(
      target_name='NPAPI Release',
      browser='NPAPI',
      mode='opt',
      settings_objects_to_inherit_from=[common_npapi_settings, 
                                        release_settings])
  npapi_debug_target = DevEnvSettings(
      target_name='NPAPI Debug',
      browser='NPAPI',
      mode='dbg',
      settings_objects_to_inherit_from=[common_npapi_settings, debug_settings])
  
  return [ie_release_target, ie_debug_target, 
          ff_release_target, ff_debug_target,
          npapi_release_target, npapi_debug_target]
  
  
def LoadFolderContents(folder_path, templatizer):
  """Recursively traverse project dirs and get the corresponding VS XML.

  Args:
    folder_path: Path to folder to traverse.
    templatizer: Templatizer object used to do template substitution to generate
      XML for each file.
      
  Returns:
    A string containing the XML representing the directory structure for the
    directory specified by folder_path.
  """

  ret_list = []

  names = os.listdir(folder_path)

  for name in names:
    if name.startswith("."):
      continue

    full_path = os.path.join(folder_path, name)

    try:
      st = os.lstat(full_path)
    except os.error:
      continue
    if stat.S_ISDIR(st.st_mode):

      # TODO(aa): Consider a more data-driven approach to ignoring directories
      # and files.
      if "bin-" in name or "third_party" in name:
        continue
        
      file_list_string = LoadFolderContents(full_path, templatizer)

      template_params = {
          FILE_GROUP_NAME_TAG: name,
          FILE_LIST_TAG: file_list_string}

      ret_list.append(templatizer.SubstituteIntoTemplate(FILE_GROUP,
                                                         template_params))
      
    else:
      
      # TODO(playmobil): exclude files based on targets using 
      # BUILD_EXCLUDE_GROUP
      
      template_params =  {
          REL_PATH_TO_FILE_TAG: full_path,
          FILE_EXCLUSIONS_TAG: ''}
      ret_list.append(templatizer.SubstituteIntoTemplate(FILE_DEFINITION_GROUP, 
                                                         template_params))
      
  return '\n'.join(ret_list)
      
def CreateConfigurationXML(target_objects):
  """Main entry point to create VS Project XML file.
  
  Args:
    target_objects: List of DevEnvSettings objects representing all compilation
      targets for the project.
      
  Returns:
    String containing the VS project file.
  """
  
  target_strings = []  # XML definition for each target
  
  # First Load all our templates
  
  template_engine = Templatizer(os.path.join(TOOLS_DIR_PATH, 
                                             VS_PROJECT_TEMPLATE_NAME))
  
  # Construct Targets First
  
  for target in target_objects:
    template_params = {
        TARGET_NAME_TAG: target.target_name + '|Win32',
        BROWSER: target.browser,
        MODE: target.mode,
        INCLUDE_DIRS_TAG: target.GetFlattenedIncludeDirs(),
        PREPROCESSOR_DEFINES_TAG: target.GetFlattenedPreprocessorDefines()}
    
    target_strings.append(template_engine.SubstituteIntoTemplate(
        TARGET_SECTION_TEMPLATE, template_params) )
    
  # Create XML for source files
  
  file_xml = LoadFolderContents(GEARS_DIR_PATH, template_engine)
  
  # Put it all together.
  
  template_params = {TARGET_SECTIONS_TAG: '\n'.join(target_strings),
                     FILES_SECTION_TAG: file_xml}
                     
  return template_engine.SubstituteIntoTemplate(
      MAIN_VS_PROJECT_SKELETON_TEMPLATE, template_params)
  
def main():
  
  # Sanity check for OS version & script location
  
  if os.name != 'nt':
    exit("Script can run only under Windows.")
  
  if os.path.basename(TOOLS_DIR_PATH) != 'tools':
    exit('This script must reside in the tools directory in order to run.')
  
  vs_project_filename = os.path.join(TOOLS_DIR_PATH, OUTPUT_FILE_NAME)
  
  # Open output VS project file.
  
  try:
    output_file = open(vs_project_filename, 'w')
  except IOError, err:
    exit("Got error: %s\nIs your VS project file (%s) editable?" %
         (err.strerror, vs_project_filename))
  
  targets = CreateTargets()
  
  vs_project_xml = CreateConfigurationXML(targets)
  
  print >> output_file, vs_project_xml
  
  print "VS Project generated successfully"
  
if __name__ == "__main__":
  main()
