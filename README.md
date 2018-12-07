[![Build Status](https://travis-ci.org/pratas/jarvis.svg?branch=master)](https://travis-ci.org/pratas/jarvis)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/4fd7f444dc624bdcb7b83af0273dd329)](https://www.codacy.com/app/pratas/jarvis?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=pratas/jarvis&amp;utm_campaign=Badge_Grade)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](LICENSE)
<p align="center"><img src="imgs/logo.png" 
alt="JARVIS" width="300" border="0" /></p>
<p align="center"><b>Extreme lossless compression of genomic sequences</b></p>

As a compression tool, JARVIS is able to provide additional compression gains over GeCo [ https://github.com/pratas/geco ] and GeCo2 [ https://github.com/pratas/geco2 ], however it uses substantially higher computational resources (RAM and processing time). JARVIS only affords reference-free compression. The core of the JARVIS method is a competitive prediction between two different classes of models: repeat models and context models. The latter models can be either context models or substitutional tolerant context models. 

## INSTALLATION ##

<pre>
git clone https://github.com/pratas/jarvis.git
cd jarvis/src/
make
</pre>

## EXECUTION

### Run JARVIS

Run JARVIS using level 3:

<pre>
./JARVIS -v -l 3 File.seq
</pre>

## PARAMETERS

To see the possible options type
<pre>
./JARVIS
</pre>
or
<pre>
./JARVIS -h
</pre>

This will print the following options:
```
SYNOPSIS                                                               
      ./JARVIS [OPTION]... [FILE]                                      
                                                                       
SAMPLE                                                                 
      Run Compression         :  ./JARVIS -v -l 4 sequence.txt         
      Run Decompression       :  ./JARVIS -v -d sequence.txt.jc        
                                                                       
DESCRIPTION                                                            
      Compress and decompress lossless genomic sequences for           
      storage and analysis purposes.                                   
      Measure an upper bound of the sequence entropy.                  
                                                                       
      -h,  --help                                                      
           usage guide (help menu).                                    
                                                                       
      -V,  --version                                                   
           Display program and version information.                    
                                                                       
      -F,  --force                                                     
           force mode. Overwrites old files.                           
                                                                       
      -v,  --verbose                                                   
           verbose mode (more information).                            
                                                                       
      -s,  --show-levels                                               
           show pre-computed compression levels (configured).          
                                                                       
      -l [NUMBER],  --level [NUMBER]                                   
           Compression level (integer).                                
           Default level: 1.                                          
           It defines compressibility in balance with computational    
           resources (RAM & time). Use -s for levels perception.       
                                                                   
      -cm [NB_C]:[NB_D]:[NB_I]:[NB_G]/[NB_S]:[NB_E]:[NB_I]:[NB_A]  
      Template of a context model.                                 
      Parameters:                                                  
      [NB_C]: (integer [1;20]) order size of the regular context   
              model. Higher values use more RAM but, usually, are  
              related to a better compression score.               
      [NB_D]: (integer [1;5000]) denominator to build alpha, which 
              is a parameter estimator. Alpha is given by 1/[NB_D].
              Higher values are usually used with higher [NB_C],   
              and related to confiant bets. When [NB_D] is one,    
              the probabilities assume a Laplacian distribution.   
      [NB_I]: (integer {0,1}) number to define if a sub-program    
              which addresses the specific properties of DNA       
              sequences (Inverted repeats) is used or not. The     
              number 1 turns ON the sub-program using at the same  
              time the regular context model. The number 0 does    
              not contemple its use (Inverted repeats OFF). The    
              use of this sub-program increases the necessary time 
              to compress but it does not affect the RAM.          
      [NB_G]: (real [0;1)) real number to define gamma. This value 
              represents the decayment forgetting factor of the    
              regular context model in definition.                 
      [NB_S]: (integer [0;20]) maximum number of editions allowed  
              to use a substitutional tolerant model with the same 
              memory model of the regular context model with       
              order size equal to [NB_C]. The value 0 stands for   
              turning the tolerant context model off. When the     
              model is on, it pauses when the number of editions   
              is higher that [NB_C], while it is turned on when    
              a complete match of size [NB_C] is seen again. This  
              is probabilistic-algorithmic model very usefull to   
              handle the high substitutional nature of genomic     
              sequences. When [NB_S] > 0, the compressor used more 
              processing time, but uses the same RAM and, usually, 
              achieves a substantial higher compression ratio. The 
              impact of this model is usually only noticed for     
              [NB_C] >= 14.                                        
      [NB_R]: (integer {0,1}) number to define if a sub-program    
              which addresses the specific properties of DNA       
              sequences (Inverted repeats) is used or not. It is   
              similar to the [NR_I] but for tolerant models.       
      [NB_E]: (integer [1;5000]) denominator to build alpha for    
              substitutional tolerant context model. It is         
              analogous to [NB_D], however to be only used in the  
              probabilistic model for computing the statistics of  
              the substitutional tolerant context model.           
      [NB_A]: (real [0;1)) real number to define gamma. This value 
              represents the decayment forgetting factor of the    
              substitutional tolerant context model in definition. 
              Its definition and use is analogus to [NB_G].        
                                                                   
      ... (you may use several context models)                     
                                                                   
                                                                   
      -rm [NB_R]:[NB_C]:[NB_A]:[NB_B]:[NB_L]:[NB_G]:[NB_I]         
      Template of a repeat model.                                  
      Parameters:                                                  
      [NB_R]: (integer [1;10000] maximum number of repeat models   
              for the class. On very repetive sequences the RAM    
              increases along with this value, however it also     
              improves the compression capability.                 
      [NB_C]: (integer [1;20]) order size of the repeat context    
              model. Higher values use more RAM but, usually, are  
              related to a better compression score.               
      [NB_A]: (real (0;1]) alpha is a real value, which is a       
              parameter estimator. Higher values are usually used  
              in lower [NB_C]. When [NB_A] is one, the             
              probabilities assume a Laplacian distribution.       
      [NB_B]: (real (0;1]) beta is a real value, which is a        
              parameter for discarding or maintaining a certain    
              repeat model.                                        
      [NB_L]: (integer (1;20]) a limit threshold to play with      
              [NB_B]. It accepts or not a certain repeat model.    
      [NB_G]: (real [0;1)) real number to define gamma. This value 
              represents the decayment forgetting factor of the    
              regular context model in definition.                 
      [NB_I]: (integer {0,1}) number to define if a sub-program    
              which addresses the specific properties of DNA       
              sequences (Inverted repeats) is used or not. The     
              number 1 turns ON the sub-program using at the same  
              time the regular context model. The number 0 does    
              not contemple its use (Inverted repeats OFF). The    
              use of this sub-program increases the necessary time 
              to compress but it does not affect the RAM.          
                                                                   
      -z [NUMBER],  --selection [NUMBER]                               
           Size of the context selection model (integer).              
           Default context selection: 12.                              
                                                                       
      [FILE]                                                           
           Input sequence filename (to compress) -- MANDATORY.         
           File to compress is the last argument.     
```


If you are not interested in setting the template for each model, then use the levels mode. To see the possible levels type:
<pre>
./JARVIS -s
</pre>

## CITATION ##

On using this software/method please cite:

D. Pratas, A. J. Pinho, P. J. S. G. Ferreira. Efficient compression of genomic sequences. Proc. of the Data Compression Conference, DCC-2016, Snowbird, UT, p. 231-240, March 2016.

## ISSUES ##

For any issue let us know at [issues link](https://github.com/pratas/jarvis/issues).

## LICENSE ##

GPL v3.

For more information:
<pre>http://www.gnu.org/licenses/gpl-3.0.html</pre>

                                                    

