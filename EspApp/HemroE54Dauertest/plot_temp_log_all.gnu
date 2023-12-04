set title "Long-term-test E54 / temperature log"

set ylabel "Temp. [C]" 
set yrange [10:70]

set xlabel "Time"
set xdata time
set timefmt "%d.%m.%Y %H:%M:%S"
set format x "%H:%M"
set xtics rotate by 90 offset 0,-2.5 out nomirror
set xrange [ "02.02.2021 15:00:00" : "02.02.2021 20:00:00" ]

set datafile separator ";"
messwerte="< cat TempLogTc08-20210202.csv"

array Titel[8]
Titel[1]="MF1"
Titel[2]="BR1"
Titel[3]="MF2"
Titel[4]="BR2"
Titel[5]="MF3"
Titel[6]="BR3"
Titel[7]="MF4"
Titel[8]="BR4"

set key outside

#set terminal X11 0 size 800, 600 noraise

set terminal png size 800,600
set output 'E54_1_2_20210202.png'
plot for [i=1:4] messwerte using 1:(column(1 + i)) title Titel[i] with lines dt i

#pause 1
#reread
