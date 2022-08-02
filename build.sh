set -e
mkdir -p build
rm -rf build/*

dir=".."
if [ -d "${1}" ];then
    dir="../${1}"
fi

cd build
cmake -S"${dir}" -B.
make
mkdir -p ../out
if [ "$dir" = ".." ];then
    cp */lib*.so ../out
else
    cp lib*.so ../out
fi

if [ "$1" = "zip" ];then
    cd ..
    zip mods.zip out/ -r
fi