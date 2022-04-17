#!/bin/bash                                                                                                
                                                                                                
declare -A fisiere                                                                                                
fisiere=( ["benchmarks/applu.ss"]=" < benchmarks/applu.in" ["benchmarks/cc1.ss"]=" < benchmarks/1stmt.i" ["benchmarks/hydro2d.ss"]="< benchmarks/hydro2d.in" ["benchmarks/li.ss"]="benchmarks/*.lsp {uses benchmarks/au.lsp, benchmarks/deriv.lsp, benchmarks/destrum0.lsp, benchmarks/tak0.lsp, benchmarks/xit.lsp, benchmarks/boyer.lsp, benchmarks/destru0.lsp, benchmarks/browse.lsp, benchmarks/div2.lsp, benchmarks/ctak.lsp, benchmarks/puzzle0.lsp, benchmarks/takr.lsp, benchmarks/dderiv.lsp, benchmarks/triang.lsp}" ["benchmarks/su2cor.ss"]=" < benchmarks/su2cor.in" ["benchmarks/swim.ss"]=" < benchmarks/swim2.in" ["benchmarks/tomcatv.ss"]=" < benchmarks/tomcatv.in" )                                                                                                
                                           
                                                 
function ruleaza()                                                                                                
{      
set -x                                                               
        echo zz >ceva.csv                                             
        for fisier in "${!fisiere[@]}";                                                                    
        do
            for percep in 2 4 6 8 10 15 20 25 30 35 40 45 50 55 60 70 80 90 100
            do
                ./sim-perceptron -redir:sim results/dataout.res -max:inst 10000000 -threshold 29 -percep $percep -pattern 8 -vht 4096 $fisier ${fisiere[$fisier]}          
                echo ./sim-perceptron -redir:sim results/dataout.res -max:inst 10000000 -threshold 29 -percep $percep -pattern 8 -vht 4096 $fisier ${fisiere[$fisier]}   
                    ((nrWrong=$(awk '/^nrWrong/ {print $2}' results/dataout.res)))
                    ((nrCorrect=$(awk '/^nrCorrect / {print $2}' results/dataout.res)))  
                    ((nrCorrectThresh=$(awk '/^nrCorrectThresh/ {print $2}' results/dataout.res))) 
                    ((sim_num_loads=$(awk '/^sim_num_loads/ {print $2}' results/dataout.res)))
                echo pattern $percep, $fisier,  rez, $(echo "scale=4; $nrCorrect/$sim_num_loads"|bc), $nrCorrect, $nrCorrectThresh, $sim_num_loads >>ceva.csv

            done                                                                                
        done    
        set +x                                                                                    
}                                                                                                
ruleaza ""
