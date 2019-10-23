#!/usr/bin/env python3

# This script runs a instance or a group of instances a given amount of times and then stores the results
# of each run in a .xlsx file under the 'results' folder

# Standard libraries
import sys
import os
import re
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

  # Open the file to read the content of each run
  file = open(filePath, 'r')

  # Data from the runs that we want to write to the worksheet
  runs = []

  # Save all lines from file in the list except the first one
  for i, line in enumerate(file):
    if i != 0:
      data = line.split(";")
      runs.append(data)

  # Format to use in header cells
  headerFormat = workbook.add_format({'bold': True, 'align': 'center'})

  # Start from the first cell. Rows and columns are zero indexed
  row = 0

  # Write headers
  worksheet.write(row, 0, '#Run', headerFormat)
  worksheet.write(row, 1, 'Total routes', headerFormat)
  worksheet.write(row, 2, 'Heuristic', headerFormat)
  worksheet.write(row, 3, 'CPLEX', headerFormat)
  worksheet.write(row, 4, 'CPU (s)', headerFormat)

  # Width of column 1 set to 15
  worksheet.set_column(1, 1, 15)

  # Alignment to use in body cells
  center = workbook.add_format({'align': 'center'})

  # Iterate over the data and write it out row by row
  for totalRoutes, heuristicValue, cplexValue, elapsed in runs:
    worksheet.write_number(row + 1, 0, row + 1, center)
    worksheet.write_number(row + 1, 1, float(totalRoutes), center)
    worksheet.write_number(row + 1, 2, float(heuristicValue), center)
    worksheet.write_number(row + 1, 3, float(cplexValue), center)
    worksheet.write_number(row + 1, 4, float(elapsed), center)
    row += 1

  # Last rows format
  topBorder = workbook.add_format({'bold': True, 'align': 'center', 'border': 2, 'bottom': 0, 'left': 0, 'right': 0})
  bottomBorder = workbook.add_format({'bold': True, 'align': 'center', 'border': 2, 'top': 0, 'left': 0, 'right': 0})

  # The '+ 1' stands for the header row that is not present in 'runs' list
  rowsLengthStr = str(len(runs) + 1)

  # Collect the minimum value of all columns except 0
  worksheet.write(row + 1, 0, 'Min', topBorder)
  worksheet.write(row + 1, 1, '=MIN(B2:B' + rowsLengthStr + ')', topBorder)
  worksheet.write(row + 1, 2, '=MIN(C2:C' + rowsLengthStr + ')', topBorder)
  worksheet.write(row + 1, 3, '=MIN(D2:D' + rowsLengthStr + ')', topBorder)
  worksheet.write(row + 1, 4, '=MIN(E2:E' + rowsLengthStr + ')', topBorder)

  # Collect the average value of all columns except 0
  worksheet.write(row + 2, 0, 'Avg', bottomBorder)
  worksheet.write(row + 2, 1, '=AVERAGE(B2:B' + rowsLengthStr + ')', bottomBorder)
  worksheet.write(row + 2, 2, '=AVERAGE(C2:C' + rowsLengthStr + ')', bottomBorder)
  worksheet.write(row + 2, 3, '=AVERAGE(D2:D' + rowsLengthStr + ')', bottomBorder)
  worksheet.write(row + 2, 4, '=AVERAGE(E2:D' + rowsLengthStr + ')', bottomBorder)

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
  filePath = input("\nEnter instance path: ")

  if not os.path.isfile(filePath):
    print("\nError: path is not a valid file")
    exit(1)

  instances.append(filePath)
else:
  instancesDir = input("\nEnter instances directory (separe multiple data sets with a space): ").split(" ")

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
  # The data file will be stored under it's original path but in the results directory
  resultsFilePath = instance.replace("instances", "results")

  # We then remove anything that comes after a dot ('.') and append the current timestamp to the file name
  resultsFilePath = re.sub("\.[^.]*", "", resultsFilePath)
  resultsFilePath += os.popen("date '+%Y-%m-%d_%H:%M:%S.temp'").read().replace("\n", "")

  for i in range(runs):
    os.system("build/e-adarp " + instance + " " + resultsFilePath)

  # Write the results to xlsx file and then remove the temp file
  storeXlsx(resultsFilePath)
  os.system("rm -f " + resultsFilePath)
