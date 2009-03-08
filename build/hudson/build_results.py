#!/usr/bin/env python

import os, sys, re, time
import getopt, subprocess
import shutil, distutils.dir_util as dir_util
import hudson
from Cheetah.Template import Template

hudson_config = hudson.HudsonConfig()
results_file = open("index.html", "r")
results_template = results_file.read()
results_file.close()

timestamp = time.localtime()
if os.environ.has_key("BUILD_TAG"):
	timestamp = os.environ["BUILD_TAG"]

results = Template(results_template, searchList = [{"hudson": hudson_config,
											"environ": os.environ,
											"timestamp": timestamp}])
index_html = os.path.join(hudson_config.results_dir, "index.html")
index_html_file = open(index_html, "w")
index_html_file.write(str(results))
index_html_file.close()

shutil.copy("index.css", hudson_config.package_dir)