# Deletes any user-defined variables and functions and restores default settings
reset session

# Produce PNG output. The 'enhanced' keyword adds support for additional text formatting
set term pngcairo size 1920, 1080 enhanced font "Verdana, 12"

# Where the image will be stored
set output "../tmp/gnuplot/solution.png"

# Define ranges for both axis
set xrange[-10.5:10.5]
set yrange[-10.5:10.5]

# Data file location
SOLUTION_FILE = "../tmp/gnuplot/solution.dat"

# We collect all the important file info to use in our plot
set table $Header
	plot SOLUTION_FILE i 0 u (INSTANCE_NAME = stringcolumn(1)):(COST = $2):(ROUTES = $3):(REQUESTS = $4):(STATIONS = $5) w table
unset table

# Title of the plot
set title sprintf("%s - Cost = %.2f", INSTANCE_NAME, COST);

# Let GNUPLOT choose the plot color for each route
set colorsequence classic

# Single line plot command
plot SOLUTION_FILE i 1 every ::0::1 u 2:3:(0.10) w circles linecolor rgb "black" lw 2 fill solid border lc rgb "black" title sprintf("Depot"), SOLUTION_FILE i 1 every ::1::(REQUESTS + 1) u 2:3:(0.10) w circles linecolor rgb "blue" lw 2 fill solid border lc rgb "blue" title sprintf("Pickup"), SOLUTION_FILE i 1 every ::(REQUESTS + 1)::(REQUESTS * 2 + 1) u 2:3:(0.10) w circles linecolor rgb "red" lw 2 fill solid border lc rgb "red" title sprintf("Delivery"), SOLUTION_FILE i 1 every ::(REQUESTS * 2 + 1)::(REQUESTS * 2 + STATIONS + 1) u 2:3:(0.10) w circles linecolor rgb "green" lw 2 fill solid border lc rgb "green" title sprintf("Station"), SOLUTION_FILE i 1 u 2:3:1 w labels font "2,0" offset char 1.0,1.0 notitle, for [r = 1 : ROUTES] SOLUTION_FILE i (r + 1) u 1:2:3:4 w vectors lw 1.5 lt r title sprintf("Route %g", r),
