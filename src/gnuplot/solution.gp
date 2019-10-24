# Deletes any user-defined variables and functions and restores default settings
reset session

# Produce PNG output. The 'enhanced' keyword supports additional text formatting
set term pngcairo size 1920, 1080 enhanced font "Verdana, 12"

# Name and location of the output file
set output "../tmp/gnuplot/solution.png"

# Variables to design where the points.dat and routes.dat files are located
POINTS = "../tmp/gnuplot/points.dat"
ROUTES = "../tmp/gnuplot/routes.dat"

stats POINTS u 0 nooutput
POINTS_COUNT = (STATS_records - 3 - 1)/2
print POINTS_COUNT

set title "Instance: a2-16"
set xrange[-10.5:10.5]
set yrange[-10.5:10.5]

# load routes file into datablock
set datafile separator "\n"
set table $Routes
    plot ROUTES u 1 with table
unset table

# loop routes
set datafile separator whitespace
stats $Routes u 0 nooutput  # get the number of routes
RoutesCount = STATS_records-1
set print $RoutesData
do for [i=1:RoutesCount] {
    # get the points of a single route
    set datafile separator "\n"
    set table $Dummy
       plot ROUTES u (SingleRoute = stringcolumn(1),$1) every ::i::i with table
    unset table
    # create a table of the coordinates of the points of a single route
    set datafile separator whitespace
    do for [j=1:words(SingleRoute)] {
        set table $Dummy2
            plot POINTS u (a=$2,$2):(b=$3,$3) every ::word(SingleRoute,j)::word(SingleRoute,j) with table
            print sprintf("%g %s %g %g", j, word(SingleRoute,j), a, b)
        unset table
    }
    print "" # add empty line
}

set colorsequence classic

plot POINTS every ::0::1 u 2:3:(0.10) w circles linecolor rgb "black" lw 2 fill solid border lc rgb "black" notitle, POINTS every ::1::(POINTS_COUNT + 1) u 2:3:(0.10) w circles linecolor rgb "blue" lw 2 fill solid border lc rgb "blue" notitle, POINTS every ::(POINTS_COUNT + 1)::(POINTS_COUNT * 2 + 1) u 2:3:(0.10) w circles linecolor rgb "red" lw 2 fill solid border lc rgb "red" notitle, POINTS every ::(POINTS_COUNT * 2 + 1)::(POINTS_COUNT * 2 + 4) u 2:3:(0.10) w circles linecolor rgb "black" lw 2 fill solid border lc rgb "black" notitle, POINTS u 2:3:1 w labels font "2,0" offset char 1.0,1.0 notitle, for [i=1:RoutesCount] $RoutesData u 3:4 every :::i-1::i-1 w lines lt i title sprintf("Route %g",i)
