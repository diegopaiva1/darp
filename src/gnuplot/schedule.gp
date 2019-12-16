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

# Do not show that weird heatmap box in the right of the screen
unset colorbox

# Draw horizontal lines
do for [i = 0 : ARG3] {
    set arrow from graph 0,first i to graph 1,first i nohead lc rgb "black" front
}

plot ARG1 i 0 w p pt 7 ps 2 palette notitle, \
     ARG1 i 0 u 1:2:(sprintf("A = %.4g", $1)) w labels offset char 0,1.5 notitle, \
     ARG1 i 1 w p pt 7 ps 2 palette notitle, \
     ARG1 i 1 u 1:2:(sprintf("B = %.4g", $1)) w labels offset char 0,1.5 notitle, \
     ARG1 i 2 w p pt 7 ps 2 palette notitle, \
     ARG1 i 2 w lines lc rgb "black" notitle, \
     ARG1 i 3 pt 1 ps 2 lc rgb "black" notitle
