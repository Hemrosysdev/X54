set title "Long-term-test E54 / temperature log ".strftime("%d/%m/%Y", time(0))

set ylabel "Temp. [C]" 
set yrange [0:110]

set xlabel "Time"
set xdata time
set timefmt "%d.%m.%Y %H:%M:%S"
set format x "%H:%M"
set xtics rotate by 90 offset 0,-2.5 out nomirror

set datafile separator ";"
messwerte="< tail -n 500 Logs/TempLogTc08-".strftime("%Y%m%d", time(0)).".csv"

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

set terminal X11 0 size 800, 600 noraise
plot for [i=1:8] messwerte using 1:(column(1 + i)) title Titel[i] with lines dt i

pause 1
reread
