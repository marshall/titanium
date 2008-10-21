#!/usr/bin/python2.4
#
# Copyright 2008 Google Inc. All Rights Reserved.

"""Script generate a visual diff of a gvn changebranch for review.

Usage: gvn_review.py "myusername/change_name"
"""

import cgi
import difflib
import os
import re
import subprocess
import sys
import tempfile
import threading


# TODO(aa): There has to be a better way to do this. What is it?
# Add Cheetah's directory to the module search path, then import it
SCRIPT_PATH = os.path.realpath(os.path.dirname(sys.argv[0]))
sys.path.append(SCRIPT_PATH + '/../../../third_party')
from Cheetah.Template import Template


ROOT_URL = 'http://google-gears.googlecode.com/svn'

REVIEW_TEMPLATE = SCRIPT_PATH + '/review.tmpl'


def CheckSVNVersion():
  version_output = RunCommand("svn --version")
  version_pattern = re.compile(r'svn, version (\d+\.\d+\.\d+) ')
  match = version_pattern.match(version_output[0])
  assert match, "Unexpected output from svn version command."
  
  digits = [int(s) for s in match.group(1).split('.')]
  if (digits[0] < 1 or (digits[0] == 1 and digits[1] < 4)):
    print "gvn_review requires at least svn version 1.4"
    sys.exit(1)


def FindBranchSource(change_path):
  """Finds the path that a given change was branched from.
  
  This is hacky because I could not find a clean way to find this information
  using the command line client. I have a dependency on the output of the svn
  log command: I assume that the first line in the output that starts with 'A'
  followed by a path which is prefixed by the change path is the root of the
  branch destination.
  
  Args:
    change_url: URL of the change branch
    
  Returns:
    Tuple containing the source and destination paths of the branch.
  """

  # This regular expression matches lines like this:
  #   A /changes/myusername/change_name (from /change/source/path)
  # The part we're interested in is where it was branched from
  branch_source_pattern = re.compile(r'\s+A (%s/[^/]+) \(from (.+?)\:' %
                                     change_path)

  change_source = None
  change_dest = None

  print 'opening change...'
  change_url = ROOT_URL + change_path
  svn_out = RunCommand('svn log --stop-on-copy -v "%s"' % change_url)
  for line in svn_out:
    match = branch_source_pattern.match(line)
    if match:
      change_source = match.group(2)
      change_dest = match.group(1)
      break

  if not change_source:
    print 'Error: could not find branch point.'
    sys.exit(1)

  print
  print 'branch source: ' + change_source
  print 'branch dest: ' + change_dest
  print

  return change_source, change_dest


def GetChangeDiffs(change_source, change_dest):
  """Gets the differences between the source and destination paths.
  
  Args:
    change_source: Path to the source directory the change was branched from
    change_dest: Path to the change branch
    
  Returns:
    Hash containing the data in a nested dictionary structure expected by
    review.tmpl
  """


  # Get the list of files that differ between the two locations, then download
  change_source_url = ROOT_URL + change_source
  change_dest_url = ROOT_URL + change_dest
  svn_out = RunCommand('svn diff --summarize --old="%s" --new="%s"' %
                        (change_source_url, change_dest_url))

  # Spin off a bunch of threads to go fetch and diff the various files. As they
  # complete, they will fill in the diffs hash, which is keyed by filename.
  diffs = {}
  diffs_lock = threading.Lock()  # Protects multithreaded access to diffs
  threads = []

  # The output from this command looks like this:
  # A      http://google-gears.googlecode.com/svn/contrib/dimitri.glazkov/database2/gears/test/testcases/database_tests.js
  for line in svn_out:
    change_type, source_url = line.split()
    path = source_url[len(change_source_url):]
    dest_url = change_dest_url + path
    if change_type == 'A':  # Added
      threads.append(threading.Thread(
          target=ProcessAddedOrRemoved,
          args=(diffs, diffs_lock, path, dest_url, '+')))
    elif change_type == 'D':  # Deleted
      threads.append(threading.Thread(
          target=ProcessAddedOrRemoved,
          args=(diffs, diffs_lock, path, source_url, '-')))
    else:
      # Modified
      assert change_type == 'M', 'Unexpected change type: ' + change_type
      threads.append(threading.Thread(target=ProcessChanged,
          args=(diffs, diffs_lock, path, source_url, dest_url)))
    threads[-1].start()

  # wait for them all to finish
  for thread in threads:
    thread.join()

  return diffs


