# -*- coding: utf-8 -*-

import os
import sys
from pathlib import Path

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
    with open(file_cpp, "r") as file:
      cpp_data = file.read()
    with open(file_info, "r") as file:
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
  content = "#pragma once\n"
  content += "#include \"./base.h\"\n"

  for cpp_file in cpp_files:
    content += f"#include \"../{cpp_file}/main.cpp\"\n"
    
  content += """
struct TFuncInstance
{
\tstd::string name;
\tstd::function<std::string(std::string)> func;
\tTFuncInstance(const std::string& n, std::function<std::string(const std::string&)> f)
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