#
# This script is the property of Appcelerator, Inc. and
# is Confidential and Proprietary. All Rights Reserved.
# Redistribution without expression written permission
# is not allowed.
#
# Titanium application mobile builder class
#
# Original author: Jeff Haynie 04/02/09
#
#
import os, shutil, distutils.dir_util as dir_util

class MobileBuilder(object):
	def __init__(self, options, log):
		self.options = options
		self.logger = log
		self.appname = options.manifest['appname']
		log("Packaging application named: %s, version: %s"%(self.appname,options.version))
	
	def log(self,msg):
		self.logger(self.options,msg)

