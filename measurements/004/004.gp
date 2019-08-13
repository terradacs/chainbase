unset key

set title  "#004 | 2,500,000/2,500,000 | RocksDB Release Build | Chainbase Release Build" textcolor "#FFFFFF"
set xlabel "time(sec)"                         offset {0,-1} textcolor "#FFFFFF"
set ylabel "tps"                               offset {2, 1} textcolor "#FFFFFF"
set zlabel "cpu load/\nram usage\ncoefficient" offset -3,2,0 textcolor "#FFFFFF"

set object rect from screen 0, screen 0 to screen 1, \
    screen 1 fillcolor rgb "#000000" fillstyle solid 0.9 behind

set grid xtics ytics ztics mytics \
    linetype 3 linewidth 0 linecolor rgb "#3366AA", \
    linetype 3 linewidth 0 linecolor rgb "#3366AA"

set border linecolor rgb "#3366AA"
set ticslevel 0

splot "data.csv" using 1:2:4 title ""          with impulses linewidth 0 linecolor rgb "#C466FF", \
      "data.csv" using 1:2:4 title "ram usage" with lines    linewidth 0 linecolor rgb "#C466FF", \
      "data.csv" using 1:2:3 title ""          with impulses linewidth 0 linecolor rgb "#EEFF66", \
      "data.csv" using 1:2:3 title "cpu load"  with lines    linewidth 0 linecolor rgb "#FFFFFF"
