# Unpacker
This program is a mini portable installation application. It doesn't modify the registry, nor generates any further applications.
Its goal is to be a quick means of dumping some package into the directory it runs from.

It doesn't use any fancy rendering library, it instead runs in a terminal and requires no user input.

This application has 2 (two) resources embedded within it:
  - IDI_ICON1		the application icon
  - IDR_ARCHIVE		the package to be installed    

The raw version of this application is useless on its own, and is intended to be fullfilled by nSuite using the `-packager` command.
The following is how nSuite uses this application:
  - writes out a copy of this application to disk
  - packages a directory, embedding the package resource into **this application's** IDR_ARCHIVE resource