#!/usr/bin/env ruby
#
# This is a convenience script for bumping amaroK's plugin framework version
# in the various engine desktop files and in pluginmanager.h.
#
# The script should be run once before each release, in order to ensure that
# no old and perhaps incompatible engines are getting loaded. After running, don't
# forget to commit to svn. The script must be started from the amarok/ folder.
#
# (c) 2005 Mark Kretschmann <markey@web.de>
# License: GNU General Public License V2


# Bump FrameworkVersion in pluginmanager.h
file = File.new( "src/pluginmanager.h", File::RDWR )
str = file.read()
file.rewind()
temp = str.scan( /static const int FrameworkVersion = [0-9]*;/ )
version = temp.join().scan( /[0-9]*/ ).join().to_i()
version = version + 1

print "Bumping the plugin framework version to: #{version}"

str.sub!( /static const int FrameworkVersion = [0-9]*;/, "static const int FrameworkVersion = #{version};" )
file << str
file.close()


print "\n"
print "\n"
print "Done :) Now commit the source to SVN."
print "\n\n"
