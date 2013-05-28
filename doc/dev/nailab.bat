rem Her settes sample info
pars det:DET01 /stitle="Test prosekt" /scollname="jld" /sdesc1="sdesc1" /sdesc4="Det01spekterref" /sident="LIMSid" /stype="Gress" /squant=10.0 /squanterr=1.0 /sunits="kg" /sgeomtry="L1" /builduptype="" /stime="1.4.2013 09:55:49"
rem Her setter vi advanced parametere
pars det:det01 /ssyserr=2 /ssysterr=4
rem Laste inn effektivitet
movedata c:\nailab\det01l1.cal det:det01 /effcal /overwrite
rem Starte detektoren
startmca det:det01 /livepreset=10 /intpreset=100,200,250
wait det:det01 /acq
rem preak locate
peak_dif det:det01 /channels=1,1024 /signif=3.00 /ftol=0.2
rem peak area
area_nl1 det:det01 /channels=1,1024 /fcont=0.2 /critlevel /fixfwhm /fixtail /fit /display_rois
pars det:det01 /roipsbtyp="step" /prreject0pks=1 /prfwhmpkmult=2.4 /prfwhmpkleft=1.27 /prfwhmpkrght=1.26
rem her trekkes bakrunnen fra
areacor det:det01 /bkgnd="c:\nailab\det01bkg.cnf"
rem effektivitetskorreksjon
effcor det:det01 /interp
rem nuklideidentifisering
nid_intf det:det01 /library="c:\nailab\nai.nlb" /mda_test /noacqdecay /confid=0.3
pars det:det01 /prusestrlib=0 /mdaconfid=5.10
rem rapport
report det:det01 /template="c:\nailab\lorakon.tpl" /newfile /firstpg /newpg /outfile="c:\nailab\spekter\det01spekter.rpt" /section="" /EM=2
rem kopiere spekteret til rett plass
movedata det:det01 c:\nailab\spekter\det01spekter.cnf /overwrite
