# Best In Class PP SSE

## Description
Best in class is an excellent mod that expands on the default behavior of Skyrim's "Best In Class" system. When you open your inventory, a diamond is appended to what the game thinks is the best current weapon/armor in your inventory. This plugin expands on that system, making it so that it only suggests better alternatives to what you currently have equipped. So if you have a one handed weapon, it will suggest one handed weapons. If you have heavy boots but light cuirasses, it only suggests heavy boots and light cuirasses. This initially started as an attempt to port Dropkicker's/Netrve's excellent mod, but it has since ballooned.

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++

## Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

## Building
```
git clone https://github.com/SeaSparrowOG/BestInClassPPSSE
cd BestInClassPPSSE
cmake --preset vs2022-windows
cmake --build build --config Release
```
