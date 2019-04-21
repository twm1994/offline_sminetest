# Simplified Minetest, Offline Version

This project is based on the [original repository](https://github.com/celeron55/minetest_nmpr), built on OMNeT++ 5.4.1 with MinGW GCC toolchain.

This offline version is for testing map loading and saving, and keeps the interface for adding networking componets.

## Dependencies and tools

 - [JThread 1.3.3](http://research.edm.uhasselt.be/jori/page/CS/Jthread.html) (Threading)
 - [Irrlicht-1.8.4](http://irrlicht.sourceforge.net/) (3D engine)
 - [JsonCpp](https://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html) (Save and load map)
 - [CMake](https://cmake.org) (Library building tool)
 - [MinGW](http://www.mingw.org/) / [GNU Make](https://www.gnu.org/software/make/) (Library building tool)
 - [Code::Block](http://www.codeblocks.org/) (Library building tool)

## Change log

These changes are necessary to fix build errors
 - In **loadstatus.h**: add `s{w,n}printf` definition

## How to build

 - Download JThread, Irrlicht and JsonCpp source file
 - Use CMake to build JThread makefile
 - Run the makefile (Use msys on MinGW or GUN Make)
 - Use Code::Block to build Irrlicht win32-gcc version .a and .dll files
 - Follow [the guide](https://github.com/open-source-parsers/jsoncpp/wiki/Amalgamated) to build JsonCpp amalgamated source and add it to the project
 - Add **Irrlicht.dll** and **libjthread.dll** to the output folder that contains the .exe file after building the project

## Note

## License

### To be updated
