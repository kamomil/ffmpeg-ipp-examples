printf "CC\t%s\n" ffmpeg_decode_raw_h264.o;
gcc -I. -I./ -D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -DZLIB_CONST -std=c11 -fomit-frame-pointer -pthread -D_REENTRANT -I/usr/include/SDL2  -g -Wdeclaration-after-statement -Wall -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes -Wempty-body -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wno-pointer-sign -O3 -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -Werror=format-security -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -Werror=vla -Wformat -fdiagnostics-color=auto -Wno-maybe-uninitialized  -D_REENTRANT -I/usr/include/SDL2 -MMD -MF ffmpeg_decode_raw_h264.d -MT ffmpeg_decode_raw_h264.o -c -o ffmpeg_decode_raw_h264.o ffmpeg_decode_raw_h264.c
printf "LD\t%s\n" ffmpeg_decode_raw_h264_g;
gcc -Llibavcodec -Llibavdevice -Llibavfilter -Llibavformat -Llibavresample -Llibavutil -Llibpostproc -Llibswscale -Llibswresample -Wl,--as-needed -Wl,-z,noexecstack -Wl,--warn-common -Wl,-rpath-link=libpostproc:libswresample:libswscale:libavfilter:libavdevice:libavformat:libavcodec:libavutil:libavresample   -o ffmpeg_decode_raw_h264_g ffmpeg_decode_raw_h264.o  -lavdevice -lavfilter -lavformat -lavcodec -lpostproc -lswresample -lswscale -lavutil -lXv -lX11 -lXext -lSDL2 -ldl -ldl -lSDL2 -lvdpau -lX11 -lva -lva-x11 -lX11 -lva -lva-drm -lva -lxcb -lxcb-shm -lxcb -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lxcb-shape -lxcb -lsndio -lasound -lSDL2 -lm -lz -pthread -pthread  -lSDL2
printf "CP\t%s\n" ffmpeg_decode_raw_h264; cp -p ffmpeg_decode_raw_h264_g ffmpeg_decode_raw_h264
printf "STRIP\t%s\n" ffmpeg_decode_raw_h264; strip ffmpeg_decode_raw_h264
