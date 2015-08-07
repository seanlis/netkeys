## What You'll Need ##

You need the following in order to check out and build the source code:
  * An SVN client. [TortoiseSVN](http://tortoisesvn.tigris.org/) for Windows is a fantastic little client, and most Linux distributions will have a client available somewhere.
  * Visual Studio 2008 on Windows (no IDE required on Linux)
  * GNU Make, Autoconf, and Automake on Linux
  * X11 header files and libraries on Linux (as well as the XTest library)

## Process ##

Check out the source code (stable):
```
svn checkout http://netkeys.googlecode.com/svn/branches/stable netkeys
```

For the latest possible code (which may be buggy or have missing features):
```
svn checkout http://netkeys.googlecode.com/svn/trunk netkeys
```

If you are on Windows, open the "dev/netkeys.sln" file. You should be able to build and run netkeys with this solution.

If you are on Linux, you will need to run `autoconf'. The Linux build is very untested and needs a lot more work, but after the 'configure' script is generated, you should be able to run:
```
./configure
make
make install
```
After this, netkeys will be ready for use.