# LXTerminal

LXTerminal is a VTE-based terminal emulator with support for multiple tabs.  It
is completely desktop-independent and does not have any unnecessary
dependencies. In order to reduce memory usage and increase the performance all
instances of the terminal are sharing a single process.

## Building and installation

### Dependencies

This dependency is listed as the package name used in Debian.  If your
distribution is neither Debian nor its derivatives, you could find equivalents
for your target distribution.

Basic requirements for building:
* libglib2.0-dev
* libgtk2.0-dev
* libvte-dev
* autotools-dev
* intltool

For generating man pages (`./configure --enable-man`):
* xsltproc
* docbool-xml
* docbook-xsl

### Building on Debian, Ubuntu or their derivatives from git

You may try:

```
# Install tools and build dependencies
sudo apt install git xsltproc docbook-xml docbook-xsl
sudo apt build-dep lxterminal

# Get the source code from git
git clone https://github.com/lxde/lxterminal.git
cd lxterminal

# Build and install
./autogen.sh
./configure --enable-man
make
sudo make install
```

Note that if you get the message `E: You must put some 'source' URIs in your
sources.list`, it means you need to add `deb-src` URI into
`/etc/apt/sources.list` to find the build dependencies.  Try duplicating the
URI of the main repository and replace `deb` with `deb-src`.
