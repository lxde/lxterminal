# LXTerminal

LXTerminal is a VTE-based terminal emulator with support for multiple tabs. 
It is completely desktop-independent and does not have any unnecessary 
dependencies. In order to reduce memory usage and increase the performance 
all instances of the terminal are sharing a single process.

- Add shortcut tab to preference
- Single instance setting for all windows
- Load menubar from menu.ui
- Extract shortcut from ui_manager after loading GtkActionEntry
- Smart copy - may set CTRL+C for copy - test selected symbol, if selected - copy, otherwise send key to terminal
- Update .po for Shortcut Prefernces tab, update ru.po localization.

##  Install

See INSTALL, and if you want to build LXTerminal, you may try next:

### Ubuntu

```
apt-get install autoconf docbook-xml docbook-xsl git intltool libgtk2.0-dev libvte-dev xsltproc
git clone https://github.com/lxde/lxterminal.git
cd lxterminal
apt-get build-dep lxterminal
./autogen.sh
./configure --prefix=/usr --enable-man
make && sudo make install
```
