Torque3D   +   -Terrain-Master- by BrokeAss Games, LLC
========       =======================================


-Terrain-Master- by BrokeAss Games, LLC

Author: Chris Calef
Email:  chris@brokeassgames.com
Date:   November 15, 2012

Welcome to the T3D Terrain Master documentation.  

T3D Terrain Master is a free software project licensed under MIT, and built on the Torque3D Game 
Engine from Garage Games.  Please see the following documentation and licenses associated with T3D:

(Continue reading below the T3D information for the rest of the Terrain Master README.)


===========================================================================================
MIT Licensed Open Source version of [Torque 3D](http://www.garagegames.com/products/torque-3d) from [GarageGames](http://www.garagegames.com)

More Information
----------------

* Documentation is in the [Torque3D-Documentation](https://github.com/GarageGames/Torque3D-Documentation) GitHub repo.
* T3D [Beginner's Forum](http://www.garagegames.com/community/forums/73)
* T3D [Professional Forum](http://www.garagegames.com/community/forums/63)
* GarageGames [Professional Services](http://services.garagegames.com/)

License
-------

Copyright (c) 2012 GarageGames, LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

===========================================================================================


TERRAIN MASTER

1. What is the T3D Terrain Master?

2. What do I need to get started? 

3. How can I use TM to import USGS DEM data?

4. How can I use TM for paging terrains in T3D?

5. Future directions, and how you can help...


------------------------------------------------------------------------------------------------

1. What is the T3D Terrain Master?

    The T3D Terrain Master, by BrokeAss Games, LLC, is a free add on for the T3D Game Engine
 by Garage Games.  It provides two services:  a system for importing very large terrain 
 heightmap datasets, and a paging system for loading and dropping terrain tiles as the player 
 moves across the landscape.

     T3D can already import terrain heightmaps out of the box, but only one at a time.  Terrain
 Master works with much larger datasets, saved as raw binary float matrices.  (If you don't
 know what that is, don't worry, we'll get to it.)  Since the datasets we're interested in 
 are too large to render as a single map, Terrain Master breaks them up into a grid, with 
 relative positions stored in the filenames.

     As of this writing, the second service of Terrain Master, the terrain paging system, is
 in need of some work.  It functions for the first small set of terrains, but then promptly 
 crashes, apparently because of my delete terrain process has some fundamental flaw.  It is 
 hoped that among the first group of early adopters there may be someone who can point out
 the problem and help us on our way!

------------------------------------------------------------------------------------------------

2. What do I need to get started? 

     First, I'm assuming the fact that you are looking at this readme means you have already 
 acquired the T3DTerrainMaster project from:

         http://github.com/ChrisCalef/T3DTerrainMaster

     If you have not, and are reading this from some other source, then be sure you understand
 how git and github work, and make sure you have a git client you are comfortable with if you 
 don't plan on using the command line, and then go ahead and start by cloning the URL above..

     Now, that said, there are two other programs which you will find very useful if not absolutely 
 mandatory.  The first is called 3DEM, and although it is officially no longer under development
 and the website has been taken down, there is another site where you can find it, along with
 some helpful tips:

         http://freegeographytools.com/category/3dem

     In case you miss it, the direct download link is here:

         http://freegeographytools.com/3dem_setup.exe

     The second program is called L3DT, and you might as well grab it right now as well if you 
 have the budget (indie license is $35). There is a free version as well, but among many other
 limitations, it won't export Torque terrain files, so it won't do you much good right now.

 The professional version can be found here:

         http://www.bundysoft.com/L3DT/downloads/professional.php

------------------------------------------------------------------------------------------------

3. How can I use TM to import USGS DEM data?

     First: if you are exclusively interested in the terrain paging aspects of Terrain Master, 
 and you do not wish to bother downloading any new DEM data, then you may grab the Southern 
 Willamette Valley dataset I have been working with from BAG's servers, here:

         http://www.brokeassgames.com/blogs/chris/TerrainMaster/SouthernWillamette.zip

     You can find a visual overview of the entire dataset here:

         http://www.brokeassgames.com/blogs/chris/TerrainMaster/SouthernWillamette.jpg

     However, from here out I will assume you wish to download and import data for your own region,
 or some other area of interest besides the Southern Willamette Valley in Oregon.

     The first thing you need to do is download the DEM data.  You can get 10 meter 
 resolution heightmap data for (as far as I know) the entire continental United States,
 from the following website:

         http://data.geocomm.com/dem/demdownload.html

     You will have to click around a bit to find what you need, but start by clicking on your
 state, then find you county, and then (for Oregon, at least) go to the link labeled 
 "Digital Elevation Models (DEM) - 24K".  From that point, you should be confronted with a
 list of names of either towns or landscape features.  These correspond to the grid of USGS
 quad map names for your area... I can't link directly to a map laying these out for you, but 
 if you look around long enough on the GeoCommunity site you should be able to find one.

     Once you have your bearings and you know what DEM maps you are going to need, go ahead and grab 
 a few of them, in a block.  I can't speak for your results, but on my not-fancy hardware I was
 able to import a block of about 24 (six wide by four high) into 3DEM before I ran out of RAM.

     But I should back up a bit.  3DEM loads DEM maps, shows them to you in color coded form, merges
 multiple contiguous DEM maps into a single map, and supports several different export formats.
 To open multiple DEMs, you merely have to multiselect them and open them, just as you would a 
 single map.  

     Once you get your data loaded, the export format we are interested in is Terrain Matrix, which is
 simply a raw array of floating point data, along with a header file telling us the exact dimensions
 of the array and some other useful data.

     When you have loaded all the files you are going to want to use, go to File->Save Terrain Matrix,
 select Binary/Floating Point, and put the resulting .bin file into your art/terrains directory.
 
     Now, look for the .hdr file that should have been saved next to the .bin file.  It should look 
 something like this:

 file_title             = SouthernWillamette
 data_format            = float32
 map_projection         = UTM Zone 10N
 ellipsoid              = NAD27
 left_map_x             = 459750
 lower_map_y            = 4843900
 right_map_x            = 520120
 upper_map_y            = 4899540
 number_of_rows         = 5565
 number_of_columns      = 6038
 elev_m_unit            = meters
 elev_m_minimum         = 88
 elev_m_maximum         = 1175
 elev_m_missing_flag    = -32767

     We will probably need to use some of that other data before we're finished, but for now all we
 need to worry about are the number_of_rows and number_of_columns field.  You'll want to write 
 them down or remember they were here in a moment.

     The next thing you will need to do is examine your EmptyRoom.mis file in Templates/Full/game/levels.
 (I built the Terrain Master in the Templates/Full project, but it should be quick work in WinMerge to 
 port it to your own project, there are few files modified.)  You will notice a new addition to the mission:

   new TerrainMaster(theTM) {
      squareSize = "10";
      matrixFile = "art/terrains/SouthernWillamette.bin";
      outputDirectory = "art/terrains/SouthernWillamette256/backup";
      inputDirectory = "art/terrains/SouthernWillamette256";
      checkCycle = "1000";
      loadDistance = "5100";
      dropDistance = "7650";
      left_map_x = "459750";
      lower_map_y = "4843900";
      right_map_x = "520120";
      upper_map_y = "4899540";
      numRows = "5565";
      numColumns = "6038";
      numGridRows = "21";
      numGridColumns = "23";
      minHeight = "88";
      maxHeight = "1175";
      worldBlockSize = "2560";
      canSave = "1";
      canSaveDynamicFields = "1";
      position = "0 0 0";
      rotation = "1 0 0 0";
   };

    To use TerrainMaster with your own DEM files, you will need to modify the paths and filenames accordingly,
 and set the numRows and numColumns.  Most of the other fields will not be used for the terrain import phase, 
 but a few of them will become important when we get to the next step, terrain paging.  You can copy the other
 data over if you want, although you won't know the number of grid rows and grid columns until we finish the 
 import step.  You should leave squareSize and worldBlockSize alone.

    Once you have verified appropriate paths and number of rows/columns (taken from your .hdr file), you are
 ready for the big step of importing your dataset!

    First, make sure you have a directory (in my case it was called SouthernWillamette256, but you can name
 it whatever you want) in your art/terrains directory.  Then make sure you have another directory called 
 backup inside that directory.  That "backup" folder is so named because this is where you will store the 
 originals of all of your terrain tiles.  You can then copy them up a level and do whatever modifications 
 of them you might want to do later.

    If you have your directory structure in place, and you have your .bin file in your art/terrains folder
 and have the name recorded under matrixFile... then you are ready to go!  All you need to do is run your 
 Full.exe executable, and select the ImportTerrain mission.  Then, when the level loads (you should see a 
 large perfectly flat terrain block beneath you) simply go to the console and type:

        saveTerrain();

    And that's all there is to it!  You might want to open up a windows explorer window to your backup folder 
 so you can watch it fill up, it's kind of gratifying.

    Now, for a little more explanation... what you should see is a series of terrain files being generated, 
 with filenames that look like this:

        0_0.ter
        0_255.ter
        0_510.ter
        ...
        255_0.ter
        255_255.ter
        255_510.ter
        ...

    This should go all the way up to whatever your maximum, mine is 5610_5100.ter.  These are 256x256 point 
 terrains, with 10 meter square size.  Because the number of actual terrain squares is the number of data 
 points minus one,  we actually only cover 2550 meters with each terrain tile instead of the 2560 you might
 expect, which is why the numbers go in increments of 255 instead of 256.

    Now, what you have here is a large number of T3D terrain files, with no texture, lightmap, or normal map
 information at all.  You can use them as is, but they will probably default to an ugly grass texture, and
 it is unlikely they will serve you for any final game or rendering application in this form.  This is why
 I had you download L3DT, because it is an amazing application for smoothing, modifying, and texturing 
 Torque terrains.  

    Unfortunately, and here we are coming to one of the ways you might be able to help this project along...
 I did not get far enough in my L3DT research to figure out a way to automate the next part of the process,
 which is running each of these terrains through L3DT's Calculation Wizard and Alpha Map generator, and 
 saving them all back out.  I don't even know if this would be better off done with a script or a plugin,
 but I hope to find out before too long.  In the meantime, you can process a few of your files by hand to 
 see what they will look like, and you can certainly go through and do your whole pile manually if you need
 them done right now... but I do hope to find a more civilized way soon.

    Whatever you do, you will need to decide where to put the finished product.  I suggest keeping your 
 original terrain files in the "backup" directory as mentioned above, and then either exporting finished, 
 processed terrains to the directory above (SouthernWillamette256 in my example), or just copying up the
 originals to this directory if you are in a hurry, OR if you are really in a hurry, simply changing the 
 inputDirectory parameter of your TerrainMaster object to your backup directory, if you do not want to 
 do any further processing to your files.
 
------------------------------------------------------------------------------------------------

4. How can I use TM for paging terrains in T3D?

    But anyway, now that you have either generated your own great batch of T3D terrain files, or else downloaded 
 mine and dropped them into your art/terrains folder, the next step is to try out the terrain pager!  

    If you have made your own terrains, however, you have one tiny bit of math to do before you start.
 Remember the numGridRows and numGridColumns fields that I mentioned before?  This is where you need them.  
 What they are is the total count of terrain tiles in your new grid, horizontally (columns) and vertically
 (rows).  The way you find them is by taking the final coordinates from the filename of the last terrain
 file in your backup directory, dividing each number by 255, and adding one.  

    The first number is your numGridColumns (or X), the second is your numGridRows (or Y).  In my case, where my 
 last file was 5610_5100.ter, I would get:

        numGridColumns = (5610 / 255) = 22 + 1 = 23
        numGridColumns = (5100 / 255) = 20 + 1 = 21

    You'll need to enter those numbers into the TerrainMaster block in whatever mission you'd like to see
 the terrain paging in.  (I suggest EmptyRoom.mis for testing.)

    And now, it's time for another sort of a kluge.  You can either load the mission and then bring up the 
 console real fast and type "tryCheckTerrain", or you can go to game/scripts/server/player.cs and uncomment
 the last line in Armor::onAdd(), to do it automatically:

        ////HERE: uncomment this when you are ready to start terrain paging.
        //tryCheckTerrain();

    With that done, you should see the bottom leftmost terrain square from your big map appearing beneath you, 
 as well as a few more to the north and east.  The exact number that appear at any one time is determined by 
 the "loadDistance" parameter of your TerrainMaster object.  It is currently set at a conservative 5100, or two
 terrain tiles' distance, but you can probably get away with higher numbers.  You will find the limit when 
 Torque starts to crash on you. ;-)

------------------------------------------------------------------------------------------------

5. Future directions, and how you can help...

    First, how can you help?  Well, I'm glad you asked!  I have a couple of pressing concerns right now with 
 this build:

    A) Crash Bug:  The Terrain Master at time of this writing actually has a _huge_ crash bug, rendering it 
 inoperable outside of the first few terrains loaded... :-( ... but I'm hoping that by exposing my code to the 
 light of public scrutiny, my blundering errors shall be promptly corrected.  If you are a competent and 
 experienced, or just novice and lucky, Torque programmer, and you have a moment, then please look over the
 code here, and tell me: what is wrong with my process for deleting terrain blocks??  It appears that I am 
 failing to free some huge chunk of memory somewhere, or some equally horrible crime, because whether or not 
 I am deleting terrains, when I add beyond a certain number of tiles the program crashes, off in some dll world
 where Visual Studio debugging doesn't give me a useful callstack.  

    I'm calling the Waahmbulance, and it is you...

    B) The other major area where I could use assistance from the knowledgeable people in the Torque community
 is with scripting, or coding plugins for, L3DT.  This is an amazing app, and I'm very excited to be using it, 
 but with all my other work I have not had time to dig deep into and and find out how to do what I need to do. 
 Which is, namely:  loop through a directory full of .ter files, and for each terrain, call all the Calculation
 Wizard operations, generate Alpha Maps, and save everything back out as T3D terrains, with no interactive button
 clicking necessary.  I have 483 files to work with in just this one batch, and a very limited supply of patience,
 so if I am ever to see this project to its logical conclusion using L3DT, I'm going to need help!

    Beyond those two immediate pressing needs, you are invited to fork and advance to your heart's content.  I
 would be honored to accept pull requests for any improvement you feel would be generally useful.  One direction
 for improvement I'm already considering is to include a much larger scale "mega-terrain" with a square size of 
 40 or 80 meters, to be used outside all of the currently loaded smaller terrain tiles.  This would require 
 dropping out areas of the larger terrain, and replacing them when local terrains are deleted again, but it could
 enable the player to have a view of extremely distant landmarks which would actually be expected by a player 
 familiar with the geography in question.  (For example, South Sister is a mountain clearly visible from Eugene
 on a good day, but it is more than sixty miles away, far beyond any range that we would wish to fill with 10
 meter terrain data!

    As of this writing, this project is only a couple of weekends deep, plus a few extra nights, but I hope it 
 provides a foundation worthy of building on.


 Chris Calef
 chris@brokeassgames.com
 BrokeAss Games, LLC
 Eugene, OR





Terrain Master License
----------------------

Copyright (c) 2012 BrokeGames, LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.