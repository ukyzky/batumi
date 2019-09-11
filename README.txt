XOAC Firmware Update Tool
=========================

File: XAOC Firmware Update Tool - OSX.app
File: XAOC Firmware Update Tool - Windows.exe

This utility will copy the new firmware to the module's flash memory
via the USB port on the back. There are two executables: one for
Windows and one for OSX.

To proceed, carefully follow the steps below:
- Unplug Batumi from the Eurorack power, and find a Mini USB cable to connect
  Batumi to your computer;
- Detach the back PCB from the rest of the module by carefully pulling
  on all edges of the PCB at the same time;
- Put the "Update" jumper on the back to the "Update" position; if
  needed, you can take a jumper from the waveform select cluster, you
  will replace it afterwards.
- Do *not* plug the USB cable just yet...
- Launch the update tool, and follow the instructions. You will be
  asked to plug Batumi after a few steps;
- When the update is done, do not forget to put the "Update" jumper
  back to its original position, and to plug it back into your case.

FTDI Drivers
------------

Both Windows and OSX should come pre-installed with generic drivers to
communicate with the Batumi, but in some cases it has been found
necessary to install dedicated drivers (FTDI). First try the tool
without them; if it fails with the error "No new devices found", you
will need to install them. Detailed instructions on how to install the
drivers can be found here:

- Windows: https://learn.sparkfun.com/tutorials/how-to-install-ftdi-drivers/windows---in-depth
- Mac OS X: https://learn.sparkfun.com/tutorials/how-to-install-ftdi-drivers/mac

Important note
--------------

In some rare cases, overheating has been observed when USB is plugged
into Batumi. This could potentially damage your module, and the cause
is still unknown. Detaching the back PCB from the module should
prevent this problem, but we advise nontheless to keep one finger on
the left of the Eurorack power connector (that's where the MCU
is). Should this zone become hot, unplug the USB connector and please
report all the circumstances (positions of the jumpers, PCB detached
or not, step in the update, operating system and driver used...).


Batumi main firmware
====================

File: batumi-v0.45.bin

List of changes with respect to the previous version:

- in Quad mode, Pots 2, 3, and 4 control the respective channel's
  amplitude. CV inputs are added to this level.

- in Quad, Phase and Divide modes, the second Reset input now holds
  (pauses) all the LFOs, the third Reset input reverses the direction
  of the wave, the fourth Reset input selects next waveform. Waveform
  selection is reset when switching modes.

- when exiting Zoom mode, LEDs will flash if the value of a parameter
  does not correspond to the position of its slider when going out of
  Zoom mode.

- in Divide mode, the fine setting will add or subtract 0-3 to the
  division set outside of Zoom mode. It can be used to access
  divisions not accessible previously.

- when in Sync, phase will be aligned to the external clock.

- a short press on the button now exits Zoom mode


Batumi alternate firmware documentation
=======================================

File: batumi-alt-v0.30.bin

The alternate firmware has all the features of the new main firmware
above, plus the following:

- when synced to an external clock, the CV input of the channel
  controls the frequency division/multiplication (1-8). To go out of
  Sync and back to free-running LFO, move the slider of this channel.

- Random wave bank (see below)
- New Quad mode (see below)
- Advanced zoom interface (see below)


Random wave bank
----------------

There are now two banks of assignable waveforms (on the Saw/Asgn
output): the original one--saw, ramp, triangle and trapezoid--and a
new bank of more unpredictable waveforms. To switch bank, press the
button for two seconds, until the short LED animation.

The lower jumper, or the lower switch if you have the Poti expander,
selects the kind of unpredictivity of the waveforms. The first kind is
classic random voltage sampling: it follows a uniform distribution,
successive values are picked uniformly on the whole scale, and they
are uncorrelated to the previous ones. The second kind is the
approximation of a simple chaotic system known as Verhulst's logistic
map. Listening to it carefully, you will hear patterns emerge:
sometimes it will fall into a repetitive sequence for a while, then
break out of it, vary around a random center, then jump
unexpectedly...