def ProcessAddedOrRemoved(diffs, diffs_lock, path, url, change_type):
  """Fetches a file from SVN that was either added or removed (not changed) and
  adds information about the change to the diffs dictionary.
  
  Args:
    diffs: Dictionary to add details to. The structure is:
      dict[file_path] -> array of dictionaries, each containing:
      - type: '+' or '-'
      - content: The content of the line that was added or removed
      - sourceline: The line in the source file that was removed
      - destline: The line in the destination file that was added
    path: Path of the file to fetch.
    url: URL of the file to fetch.
    change_type: The type of change. Either 'A' or 'D'.
    
  Returns:
    None
  """

  lines = RunCommand('svn cat "%s"' % url)
  result = []
  numdigits = len(str(len(lines)))
  emptylinefield = ''.ljust(numdigits)

  for i, line in enumerate(lines):
    lineinfo = {'type': change_type, 'content': FixupLine(line)}
    linefield = str(i+1).rjust(numdigits)

    if change_type == '-':
      lineinfo['source'] = linefield
      lineinfo['dest'] = emptylinefield
    else:
      assert change_type == '+'
      lineinfo['source'] = emptylinefield
      lineinfo['dest'] = linefield

    result.append(lineinfo)

  diffs_lock.acquire()
  try:
    diffs[path] = result
  finally:
    diffs_lock.release()

  print path


def ProcessChanged(diffs, diffs_lock, path, source_url, dest_url):
  """Analyze the differences between two URLs in SVN. 
  
  Args:
    diffs: Dictionary to add details to. The structure is:
      dict[file_path] -> array of dictionaries, each containing:
      - type: '+' or '-' (modifications are represented as a deleted then add
      - content: The content of the line that was added or removed
      - sourceline: The line in the source file that was removed
      - destline: The line in the destination file that was added
    path: Path of the file that was changed.
    source_url: URL of the source file.
    dest_url: URL of the destination file.
    
  Returns:
    None
  """

  source = RunCommand('svn cat "%s"' % source_url)
  dest = RunCommand('svn cat "%s"' % dest_url)
  diff = difflib.Differ().compare(source, dest)
  result = []

  sourceline = 1
  destline = 1
  for line in diff:
    change_type = line[0]
    lineinfo = {'type': change_type, 'content': FixupLine(line[2:])}

    # filter out the '?' lines; we don't use these for now
    if change_type == '?':
      continue

    assert change_type in ('+', '-', ' ')

    if change_type != '+':
      lineinfo['source'] = sourceline
      sourceline += 1
    else:
      lineinfo['source'] = ''

    if change_type != '-':
      lineinfo['dest'] = destline
      destline += 1
    else:
      lineinfo['dest'] = ''

    result.append(lineinfo)

  # Pad each line number to the length of the longest one, so that they line up
  # correctly in the monospaced output.
  sourcedigits = len(str(sourceline))
  destdigits = len(str(destline))

  for lineinfo in result:
    lineinfo['source'] = str(lineinfo['source']).rjust(sourcedigits)
    lineinfo['dest'] = str(lineinfo['dest']).rjust(destdigits)

  diffs_lock.acquire()
  try:
    diffs[path] = result
  finally:
    diffs_lock.release()

  print path


def GenerateHtml(change_name, diffs):
  """Generates an HTML file containing a visual diff.
  
  Args:
    change_name: The name of the change branch.
    diffs: A dictionary containing the details of the differences.
    
  Returns:
    None
  """

  searchList=[{'name': change_name, 'files': diffs}]

  output_file = os.path.join(tempfile.gettempdir(), change_name + '.html')
  output_dir = os.path.dirname(output_file)
  if not os.path.exists(output_dir):
    os.makedirs(output_dir)

  output = open(output_file, 'w')
  output.write(str(Template(file=REVIEW_TEMPLATE,
                            searchList=searchList)))
  output.close()

  print
  print 'Done! Open yonder file in browser: ' + output_file


def RunCommand(command):
  """Runs a command in a subproces. The subprocesses module in python is not
  threadsafe, and ocassionally fails. See: http://bugs.python.org/issue1731717.
  As a workaround, we retry each command up to 5 times.
  
  Args:
    command: The command to run. Can rely on shell expansion.
    
  Returns:
    List of output lines for the command.
  """

  for i in range(0, 5):
    try:
      run = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
      out, err = [e.splitlines() for e in run.communicate()]
      return out
    except:
      print 'failed running command: ' + command
      if i == 4:
        print 'Giving up.'
        raise
      else:
        print 'trying again...'
        pass


def FixupLine(line):
  """Post-processes a line of output from svn diff before it is added to the
  HTML output. Justify the line to 80 columns, escape HTML chars, etc.
  
  Args:
    line: Line of text to process.
    
  Returns:
    Altered line.
  """

  line = line.ljust(80)
  line = cgi.escape(line)

  # Highlight tab and carriage return characters, which should not be used.
  line = line.replace('\t', '<span class="badchar">&#187;</span>')
  line = line.replace('\r', '<span class="badchar">&#171;</span>')
  return line


def main():
  if len(sys.argv) != 2:
    print 'Usage:  %s "my_username/my_change"' % os.path.basename(sys.argv[0])
    sys.exit(1)

  CheckSVNVersion()

  change_name = sys.argv[1]
  change_path = '/changes/' + change_name

  change_source, change_dest = FindBranchSource(change_path)
  diffs = GetChangeDiffs(change_source, change_dest)

  GenerateHtml(change_name, diffs)


if __name__ == "__main__":
  main()
