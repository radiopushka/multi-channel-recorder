# Multi Channel Recorder
A Linux "script" that can record multiple audio interfaces and merge them into a file
	for example, running: $recorder default default
	
For Ubuntu and Mx Linux you will see two recording interfaces in the Pulse Audio Volume Control program

you can change the Port there, to have one record the microphone and the other record the monitor of Built in Analog Stereo

Then if you press CTRL-C in the terminal where recorder is runnning it will merge the audio recorded from the two interfaces together into a file called 
"out.wav" which will appear in the same directory you ran recorder

the "out.wav" can then be opened by Audacity.

This can be used for making voice overs for youtube videos for example.

The program can open as many interfaces as you tell it too, but make sure in Pulse Audio that all of these interfaces are recording from DIFFERENT sources.

compilation: sh compile.sh

install: sudo cp ./recorder /usr/local/bin
