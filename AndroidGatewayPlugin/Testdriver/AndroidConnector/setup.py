#!/usr/bin/env python

from setuptools import setup, find_packages
from distutils.spawn import find_executable
import sys
import os
import subprocess

# Protobuf compilation stuff shamelessly stolen from protobuf's setup.py:
# http://code.google.com/p/protobuf/source/browse/trunk/python/setup.py
# (c) 2008 Google Inc.  See $GATEWAY_ROOT/CREDITS for the full license.

VERSION_DEFAULT = "0.3.4"

# Find the Protocol Compiler.
protoc = find_executable("protoc")
sh = find_executable("sh")

def generate_proto(source):
  """Invokes the Protocol Compiler to generate a _pb2.py from the given
  .proto file.  Does nothing if the output already exists and is newer than
  the input."""

  output = source.replace(".proto", "_pb2.py").replace("../src/", "")

  if not os.path.exists(source):
    print "Can't find required file: " + source
    sys.exit(-1)

  if (not os.path.exists(output) or
      (os.path.exists(source) and
       os.path.getmtime(source) > os.path.getmtime(output))):
    print "Generating %s..." % output

    if protoc == None:
      sys.stderr.write(
          "protoc is not installed.  Please compile it "
          "binary package.\n")
      sys.exit(-1)

    protoc_command = [ protoc, "-I../../../common/protocol", "-I.", "--python_out=./ammo", source ]
    if subprocess.call(protoc_command) != 0:
      sys.exit(-1)
      
if __name__ == '__main__':
  # TODO(kenton):  Integrate this into setuptools somehow?
  if len(sys.argv) >= 2 and sys.argv[1] == "clean":
    # Delete generated _pb2.py files and .pyc files in the code tree.
    for (dirpath, dirnames, filenames) in os.walk("."):
      for filename in filenames:
        filepath = os.path.join(dirpath, filename)
        if filepath.endswith("_pb2.py") or filepath.endswith(".pyc") or \
          filepath.endswith(".so") or filepath.endswith(".o"):
          os.remove(filepath)
  else:
    # Generate necessary .proto file if it doesn't exist.
    # TODO(kenton):  Maybe we should hook this into a distutils command?
    generate_proto("../../../common/protocol/AmmoMessages.proto")
    
  # Get project version from git (assumes this is in a git repo)
  print "Getting version number...",
  version_number = VERSION_DEFAULT
  versionProcess = subprocess.Popen(["/bin/sh", "-c", "git describe --match release-\*  | sed 's/release-//' | cut -d- -f 1,2"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  versionOutput = versionProcess.communicate()
  if(versionProcess.returncode == 0):
    version_number = versionOutput[0].replace("\n", "")
    print "Got version", version_number
  else:
    print "An error occurred when getting version number...  using default."
    print versionOutput[1]
  
  setup(
    name = "AndroidConnector",
    version = version_number,
    packages = find_packages(),
    install_requires = ['twisted>=10.0', 'M2Crypto>=0.21.1'],
  )

