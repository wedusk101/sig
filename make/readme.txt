
Just call make from this directory to build libraries and examples.

To force compilation in 32 or 64 bits type 'make ARCH=32' or 'make ARCH=64'

makefile commands:
clean : clean temporary files
cleanex : clean executables and temporary files
cleanall : clean libraries, executables and temporary files

For developing your own application, start from the project in
../examples/sigapp.7z, which is prepared to be compiled outside
of the SIG distribution folder.

Linux installation:
libsig and libsigkin are graphics-independent and just require:

The other modules require the following packages:
X11 OpenGL and GLFW3
 
