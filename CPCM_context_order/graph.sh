#!/bin/bash
#
file=${1}"_processed";
pdfFile=${1}".pdf";
# set zrange [1.58:1.72]
# set cbrange [1.58:1.72]
gnuplot << EOF
    reset
    set terminal pdfcairo enhanced color font 'Verdana,8'
    set size 0.98,1
    set output "${pdfFile}"
    set boxwidth 1
    set size ratio 0.6
    
    set style line 101 lc rgb '#000000' lt 1 lw 3
    set key outside horiz center top
    set tics nomirror out scale 0.75
    set xrange [0:12]
    set yrange [16:1]
    set zrange [:]
    set cbrange [:]

    set grid xtics lt -1 
    set grid ytics lt -1
    set grid ztics lt -1
    set border 3 front ls 101    
    set view 50,60
    

    set xyplane 0
    set xlabel "Mode" rotate parallel font "Verdana-Bold,10" offset 0,-1,0
    set ylabel "Context Order" rotate parallel font "Verdana-Bold,10" offset 0
    set zlabel "BPS" rotate parallel font "Verdana-Bold,10" offset -2

    set xtics font "Verdana-Bold,10" offset 0,-1,0
    set ztics font "Verdana-Bold,10" offset -1
    set ytics font "Verdana-Bold,9" offset -1
    set cbtics font "Verdana-Bold,10" offset -1

    set datafile separator "\t"
    set title noenhanced
    set title "${1}" font "Verdana-Bold,12"
    set palette rgb 7,5,15; 
    splot "${file}" using 1:2:3 with pm3d notitle 
EOF
