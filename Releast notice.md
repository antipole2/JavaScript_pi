V3 of the JavaScript plugin adds many new features.  References to the User Guide are for section numbers in the new version accompanying V3.

## Clipboard

You can now access text from your clipboard with the `fromClipboard` function and copy text to the clipboard with `toClipBoard`.

User Guide #2.

## Drivers

There is now support for accessing and using OpenCPN's input/output driver handles.

User Guide #3.

## Enduring call-backs

When a callback to a function is set up, it is normally in effect for one call only.  Now many callbacks have an _all_ variant that sets up an enduring callback.

## Navigation data

There are now callbacks for when OpenCPN has revised navigation data.  Added `OCPNonNavigation` & `OCPNonAllNavigation`.

## NMEA0183 data

The NMEA functions have been renamed to NMEA0183 to distinguish them from NMEA2000.  The existing function names remain but are deprecated.

The existing `OCPNonNMEAsentence` callback receives every NMEA0183 sentence.  You can now specify a  three or five character sentence type to receive just those sentences in a callback.  You can have multiple such callbacks so you can handle different sentence types in different functions.

You can now push out NMEA0183 data through a specific driver.

As before, the plugin handles NMEA0183 checksums automatically, checking them on received data and adding them to pushed data.  However, if required, you can now obtain the two check characters with the `NMEA0183checksum` function.

## NMEA2000 support

You can now receive or send NMEA2000 binary data.

You can now construct a JavaScript NMEA2000 object for any PGN for which the Canboat project has a PGN descriptor.  This object's `decode` method can be used to decode NMEA2000 binary data into a JavaScript structure.  An `encode` method will encode a JavaScript structure into a binary message for sending.

User Guide #18.

## Timers

Internally, the timer functionality has been completely rewritten to be more efficient and accurate.  You can now specify timer intervals to less than 1 second (0.5, 2.5 etc.).  You can now cancel individual timers as well as all timers.  You can set a repeating timer.

## Running other processes

There are now APIs to execute a command or run a shell script.  User Guide #19.

## File handling

You can now construct one or more _File_ objects to handle file access.  You can seek to a file position and read or write a specific length of data at a location.  Both text and binary data are supported.

You can now read a text file line-by-line.  This works without reading the whole file into memory and so can be used to process large files.

User Guide #16.

## Enduring variable

There is now a special variable `_remember` for each console that endures between script runs and across plugin restarts.  This can be used to remember information such as preferences.

## Parking

The parking feature has been completely reimplemented.  When a console is parked, it is allocated a space within the parking area.  The Close button can be used to unpark a console. returning it to the position and size from where it was parked.  On re-parking, it returns to its allotted space.

When a console is renamed, the parking lots are adjusted to accomodate its new size.  When a console with a parking space is deleted, the vacated space is removed.

## The console Close button

The console Close button now has several functions.  This is rather unusual but when a console is minimised or parked, its Close button is the only attribute that is actionable.

A script may use the `onCloseButton` function to set up a callback when the Close button is clicked.  This is the way of regaining the attention of a running script without stopping it.

Action on a Close button click depends on circumstances

1. If a script is running and it has set up an `onCloseButton` callback, the callback function is invoked.
2. Else if the console is parked, it will be un-parked.
3. Else the console is considered for closure (deletion).
 * If the console's script is running, you will be invited to stop it.
 * Else if there is a script in the script pane, you will be advise to clear it.
 * If the script is not running and there is no script in the script pane, the console will be deleted.

## Canvas functions

`OCPNgetCanvasView` returns selected attributes of the current canvas view port.

`OCPNcentreCanvas` allows you to centre the canvas at a position and set the zoom level.

## Cleaning text strings

OpenCPN and many text editors support the UTF-8 extended character set.  This includes characters like smart single and double quotes.  These often complicate processing of text and the JavaScript engine does not like them.  The plugin has a built in filter which converts such characters to an appropriate ASCII equivalent.  You can test this filter in the Diagnostics tab of the plugin tools.

The filter now converts superscript 0 to the degree character.

The function `cleanString()` is now available to apply this filter to text you may have read from a file or from the clipboard.

## Load

The Load button on the console allows you to load a script from a file.

You can now copy the URL of a .js file to the clipboard and use the new _URL on clipboard_ button to load the script from the URL.  The URL is noted as a _Recent_ and can be made a _Favourite_ like any other script file.  Just as scripts are loaded from files when the plugin starts up, it will reload from the URL if you are on-line.

If you want to reload a script when you are off-line, you will need to save it to a local file.

## `OCPNsoundAlarm()`

You can now specify a sound file to be played.  The default is the alarm sound.

## Internals

There has been much cleaning up and rationalisation of the internal code.

## User Guide

The User Guide gives fuller details and code illustrations.  It can be 
downloaded as a .pdf [from here](https://github.com/antipole2/JavaScript_pi/blob/e54ebbccab91ae92514b5d61ecaab7fdf537db83/documentation/JavaScript_plugin_user_guide.pdf), or in epub format [from here](https://github.com/antipole2/JavaScript_pi/blob/e54ebbccab91ae92514b5d61ecaab7fdf537db83/documentation/JavaScript_plugin_user_guide.epub).
