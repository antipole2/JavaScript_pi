# JavaScript plugin for OpenCPN

This plugin allows you to run JavaScript and to interact with OpenCPN.  You can use it to script your own enhancements to standard OpenCPN functionality.

The functionality in OpenCPN that can be scripted depends on the APIs built into the plugin.  These will be expanded as the need arises.  [A user guide is available here.](https://github.com/antipole2/JavaScript_pi/blob/master/JavaScript%20plugin%20user%20guide.pdf)

At this stage the plugin is an alpha release which has been built for MacOS only using Xcode.  It awaits a volunteer familiar with Xcode and Windows to update the Cmake files so it can be built for a non-Xcode environment.  User and t technical guides are included in the project.

***NB This alpha release has no protection against an indefinite JavaScript loop, which would require OpenCPN to be killed off.  You should be confident of your scripts before running them during real navigation!***
