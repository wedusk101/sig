
First make sure to have installed in your system:
build-essential, libglfw3-dev, libglfw3

Then call make from this directory to build libraries and examples.

To force compilation in 32 bits type 'make ARCH=32'

makefile commands:
clean : clean temporary files
cleanex : clean executables and temporary files
cleanall : clean libraries, executables and temporary files

For developing your own application, start from the project in
../examples/sigapp.7z, which is prepared to be compiled outside
of the SIG distribution folder.


