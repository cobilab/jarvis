#!/bin/bash
#

function process {
    ttt=$1;
    
    for y in {1..12}
    do
        for i in {1..16}
        do
            echo  $i >> dois;
            echo  $y >> um;
        done
        printf "\n" >> dois;
        printf "\n"  >> um;
    done


    cat $ttt | awk '{print $3}'> b; 
    printf "\n" >> b;
    cat $ttt | awk '{print $4}'> c; 
    printf "\n" >>c;
    cat $ttt | awk '{print $5}'> d;
    printf "\n" >>d;
    cat $ttt | awk '{print $6}'> e;
    printf "\n" >>e
    cat $ttt | awk '{print $7}'> f;
    printf "\n" >>f
    cat $ttt | awk '{print $8}'> g;
    printf "\n" >>g;
    cat $ttt | awk '{print $9}'> h;
    printf "\n" >> h;
    cat $ttt | awk '{print $10}'> i;
    printf "\n" >>i;
    cat $ttt | awk '{print $11}'> j;
    printf "\n" >>j;
    cat $ttt | awk '{print $12}'> k;
    printf "\n" >>k;
    cat $ttt | awk '{print $13}'> l;
    printf "\n" >>l;
    cat $ttt | awk '{print $14}'> m;
    cat b c d e f g h i j k l m > Files;
    paste um dois Files > ${ttt}_processed;
    rm  b c d e f g h i j k l m;
    rm um dois Files;
}

    # process "AeCa"
    # process "BuEb" 
    # process "EnIn" 
    # process "HePy" 
    # process "OrSa" 
    # process "YeMi" 
    # process "AgPh" 
    # process "DaRe" 
    # process "DrMe" 
    # process "EsCo" 
    # process "GaGa" 
    # process "HaHi" 
    # process "HoSa" 
    # process "PlFa" 
    # process "ScPo" 

    bash graph.sh "AeCa"
    bash graph.sh "BuEb" 
    bash graph.sh "EnIn" 
    bash graph.sh "HePy" 
    bash graph.sh "OrSa" 
    bash graph.sh "YeMi" 
    bash graph.sh "AgPh" 
    bash graph.sh "DaRe" 
    bash graph.sh "DrMe" 
    bash graph.sh "EsCo" 
    bash graph.sh "GaGa" 
    bash graph.sh "HaHi" 
    bash graph.sh "HoSa" 
    bash graph.sh "PlFa" 
    bash graph.sh "ScPo" 
    



