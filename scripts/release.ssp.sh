#/bin/sh
echo SSP RELEASE : copy resourcs and build.ssp/PercCmd to release/ssp

cp resources/S80perccmd releases/ssp
cp resources/perccmd.json releases/ssp
cp resources/perccmd.sh.ssp releases/ssp/perccmd.sh
cp build.ssp/PercCmd releases/ssp
/opt/homebrew/bin/arm-linux-gnueabihf-strip --strip-unneeded releases/ssp/PercCmd
