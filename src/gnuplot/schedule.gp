# This script must receive 3 command-line args:

# 1st: File containing the data to be plotted;
# 2nd: Output file name;
# 3rd: Amount of nodes, which defines the range of the y-axis.

# Deletes any user-defined variables and functions and restores default settings
reset session

# Produce PNG output. The 'enhanced' keyword adds support for additional text formatting
set term pngcairo size 1920, 1080 enhanced font "Verdana, 12"

# Where the resulting plot will be stored
set output ARG2

set xlabel "Time"
set ylabel "Index"

set yrange [0 : ARG3]

# Color numbers we will work on
set cbrange [0 : 10]

# Our custom palette
set palette defined (0 "black", 1 "magenta", 2 "green", 3 "blue", 4 "red", 5 "cyan", 6 "violet", 7 "gold", 8 "purple", 9 "grey", 10 "dark-green")

# Do not show the big heatmap box in the right of the screen
unset colorbox

# Draw horizontal lines
do for [i = 0 : ARG3] {
    set arrow from graph 0,first i to graph 1,first i nohead lc rgb "black" front
}

plot ARG1 i 0 u 1:2:3 w p pt 7 ps 2 palette notitle, \
     ARG1 i 0 u 1:2:(sprintf("A")) w labels offset char 0,1.5 notitle, \
     ARG1 i 1 u 1:2:3 w p pt 7 ps 2 palette notitle, \
     ARG1 i 1 u 1:2:(sprintf("B")) w labels offset char 0,1.5 notitle, \
     ARG1 i 2 u 1:2:3 w p pt 7 ps 2 palette notitle, \
     ARG1 i 2 u 1:2:(sprintf("D")) w labels offset char 0,1.5 notitle, \
     ARG1 i 3 w lines lw 2 lc rgb "black" notitle, \
     ARG1 i 4 pt 1 ps 2 lc rgb "black" notitle

