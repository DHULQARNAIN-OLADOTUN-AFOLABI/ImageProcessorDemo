# ImageProcessorDemo
an image processor to demonstrate openmp

# To Run
Download Only ImageProcessing.exe and run.

# to Compile
Note: stb_image.h, stb_image_write.h must be present in the folder, if you didnt clone the entire repo
To compile you need gcc, preferably you should use msys2 (as it supports openmp natively) others gave issues.
# Steps for msys2
1. Download it from  https://www.msys2.org
2. After installation, it open a terminal automatically enter `pacman -Syu`
3. It will ask you to close the window midway — close it, then reopen MSYS2 MINGW64 from the Start menu and run: `pacman -Su`
4. After that runs in the same terminal enter `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-openmp`
5. Add this to your system PATH variable: `C:\msys64\mingw64\bin` or the directory where you installed it.
6. compile with `g++ -O2 -fopenmp -mwindows -o image_processor.exe image_processor.cpp -lcomdlg32 -lshell32`
   
