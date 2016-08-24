gcc -ldl -m64 -fPIC -shared `pkg-config --cflags --libs sdl2` -lGL -o apocalypse64.so gettime_speedhack.c info_window.c config.c
gcc -ldl -m32 -fPIC -shared `pkg-config --cflags --libs sdl2` -lGL -o apocalypse32.so gettime_speedhack.c info_window.c config.c
cp *.so ~/.preloads/
