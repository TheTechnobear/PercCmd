#/bin/sh
echo XMX RELEASE : copy resourcs and build.xmx/PercCmd to release/xmx

cp resources/S80perccmd releases/xmx
cp resources/perccmd.json releases/xmx
cp resources/perccmd.sh.xmx releases/xmx/perccmd.sh
cp build.xmx/PercCmd releases/xmx
/opt/homebrew/bin/aarch64-elf-strip --strip-unneeded releases/xmx/PercCmd
