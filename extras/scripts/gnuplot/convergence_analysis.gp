# Deletes any user-defined variables and functions and restores default settings
reset session

# Produce PNG output. The 'enhanced' keyword adds support for additional text formatting
set term pngcairo dashed size 600, 350 enhanced font "Verdana, 12"

# Where the image will be stored
set output ARG2

set grid
set logscale x 2

set ytics nomirror
set xtics nomirror

set format x "%.2f"
set xtics 4

set xlabel "Time (minutes)"
set ylabel "Objective"

stats ARG1 nooutput

plot ARG1 i 0 u ($1/60.0):2 w l dt 4 lc rgb "#50ff0000" notitle, \
     ARG1 i 1 u ($1/60.0):2 w l lc rgb "#ff0000" lw 3 t "GRASP", \
     ARG1 i 2 u ($1/60.0):2 w l dt 4 lc rgb "#50ff0000"  notitle, \
     ARG1 i 3 u ($1/60.0):2 w l dt 4 lc rgb "#50ff0000" notitle, \
     ARG1 i 4 u ($1/60.0):2 w l dt 2 lc rgb "#5000008B" notitle, \
     ARG1 i 5 u ($1/60.0):2 w l dt 2 lc rgb "#5000008B" notitle, \
     ARG1 i 6 u ($1/60.0):2 w l lc rgb "#00008B" lw 3 t "ILS", \
     ARG1 i 7 u ($1/60.0):2 w l dt 2 lc rgb "#5000008B" notitle, \
