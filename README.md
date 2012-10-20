MrsWatson
=========

MrsWatson is a command-line audio plugin host. It takes one or more plugins,
sends an audio signal to them for processing, and saves the resulting output.
Although the plugins think they are running in realtime, processing is
actually done offline (this is not the same as the "offline" processing in VST
terminology). This makes MrsWatson ideal for developing or debugging plugins,
or processing audio on a server.


History
-------

MrsWatson takes its name from a prior product from [Teragon Audio][1] called
*MissWatson* (and the original name there, in case you were wondering, is a
bit of a joke on the [Dr. Watson][2] utility).

In 2009 I sold the exclusive rights to MissWatson to a company interested in
using it for server-side audio processing. As per the terms of our agreement,
I made one last public release, and removed the code from my website. Since
then, I have received numerous emails and inquiries regarding the project,
mostly from plugin developers or people interested in VST technology. Though I
have no regrets about discontinuing the original MissWatson (particularly
since I didn't have time to properly maintain it), I felt that it was a useful
utility which should be made available and open-sourced.

In 2011, my NDA with the aforementioned company expired, and I started working
on a new MissWatson. MrsWatson is the result of this labor.

As I no longer have the source code for MissWatson, MrsWatson is a black-box
implementation. They share only the name and concept in common. The two
programs are **incompatible** and have **fundamentally different behavior**.
They are also written in different languages (the original in C++, MrsWatson
in C), and have completely different internal architecture.

If you have built some system which used the original MissWatson, then you
will likely have to make some big changes to migrate to MrsWatson. Likewise,
if you were using some of the original MissWatson code in your project, you
will *not* be able to replace it with MrsWatson without significant work on
your end.


Examples
--------

Say you have an audio file which you would like processed through a group of
mastering plugins.

    mrswatson --input mysong.pcm --output out.pcm --plugin plugin1;plugin2

This will print the following output:

    - 00000000 000000 Plugin 'plugin1' is of type VST2.x
    - 00000000 000000 Plugin 'plugin2' is of type VST2.x
    - 00000000 000000 MrsWatson version 0.1.0 initialized
    - 00000000 000000 Opening VST2.x plugin 'plugin1'
    - 00000000 000029 Opening VST2.x plugin 'plugin2'
    - 00000000 000144 Processing with samplerate 44100, blocksize 512, 2 channels
    - 03532800 000204 Total processing time 60ms, approximate breakdown by component:
    - 03532800 000204   plugin1: 38ms
    - 03532800 000204   plugin2: 11ms
    - 03532800 000204   MrsWatson: 11ms
    - 03532800 000204 Read 3528000 frames from mysong.pcm, wrote 3532800 frames to out.pcm
    - 03532800 000204 Shutting down
    - 03532800 000210 Closing plugin 'plugin1'
    - 03532800 000212 Closing plugin 'plugin2'
    - 03532800 000212 Goodbye!

To see more or less logging output, use the `--verbose` or `--quiet` options,
respectively. MrsWatson generates colored output (if your terminal supports
it) with two times per line, the first for the current sample and the second
for the time in milliseconds since processing began. The sample time also
changes colors after every 44100 samples to help visually break up processing
times. This value can be changed with the `--zebra-size`  option.

To process a MIDI file through an instrument, you'd do something like this:

    mrswatson --midi-file mysong.mid --output out.pcm --plugin piano,soft.fxp

Like the first example, a list of plugins separated with semi-colons can be
given here so that the audio generated by the instrument can be processed
through any number of effects. Each plugin name can be followed by a comma and
the location to a preset file to be loaded before processing.

Complete help for MrsWatson can be found by running the program with no
arguments, or with `--help`. The `--help` switch prints quite a lot of output,
so you can also use `--options` to see all supported options and their default
values.


Loading Plugins
---------------

Currently, MrsWatson loads plugins by their short name by searching in the
standard installation locations for your platform, as well as the current
working directory and by absolute path. Use the `--list-plugins` option to see
the order of locations searched and the plugins found there.

While MrsWatson has some understanding of getting and setting plugin
parameters, it does not extend this functionality to the end user. Rather than
passing a huge list of parameters on the command line, one should instead
create a preset for the plugin and load it with the comma-separated syntax
as shown above.


Limitations
-----------

As MrsWatson is a new codebase, there are lots of missing features, some more
important than others. To encourage a quick initial release, the following
features are not yet present in MrsWatson, but may be added at some point in
the future:

* AudioUnit plugins on Mac OSX
* File support for compressed audio
* Resampling of input source if desired
* True realtime mode

I have also tried to identify incomplete areas of the code and log them to the
console, which means that we are aware that this feature is missing and will
be added soon. If you see some other missing functionality or experience a
crash or other bug, please report an issue on the [project page][3].


Building
--------

MrsWatson can be built either from the command line or with a few popular IDE's.
For the command line builds, you will need the following:

* CMake version 2.6 or better
* A relatively recent automake
* A relatively recent autoconf
* On Windows, either mingw or Visual Studio 2010

To build from a fresh clone, first run `cmake .`, then `make`. The build will
create an executable named `mrswatson` in the `main` directory, and another
one named `mrswatsontest` in the `test` directory.

To make a debug build, instead run `cmake -DCMAKE_BUILD_TYPE=Debug` or use the
`ccmake` GUI to switch the build type.

As MrsWatson is a 32-bit executable, which is needed to support 32-bit plugins
(of which are the vast majority in the plugin world), you may also need some
32-bit compatibility libraries installed on your system. On Linux, you may
need to install the following packages to build MrsWatson from source:

* gcc-multilib, and also possibly gcc-4.6-multilib
* g++-multilib, and also possibly g++-4.6-multilib
* ia32-libs
* libc6-dev:i386

Alternately, MrsWatson can be built with a few popular IDE's. Build files are
included in the `projects` directory under the project sources root.
Distribution builds of MrsWatson are always generated with `make`, except for
on Windows where the Visual Studio project is used.


Bug Reporting
-------------

The easiest way to report a bug is to send an email to Teragon Audio's support
address: support (at) teragonaudio (dot) com. MrsWatson has a special
command-line switch to aid in diagnosing runtime problems, `--error-report`.
When enabled it will create a zipfile on the desktop containing the input,
output, logs, and optionally the plugins themselves. Please include these
reports for bugs resulting in incorrect behavior or crashes.

MrsWatson uses [ticgit][4] for bug reporting, so the list of current issues
can be viewed within the repository itself.

Please Help!
------------

As mentioned, MrsWatson is fully functional but needs a lot of polish. If you
find areas of the code which are incomplete or not compatible with some
plugins, please make a pull request.

Likewise, if you want to help out without programming, the best way to do so
is to test and report any problems with given plugins or platforms.


Donate
------

If you are using MrsWatson to do something cool, please send me a link to your
project! If you appreciate MrsWatson and would like to donate money, please
instead make a donation to a charity on our behalf, and let us know about it.
The organizations which have helped us the most are:

* [EFF](https://supporters.eff.org/donate): Without the EFF, programs like
  MrsWatson would be significantly harder to create and distribute.
* [Wikipedia](http://wikimediafoundation.org/wiki/WMFJA085/en): Writing
  MrsWatson involves a lot of research as well as coding, and Wikipedia is
  an essential part of this.


Special Thanks
--------------

Big additional thanks to:

* Andrew McCrea (@thrusong)


Licensing
---------

MrsWatson is made available under the BSD license. For more details, see the
`LICENSE` file distributed with the source code. MrsWatson also uses the
following third-party libraries, which are licensed under the respective
agreements:

* [VST](http://www.steinberg.net/en/company/developer.html): Licensed under
  Steinberg's VST SDK license agreement, version 2.4. For more information,
  see Steinberg's developer portal.
* [LibAiff](http://aifftools.sourceforge.net/libaiff/): Written by Marco
  Trillo, made under the BSD license. For more details, see the LICENSE file
  in the LibAiff source code.


[1]: http://www.teragonaudio.com
[2]: http://en.wikipedia.org/wiki/Dr._Watson_(debugger)
[3]: http://github.com/teragonaudio/MrsWatson
[4]: https://github.com/schacon/ticgit/wiki
