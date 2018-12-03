<p align="center"><img src="imgs/logo.png" 
alt="JARVIS" width="300" height="260" border="0" /></p>
<b>Extreme lossless compression of genomic sequences</b>. As a compression tool, JARVIS is able to provide additional compression gains over GeCo and GeCo2, however it uses substantially higher computational resources (RAM and processing time). JARVIS only affords reference-free compression.

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

                                                    

