#!/usr/bin/env python

import sys
sys.path.append('../../../../../build_tools/scripts')
import base
import os

#compilation_level = "WHITESPACE_ONLY"
compilation_level = "SIMPLE_OPTIMIZATIONS"
base.cmd("java", ["-jar", "../../../../../sdkjs/build/node_modules/google-closure-compiler-java/compiler.jar",
                  "--compilation_level", compilation_level,
                  "--js_output_file", "output.js",
                  "--js", "drop_data.js"])
                  
code_content = base.readFile("output.js")

code_content = code_content.replace("\r", "")
code_content = code_content.replace("\n", "")
code_content = code_content.replace("\\", "\\\\")
code_content = code_content.replace("\"", "\\\"")

base.delete_file("output.js")
base.writeFile("output.js", code_content)