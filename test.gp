set term pngcairo
set output "teste.png"

set title 'BP and Heartrate'
set yrange [50:160]
set label 'finished walk' at 15, 140
unset label
set label 'finished walk' at 15, 105
set colorsequence classic
plot 'test.dat' u 1:2 w lp t 'systolic', 'test.dat' u 1:3 w lp t 'diastolic', 'test.dat' u 1:4 w lp t 'heartrate'

set output
