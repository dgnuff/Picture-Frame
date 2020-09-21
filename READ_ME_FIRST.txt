This won't build as pulled directly from the repo.  The libraries in the 3rdparty folder will need to be unpacked first.

There's also a READ_ME.txt file in that folder, but the TL;DR is that you simply extract the .lib files from the .zip, and place them in the same folder as the headers that are already there, and the .zip file itself.

Other than that, this should build with the latest version of Visual Studio, community works fine.

sspf is the picture frame itself, the easiest way to drive it is to place the files you want on display in a filder, and then place all their names in a file, the convention I use is "list.txt"  Then simply pass that filename as a command line argument to sspf.exe.

