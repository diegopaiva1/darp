# Deletes any user-defined variables and functions and restores default settings
reset session

# Produce PNG output. The 'enhanced' keyword adds support for additional text formatting
set term pngcairo size 600, 350 enhanced font "Verdana, 12"

# Where the image will be stored
set output ARG3

set title sprintf("Target = %s", ARG2)

set grid
set key outside
set ytics nomirror
set xtics nomirror
set ytics 0.2

set xlabel "Time to sub-optimal (minutes)"
set ylabel "Probability"

plot ARG1 i 0 u ($2/60.0):(($1 - (1/2))/100) w l lw 3 t "GRASP", \
     ARG1 i 1 u ($2/60.0):(($1 - (1/2))/100) w l lw 3 t "ILS"
