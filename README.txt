Source files:
    - hw1.cpp
    - initshader.cpp
    - fshader.glsl
    - vshader.glsl
    - MakeFile
    / main_files
        - Angel.h
        - CheckError.h
        - mat.h
        - vec.h

To compile and run:
    - make clean
    - make

Calistirmak icin:
    hw1.cpp ve initshader.cpp'deki
    #include "/home/omer/Desktop/421_code/hw1/main_files/Angel.h"
    Bu ilk satirdaki path için main_files'taki Angel.h'in path'ı eklenmelidir.
    Relative path yazmaya çalışınca olmadı.

Tested on Linux.

GLFW is used.

Renklendirme ve çarpışma kısmını yapamadım.