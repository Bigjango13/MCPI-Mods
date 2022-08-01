cd .
mkdir -p build
cd build
cmake ..
make
mkdir -p ../out
cp lib*.so ../out

cd ..
for i in week*/install.sh; do
	. "$i"
done; unset i

echo "Done! Now just move the mods in out/ to ~/minecraft-pi/mods"