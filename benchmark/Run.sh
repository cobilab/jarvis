#!/bin/bash
#
rm -fr jarvis
git clone https://github.com/pratas/jarvis.git
cd jarvis/src/
make
cp JARVIS ../../
cd ../../
#
wget http://sweet.ua.pt/pratas/datasets/DNACorpus.zip
unzip DNACorpus.zip
mv DNACorpus/* .
#
#
cat AeCa AgPh AnCa BuEb DaRe DrMe EnIn EsCo GaGa HaHi HePy HoSa OrSa PlFa ScPo WaMe YeMi > ALL;
#
#
for (( x = 1 ; x < 10 ; ++x ));
  do
  (./JARVIS -v -l $x AeCa ) &> REPORT_JARVIS_AeCa_$x;
  (./JARVIS -v -l $x AgPh ) &> REPORT_JARVIS_AgPh_$x;
  (./JARVIS -v -l $x AnCa ) &> REPORT_JARVIS_AnCa_$x;
  (./JARVIS -v -l $x BuEb ) &> REPORT_JARVIS_BuEb_$x;
  (./JARVIS -v -l $x DaRe ) &> REPORT_JARVIS_DaRe_$x;
  (./JARVIS -v -l $x DrMe ) &> REPORT_JARVIS_DrMe_$x;
  (./JARVIS -v -l $x EnIn ) &> REPORT_JARVIS_EnIn_$x;
  (./JARVIS -v -l $x EsCo ) &> REPORT_JARVIS_EsCo_$x;
  (./JARVIS -v -l $x GaGa ) &> REPORT_JARVIS_GaGa_$x;
  (./JARVIS -v -l $x HaHi ) &> REPORT_JARVIS_HaHi_$x;
  (./JARVIS -v -l $x HePy ) &> REPORT_JARVIS_HePy_$x;
  (./JARVIS -v -l $x HoSa ) &> REPORT_JARVIS_HoSa_$x;
  (./JARVIS -v -l $x OrSa ) &> REPORT_JARVIS_OrSa_$x;
  (./JARVIS -v -l $x PlFa ) &> REPORT_JARVIS_PlFa_$x;
  (./JARVIS -v -l $x ScPo ) &> REPORT_JARVIS_ScPo_$x;
  (./JARVIS -v -l $x WaMe ) &> REPORT_JARVIS_WaMe_$x;
  (./JARVIS -v -l $x YeMi ) &> REPORT_JARVIS_YeMi_$x;
  #
  (./JARVIS -v -l $x ALL ) &> REPORT_JARVIS_ALL_$x;
  done
#  
