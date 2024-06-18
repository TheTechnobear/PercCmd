# PercCmd

a simple launcher for percussa which will launch an application depending on which button is pressed at boot
if no button is held, then application launched last time will be launched

perccmd will default to /media/BOOT/Synthor if anything fails.


# goals
- simple application 
- allow multiple application launches configured by user.
- fail safe if anything goes wrong (e.g. misconfiguration by user) to synthor
- allow for update in case future applications need some new functionality 


# installation
copy resources/S90perccmd to /etc/init.d/
copy resources/perccmd.sh to /root
copy resources/perccmd.json to /media/BOOT

PercCmd binary should be copied to /media/BOOT


note:  you should also disable synthor launch by moving /etc/init.d/S90synthor to D90Synthor.

# environment
currently we use a common 'environment' which is setup in /root/perccmd.sh


# synthor fail safe
PercCmd assumes Synthor uses the following  
binary : ./Synthor
working directiry : /media/BOOT

ofc, this can be overriden in the perccmd.json, but if all else fails this is what is used.
this is the hardcoded defaults for fail safe.



# possible future changes
### environment
currently we assume environment is setup in /root/perccmd.sh 
however, changing perccmd.sh needs a new image as its in /root
also it may be some apps need different environment var.
we could allow optional configuration in perccmd.json
(or perhaps we could just (optionally) source a shell script prior to launch.)


# open questions 
### perccmd.sh
should we move perccmd.sh -> /media/BOOT