The upper jumper or switch turns interpolation on or off: with
interpolation off, the voltages will be stepped like a sample and
hold; with interpolation on, transition between values will be
smoothed.

Resetting a channel will immediately produce a new random value,
therefore possibly producing a discontinuity in the otherwise smooth
waveform.

In Free and Quad mode, the random values of the four channels will be
completely unrelated. But in Phase mode, only one stream of random
value will be produced; the other 3 channels will follow the same
stream, only delayed by the phase shift. Similarly, in Divide
mode, the random values of the slave channels (2-4) will be sampled
from the master channel (1).

New Quad mode
-------------

The Quad mode changes function completely. The old Quadrature mode can
be accessed in Phase mode by setting the sliders 2, 3 and 4 to their
highest position. The phase will then "snap" to quadrature mode.

In the new Quad mode, the channels are summed in various proportions
to obtain original waveforms. The first channel's slider (and CV
input) sets the master frequency; the three remaining ones are locked
to divisions of this frequency (by default, /2, /3 and /4). Each
output is the mix of its own channel's waveform, plus all the channels
on its right. For instance, the third Sine output is the sum of the
sinusoids of Channels 3 and 4; the first Saw output is the sum of the
saws of all channels. Sliders 2, 3 and 4 (and their CV input) set the
amplitudes of their respective channel. Square outputs a 1-bit version
of the Sine output: if Sine is positive, it will be +5V, if Sine is
negative, it will be -5V. Note that the division can be changed with
the advanced Zoom interface (see below).

The Sine outputs can be used in your patches to break the
monotony of modulating with sines. Square output odd rhythms when
it triggers an envelope; they will change dramatically when the levels
are changed, but will always repeat every 24 steps (depending on the
divisions).

You can also use triangle, saw and ramps from the assignable outputs,
but there is one exception in this mode: trapezoid has been replaced
by a square. Contrarily to the Square output, which will always be
binary, thes squares are summed together, therefore producing
repetitive sampled voltages. This square waveform is perfect to
generate surprising sequences: feed the first assignable output to the
FM input of your oscillator and change the levels.

A final tip: try to abuse the first Sine output as a VCO, putting an
offset in the first CV input.

Advanced zoom interface
-----------------------

The Zoom mode changes drastically: instead of a fine control, it now
allows to set four different parameters for each channel, including
the fine control. Zoom mode acts only on the last channel modified; to
select another channel, move its slider a little bit. When
long-pressing on the button, the LED corresponding to the focused
channel will blink.

Each slider now sets a different parameter:
- Slider 1 fine-tunes the value of the main parameter
  (frequency, level, phase or division).
- Slider 2 sets the output levels of the Sine and Saw outputs
- Slider 3 sets the CV attenuation
- Slider 4 sets the phase

There are two exceptions to this assignment, to avoid redundancy:
- In Quad mode, the level is already set by the main parameter, so
  Slider 2 sets the division value (from 1 to 8).
- In Phase mode, the phase is already set by the main parameter, so
  Slider 4 allows to divide the frequency (from 1 to 8).

Batumi custom firmware by ukyzky documentation
==============================================

File: batumi-aiw-ukyzky-v010.bin

Based on amazing Batumi alternate individual waveform firmware,
new feature is:

- In Divide mode, slider can set divide and multiply. center position
  is initial. upper position becomes multiply, and lower position
  becomes divide.

- In Individual Waveform mode, slider can set classic wave and random
  wave. 4 random wave in upper position and 4 classic wave in lower
  position. Before set waveform in this mode, random waveform is
  decided by jumper/Poti. After set waveform in this mode, jumper/
  Poti setting is ignored.

Special thanks
--------------

polyshaft (moddict)

Roadmap (currently not PoC nor implemented)
-------------------------------------------

- In Free mode with Syncing from CV input, slider can set divide and
  multiply.

- Wavefold mode. Like Free mode behavior. Freq CV input can set
  wavefold amount.

- QuadWavefold mode. Like Phase mode behavior. Based on ch.1 signal,
  ch.2-4 slider can set wavefold amount.
