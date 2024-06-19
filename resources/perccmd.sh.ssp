#!/bin/sh

echo 1 > /proc/irq/34/smp_affinity # sd card dw-mci 
echo 2 > /proc/irq/35/smp_affinity # audio sdio dw-mci 
echo 1 > /proc/irq/41/smp_affinity # usb
echo 1 > /proc/irq/42/smp_affinity # usb 
echo 1 > /proc/irq/43/smp_affinity # usb 
echo 4 > /proc/irq/58/smp_affinity # dma controller sdio
echo 4 > /proc/irq/59/smp_affinity # dma controller sdio 

# not used anymore at the moment
#echo 4 > /proc/irq/28/smp_affinity
###echo 8 > /proc/irq/31/smp_affinity
#echo 8 > /proc/irq/36/smp_affinity

#sudo /home/linaro/clean.sh 
#sudo /home/linaro/gadget.sh 
/root/gadget.sh 

#amixer sset 'PCM' 97%
# -4.72V .. +4.72V 
amixer sset 'PCM' 99%
# -5.2V .. +5.2V 
#amixer sset 'PCM' 100%

amixer sset 'High-Pass Filter' off

export LD_LIBRARY_PATH=.

cd /media/BOOT
tail -f ./PercCmd
