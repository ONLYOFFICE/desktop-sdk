# -*- coding: utf-8 -*-

import os
import sys
from pathlib import Path

def generate_header_file(folder_path, output_path="./funcs.h"):
  if not os.path.exists(folder_path):
    return False

  cpp_files = []
  for file in os.listdir(folder_path):
    if file.endswith('.cpp'):
      filename_without_ext = os.path.splitext(file)[0]
      cpp_files.append(filename_without_ext)

  header_content = generate_header_content(cpp_files)
  with open(output_path, 'w', encoding='utf-8') as f:
    f.write(header_content)
  return

def generate_header_content(cpp_files):
  content = "#pragma once\n"
  content += "#include \"./base.h\"\n"

  for cpp_file in cpp_files:
    content += f"#include \"../{cpp_file}.cpp\"\n"
    
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
  return

if __name__ == "__main__":
  main()