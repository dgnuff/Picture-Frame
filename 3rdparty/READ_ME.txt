.gitignore is set to ignore library files, but we need these third party libraries to build.  So they're placed in a zip file, and will need to be extracted here before building.


The correct setup is for all the headers and libraries from the zip file to be in this folder.  The zip file will remain here since it's in the repo, it can be otherwise ignored.