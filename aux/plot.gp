set key right bottom textcolor "#FFFFFF"
set y2tics
set ytics nomirror

set title   "Title goes here" textcolor "#FFFFFF"
set xlabel  "time(sec)"                      offset { 0,0} textcolor "#FFFFFF"
set ylabel  "tps"                            offset {-1,0} textcolor "#FFFFFF"
set y2label "cpu load/ram usage coefficient" offset { 1,0} textcolor "#FFFFFF"

set object rect from screen 0, screen 0 to screen 1, \
    screen 1 fillcolor rgb "#000000" fillstyle solid 0.9 behind

set grid xtics ytics \
    linetype 3 linewidth 0 linecolor rgb "#3366AA", \
    linetype 3 linewidth 0 linecolor rgb "#3366AA"

set border linecolor rgb "#3366AA"
set ticslevel 0

plot "data.csv" using 1:2 axis x1y1 title "tps"       with linespoints linewidth 0 linecolor rgb "#C466FF", \
     "data.csv" using 1:3 axis x1y2 title "cpu load"  with linespoints linewidth 0 linecolor rgb "#FF8266", \
     "data.csv" using 1:4 axis x1y2 title "ram usage" with linespoints linewidth 0 linecolor rgb "#EEFF66"
