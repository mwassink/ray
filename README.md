# About
This is a toy ray tracer I wrote in ~1 day. It only traces spheres and planes. The demo is also baked into the source file.
# Build (Windows)
To build this, run the build.bat file, which requires Visual Studio 2019. If that does not work, you could make a build folder in the root of the directory and run the command:
cl -FC -Zi ..\ray.cc  user32.lib gdi32.lib. If that doesn't work you could also use the release on this GitHub.

# Build (Other)
I believe this will build on other machines. To do so, call the compiler appropriately (e.g g++ ray.cc -o ray). 
# Run
To run the demo, double click on the ray.exe executable once it is built. This will create a bitmap of the image.
