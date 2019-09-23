set multiplot
set size 1, 0.3

set title "Title goes here" offset 0,2 textcolor "#FFFFFF"

set key right bottom textcolor "#FFFFFF"

set grid xtics ytics \
    linetype 3 linewidth 0 linecolor rgb "#3366AA", \
    linetype 3 linewidth 0 linecolor rgb "#3366AA"

set border linecolor rgb "#3366AA"
set ticslevel 0

set origin 0.0,0.65
plot "data.csv" using 1:2 axis x1y1 title "tps"       with linespoints linewidth 0 linecolor rgb "#C466FF"

set origin 0.0,0.35
plot "data.csv" using 1:3 axis x1y2 title "cpu load"  with linespoints linewidth 0 linecolor rgb "#FF8266"

set origin 0.0,0.05
plot "data.csv" using 1:4 axis x1y2 title "ram usage" with linespoints linewidth 0 linecolor rgb "#EEFF66"
