# Deletes any user-defined variables and functions and restores default settings
reset session

# Produce PNG output. The 'enhanced' keyword adds support for additional text formatting
set term pngcairo enhanced font "Verdana, 12"

# Where the image will be stored
set output ARG2

# Title of the plot
set title "{/Symbol a} probability distribution"

set xlabel "{/Symbol a}"
set ylabel "Probability"

# Number of intervals
n = ARG3

# Min and max values of x-axis
min = ARG4
max = ARG5

# Define ranges for both axis
set xrange [min:max]
set yrange [0:]

# To put an empty boundary around the data inside an autoscaled graph
set offset graph 0.05, 0.05, 0.025, 0.0

# Interval width
width = (max - min)/n

# Histogram box width and style
set boxwidth width * 0.6
set style fill solid 0.5

set tics out nomirror

plot ARG1 u 1:2:xtic(1):ytic(2) smooth freq w boxes lc rgb "blue" notitle
