# A script to make installing the jetpack mod simpler.
set -e
mkdir -p /tmp/mcpi-jetpack
cd /tmp/mcpi-jetpack

# libjetpack.so
wget https://github.com/Bigjango13/MCPI-Mods/releases/download/v1.0.0/mods.zip
unzip mods.zip
mv out/libjetpack.so ~/.minecraft-pi/mods

# jetpack_1.png
mkdir -p ~/.minecraft-pi/overrides/images/armor
wget https://raw.githubusercontent.com/Bigjango13/MCPI-Mods/master/mcpi-competition/week-1/images/jetpack_1.png -O ~/.minecraft-pi/overrides/images/armor/jetpack_1.png

# items.png
mkdir -p ~/.minecraft-pi/overrides/images/gui
wget https://raw.githubusercontent.com/Bigjango13/MCPI-Mods/master/mcpi-competition/week-1/images/items.png -O ~/.minecraft-pi/overrides/images/gui/items.png

# en_US.lang
mkdir -p ~/.minecraft-pi/overrides/lang/
wget https://raw.githubusercontent.com/Bigjango13/MCPI-Mods/master/mcpi-competition/week-1/images/en_US.lang -O ~/.minecraft-pi/overrides/lang/en_US.lang

rm -rf /tmp/mcpi-jetpack