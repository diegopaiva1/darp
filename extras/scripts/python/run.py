#!/usr/bin/env python3

# This script runs a instance or a group of instances a given amount of times and then stores the results
# of each run in a .xlsx file under the 'results' folder.

# Standard libraries
import sys
import os
import re
import string
import readline

# 3rd party packages
import xlsxwriter

if sys.version_info < (3, 0, 0):
  sys.stderr.write("You need python 3.0.0 or later to run this script\n")
  exit(1)

def storeXlsx(filePath):
  # Create a workbook and add a worksheet
  workbook  = xlsxwriter.Workbook(filePath.replace("temp", "xlsx"))
  worksheet = workbook.add_worksheet()

  # Worksheet columns
  cols = ["#Run", "TT", "ERT", "Cost", "CPU (min)", "Vehicles", "Best iteration", "Best alpha", "Seed"]

  # Width of columns 1 to 5 set to 10
  worksheet.set_column(1, 5, 10)

  # Width of columns 6 to 8 set to 12
  worksheet.set_column(6, 8, 12)

  # Start from the first cell (rows and columns are zero indexed)
  row = 0

  # Write header
  for i, col in enumerate(cols):
    if i == 0:
      form = {'bold': True, 'align': 'center'}
    else:
      form = {'bold': True, 'align': 'right'}

    worksheet.write(row, i, col, workbook.add_format(form))

  # Data from the runs that we want to write to the worksheet
  # Each element from 'runs' list will be a tuple with size = len(cols)
  runs = []

  # Open the file to read the content of each run
  file = open(filePath, 'r')

  # Append all lines from file in 'runs' list except the first one (the header)
  for i, line in enumerate(file):
    if i != 0:
      data = line.split(";")
      runs.append(data)

  # Iterate over the data and write it out row by row
  for r, data in enumerate(runs, 1):
    # Write run number
    worksheet.write_number(row + 1, 0, r, workbook.add_format({'align': 'center'}))

    # Write every other item in data
    for c, item in enumerate(data, 1):
      worksheet.write_number(row + 1, c, float(item), workbook.add_format({'align': 'right'}))

    row += 1

  # Last rows format
  topBorderCenter    = {'bold': True, 'align': 'center', 'border': 2, 'bottom': 0, 'left': 0, 'right': 0}
  topBorderRight     = {'bold': True, 'align': 'right',  'border': 2, 'bottom': 0, 'left': 0, 'right': 0}
  bottomBorderCenter = {'bold': True, 'align': 'center', 'border': 2, 'top':    0, 'left': 0, 'right': 0}
  bottomBorderRight  = {'bold': True, 'align': 'right',  'border': 2, 'top':    0, 'left': 0, 'right': 0}

  # Collect the minimum value and the average of all columns except column 0
  for i, col in enumerate(cols):
    if i == 0:
      worksheet.write(row + 1, 0, 'Min', workbook.add_format(topBorderCenter))
      worksheet.write(row + 2, 0, 'Max', workbook.add_format({'bold': True, 'align': 'center'}))
      worksheet.write(row + 3, 0, 'Avg', workbook.add_format({'bold': True, 'align': 'center'}))
      worksheet.write(row + 4, 0, 'SD',  workbook.add_format(bottomBorderCenter))
    else:
      # Access the letter of the cell and last row that contains a number
      letter = string.ascii_uppercase[i]
      last   = str(len(runs) + 1)

      worksheet.write(row + 1, i, '=MIN('     + letter + '2:' + letter + last + ')', workbook.add_format(topBorderRight))
      worksheet.write(row + 2, i, '=MAX('     + letter + '2:' + letter + last + ')', workbook.add_format({'bold': True, 'align': 'right'}))
      worksheet.write(row + 3, i, '=AVERAGE(' + letter + '2:' + letter + last + ')', workbook.add_format({'bold': True, 'align': 'right'}))
      worksheet.write(row + 4, i, '=STDEV('   + letter + '2:' + letter + last + ')', workbook.add_format(bottomBorderRight))

  workbook.close()

# Tab-complete feature for inputing files from file system
readline.set_completer_delims(" \t\n=")
readline.parse_and_bind("tab: complete")

# Pretty print title
os.system("figlet -f slant 'DARP'")

print("(a) Run a single instance")
print("(b) Run a data set\n")

option = input("Choose an option: ")

# Check if option is valid
if not re.match("^[aAbB]$", option):
  print("\nError: invalid option")
  exit(1)

runs = input("\nNumber of runs: ")

# Check if number of runs is valid
if not re.match("^[1-9][0-9]*$", runs):
  print("\nError: number of runs must be integer greater than 0")
  exit(1)

# Parse to int so we can use it in range() later
runs = int(runs)

# List of instances we will run the algorithm on
instances = []

if re.match("^[aA]$", option):
  paths = input("\nEnter instance path (separe multiple datasets with a space): ").split(" ")

  for path in paths:
    if not os.path.isfile(path):
      print("\nError: " + path + " is not a file")
      exit(1)

    instances.append(path)
else:
  instancesDir = input("\nEnter instances directory (separe multiple datasets with a space): ").split(" ")

  for dir in instancesDir:
    if not os.path.isdir(dir):
      print("\nError: " + dir + " is not a directory")
      exit(1)
    elif not dir.endswith("/"):
      dir += "/"

    for file in sorted(os.listdir(dir)):
      instances.append(dir + file)

# Before running let's compile the binary to make sure we are running on the most recent version
os.system("cmake build && make -C build")

for instance in instances:
  # Results directory folder
  dir = re.sub('instances', 'results', instance)
  dir = re.sub('[a-zA-Z][0-9]+.*', '', dir)

  if not os.path.exists(dir):
    os.makedirs(dir, exist_ok = True)

  # We then append the current timestamp to the file name
  path = re.sub('instances', 'results', instance) + os.popen("date '+%Y-%m-%d_%H:%M:%S.temp'").read().replace("\n", "")

  for i in range(runs):
    os.system("build/e-adarp " + instance + " " + path)

  # Write the results to xlsx file and then remove the temp file
  storeXlsx(path)
  os.system("rm -f " + path)
