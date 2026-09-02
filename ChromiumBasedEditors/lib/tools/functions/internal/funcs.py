# Copyright (C) Ascensio System SIA, 2009-2026
#
# This program is a free software product. You can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License (AGPL)
# version 3 as published by the Free Software Foundation, together with the
# additional terms provided in the LICENSE file.
#
# This program is distributed WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
# details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
#
# You can contact Ascensio System SIA by email at info@onlyoffice.com
# or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
# LV-1050, Latvia, European Union.
#
# The interactive user interfaces in modified versions of the Program
# are required to display Appropriate Legal Notices in accordance with
# Section 5 of the GNU AGPL version 3.
#
# No trademark rights are granted under this License.
#
# All non-code elements of the Product, including illustrations,
# icon sets, and technical writing content, are licensed under the
# Creative Commons Attribution-ShareAlike 4.0 International License:
# https://creativecommons.org/licenses/by-sa/4.0/legalcode
#
# This license applies only to such non-code elements and does not
# modify or replace the licensing terms applicable to the Program's
# source code, which remains licensed under the GNU Affero General
# Public License v3.
#
# SPDX-License-Identifier: AGPL-3.0-only

# -*- coding: utf-8 -*-

import os
import sys
from pathlib import Path

LICENSE = "/*\n * Copyright (C) Ascensio System SIA, 2009-2026\n *\n * This program is a free software product. You can redistribute it and/or\n * modify it under the terms of the GNU Affero General Public License (AGPL)\n * version 3 as published by the Free Software Foundation, together with the\n * additional terms provided in the LICENSE file.\n *\n * This program is distributed WITHOUT ANY WARRANTY; without even the implied\n * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For\n * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html\n *\n * You can contact Ascensio System SIA by email at info@onlyoffice.com\n * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,\n * LV-1050, Latvia, European Union.\n *\n * The interactive user interfaces in modified versions of the Program\n * are required to display Appropriate Legal Notices in accordance with\n * Section 5 of the GNU AGPL version 3.\n *\n * No trademark rights are granted under this License.\n *\n * All non-code elements of the Product, including illustrations,\n * icon sets, and technical writing content, are licensed under the\n * Creative Commons Attribution-ShareAlike 4.0 International License:\n * https://creativecommons.org/licenses/by-sa/4.0/legalcode\n *\n * This license applies only to such non-code elements and does not\n * modify or replace the licensing terms applicable to the Program's\n * source code, which remains licensed under the GNU Affero General\n * Public License v3.\n *\n * SPDX-License-Identifier: AGPL-3.0-only\n */\n\n"

def generate_info(folder_path):
  if not os.path.exists(folder_path):
    return False

  cpp_files = []
  for directory in os.listdir(folder_path):
    dir_path = os.path.join(folder_path, directory)
    if not os.path.isdir(dir_path):
      continue
    if (directory == "internal"):
      continue
    cpp_files.append(directory)

  for folder in cpp_files:
    file_cpp = os.path.join(folder_path, folder) + "/main.cpp"
    file_info = os.path.join(folder_path, folder) + "/info.json"

    info_data = ""
    cpp_data = ""
    with open(file_cpp, "r", encoding="utf-8") as file:
      cpp_data = file.read()
    with open(file_info, "r", encoding="utf-8") as file:
      info_data = file.read().strip()

    start = cpp_data.find("std::string description()")
    end = cpp_data.find("}\";\n\t}")

    new_cpp_data = cpp_data[:start] + "std::string description()\n\t{\n\t\treturn \"\\\n"

    json_correct = info_data
    json_correct = json_correct.replace("\\", "\\\\")
    json_correct = json_correct.replace("\"", "\\\"")
    json_correct = json_correct.replace("\n", "\\\n")

    new_cpp_data += json_correct
    new_cpp_data += cpp_data[end+1:]

    os.remove(file_cpp)
    with open(file_cpp, 'w', encoding='utf-8') as f:
      f.write(new_cpp_data)

  return

def generate_header_file(folder_path, output_path="./funcs.h"):
  if not os.path.exists(folder_path):
    return False

  cpp_files = []
  for directory in os.listdir(folder_path):
    dir_path = os.path.join(folder_path, directory)
    if not os.path.isdir(dir_path):
      continue
    if (directory == "internal"):
      continue
    cpp_files.append(directory)

  header_content = generate_header_content(cpp_files)
  with open(output_path, 'w', encoding='utf-8') as f:
    f.write(header_content)
  return

def generate_header_content(cpp_files):
  # The generated header carries the same licence block as every other
  # source file; without it each run of this script would strip it.
  content = LICENSE
  content += "#pragma once\n"
  content += "#include \"./base.h\"\n"

  for cpp_file in cpp_files:
    content += f"#include \"../{cpp_file}/main.cpp\"\n"
    
  content += """
struct TFuncInstance
{
\tstd::string name;
\tstd::function<std::string(std::string, CAIToolsHelper*)> func;
\tTFuncInstance(const std::string& n, std::function<std::string(const std::string&, CAIToolsHelper*)> f)
\t\t: name(n), func(f) {}
};

class CFunctions
{
public:
\tstd::map<std::string, TFuncInstance> m_funcs;
\tCFunctions()
\t{
"""
    
  for cpp_file in cpp_files:
    content += f'\t\tm_funcs.insert(std::make_pair("{cpp_file}", TFuncInstance({cpp_file}::description(), {cpp_file}::main)));\n'
    
  content += "\t}\n};\n"
  return content

def main():
  generate_header_file("./..", "./funcs.h")
  generate_info("./..")
  return

if __name__ == "__main__":
  main()