# JavaScript plugin for OpenCPN

This plugin allows you to run JavaScript and to interact with OpenCPN.  You can use it to script your own enhancements to standard OpenCPN functionality.

The functionality in OpenCPN that can be scripted depends on the APIs built into the plugin.  These will be expanded as the need arises.

At this stage the plugin is an alpha release which has been built for MacOS only using Xcode.  It awaits a volunteer familiar with Xcode and Windows to update the Cmake files so it can be build for a non-Xcode environment.

To install on MacOS download the zip file *JavaScript package MacOS/JavaScript package.zip*
When unzipped, you will find:
1. The MacOS intstaller
2. README instructions for installing
3. A user guide for using the plugin, including doumetation of the APIs and sample scripts.  This also contains technical notes for developers.

***NB This alpha release has no protection against an indefinite JavaScript loop, which would require OpenCPN to be killed off.  You should be confident of your scripts before running them during real navigation!***
