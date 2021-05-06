
1) First install in your system:
build-essential, libglfw3-dev, libglfw3

2) Make sure the latest graphics drivers are installed.

Check your video card:
    lspci | grep VGA
    sudo lshw -C video

To find and install latest nvidia driver:
    apt-cache search nvidia-driver
    sudo apt install nvidia-driver-435 nvidia-opencl-dev

To check OpenGl version:
    glxinfo | grep "OpenGL version"
    SIG will not run with Mesa, it needs access to OpenGL 4

3) Enter "make" from this directory to build libraries and examples.

To force compilation in 32 bits type 'make ARCH=32'

makefile commands:
all :  build libs and executables (default cmd)
libs :  build only libs
clean : clean all temporary files
cleanexe : clean executables and their temporary files
cleanall : clean libraries, executables and temporary files

For developing your own application, start from the project in
../examples/sigapp.7z, which is prepared to be compiled outside
of the SIG distribution folder.

