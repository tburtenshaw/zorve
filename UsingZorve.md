# Introduction #

**ZORVE** is a program designed to allow you to edit the information that your Zinwell or Olevia set-top box records to your hard-drive.

If you are not familiar with using Olevia firmware to record then take a look at my other website page: [Using and Recording with Olevia Firmware](http://code.google.com/p/zimview/wiki/UsingAndRecordingWithOleviaFirmware).

You will need operating system access to the hard-drive that you have recorded to. You may be using a _FAT32_ drive or using an _ext3_ driver with Windows.

# Browsing recordings #
ZORVE starts showing the contents of the current directory. Unless you have placed ZORVE onto your the USB hard-drive, this directory is unlikely to contain any recorded video.

Go to _File-Open_ and browse to the _Record\_Video_ directory. Double-click one of the files (they will be of a form similar to 2010Z0227\_1900\_0).

ZORVE will read all through all the relevant files in this directory and display their title, the time and date of recording, and an approximate length. The file you chose will be highlighted.

You can choose files by clicking once with the left mouse button. The _File Information_ window to the right will display some information about that particular recording.

# Changing information #
Often the set-top box will record incorrect information. For instance if you start recording at 6:58pm, it may be titled _Six O'Clock New_ even though you were recording the show afterwards.

Sometimes you may want to put more information about the title. If you are recording _The Simpsons_ every week you may want to have the episode name show up when browsing to play back the good episodes and ignore the horribly unfunny HD episodes.

![http://zorve.googlecode.com/svn/wiki/example_changetitle.png](http://zorve.googlecode.com/svn/wiki/example_changetitle.png)

## Adjusting title and description ##
After you've clicked on a show you want to rename, the _File Information_ window will display its title and description.

You can edit these text-boxes and then click _Save changes_ to permanently change the information that will be display by your set-top box.

If you have typed something that you do not want saved, you can simply click on another recording, or press _Revert_ to load what was stored in the file.

## Choosing a good title and description ##
Although the title can hold 50 (or more) characters, not all of these will fit into the display. This is true about both ZORVE and the Olevia firmware itself. Try to keep the title about as short as the other titles.

The description is limited to 200 characters.