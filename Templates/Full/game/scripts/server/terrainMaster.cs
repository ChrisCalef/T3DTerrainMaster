
//WORLD SERVER: MOST OF THIS IS NOW OBSOLETE.  loadStaticShape() might still be handy, but since 
//we're no longer saving a giant array of torque terrains, we don't need any of those functions. 


///////////////////////////////////////////////////////
// TERRAIN MASTER 
$terrainCheck = 0;//more efficient way to do this without making another global variable?
function TerrainMaster::onAdd()
{
   %temp = new SimSet(StaticGroup);
   %temp = new SimSet(ActorGroup);
   %temp = new SimSet(TerrainGroup);
   
	//%sqlite = new SQLiteObject(sqlite);
   //if (%sqlite == 0)
   //{
      //echo("ERROR: Failed to create SQLiteObject. sqliteTest aborted.");
      //return;
   //} else {
      //sqlite.openDatabase("Ecopocalypse.db");
   //}
}

function TerrainMaster::onRemove()
{
   //sqlite.closeDatabase();
   //sqlite.delete();
}

function saveTerrainRow(%rowNum)
{
   %blockSize = (theTM.worldBlockSize/theTM.squareSize)-1;
   //%blockSize = theTM.getBlockSize()-1;
   echo("loading row for terrain block size: " @ %blockSize);
   for (%i=0;%i<mFloor(theTM.numColumns/%blockSize);%i++)
   {
      theTM.setTerrainHeights(%i*%blockSize,%rowNum*%blockSize);
   }
}

function saveTerrainColumn(%columnNum)
{
   %blockSize = (theTM.worldBlockSize/theTM.squareSize)-1;
   //%blockSize = theTM.getBlockSize()-1;
   echo("loading row for terrain block size: " @ %blockSize);
   for (%i=0;%i<mFloor(theTM.numRows/%blockSize);%i++)
   {
      theTM.setTerrainHeights(%columnNum*%blockSize,%i*%blockSize);
   }
}

function saveTerrain()
{
   echo("SAVING ALL TERRAIN");
   %blockSize = (theTM.worldBlockSize/theTM.squareSize)-1;
   //%blockSize = theTM.getBlockSize()-1;
   echo("loading row for terrain block size: " @ %blockSize);
   for (%i=0;%i<mFloor(theTM.numColumns/%blockSize);%i++)
   {
      for (%j=0;%j<mFloor(theTM.numRows/%blockSize);%j++)
      {
         theTM.setTerrainHeights(%i*%blockSize,%j*%blockSize);
      }
   }
}

function checkTerrain()
{//Might want to rename this, it is really terrain master checking for everything.
   if (theTM.checkCycle)
   {
      theTM.checkTerrain();   
      schedule(theTM.checkCycle,0,checkTerrain);
   }
}

//Sigh...
function tryCheckTerrain()
{
   if (theTM.checkCycle)
      checkTerrain();   
   else
   {  
      echo ("couldn't find terrain master, checking again!");
      if ($terrainCheck<3) //Don't just keep checking forever, two or three tries should do.
         schedule(200,0,tryCheckTerrain);
      $terrainCheck++;
   }
}

function loadTerrainBlock(%filename,%pos)
{
   echo("loading terrain block: " @ %filename);
   %temp = new TerrainBlock() {
      terrainFile = %filename;
      castShadows = "1";
      squareSize = "10";
      baseTexSize = "256";
      lightMapSize = "256";
      screenError = "16";
      position =%pos;
      rotation = "1 0 0 0";
      canSave = "1";
      canSaveDynamicFields = "1";
   };
   TerrainGroup.add(%temp);   
   MissionGroup.add(%temp); 
}

function dropTerrainBlock(%pos)
{
   %terrain = 0;
   for (%i=0;%i<TerrainGroup.getCount();%i++)
   {
      %obj = TerrainGroup.getObject(%i);
      if (VectorLen(VectorSub(%obj.getPosition(),%pos))==0.0)
      {
         %terrain = %obj;
      }      
   }
   if (%terrain>0)
   {
      echo("Removing and deleting terrain: " @ %terrain);
      TerrainGroup.remove(%terrain);
      MissionGroup.remove(%terrain);
      %terrain.delete();
   }
}

function loadStaticShape(%filename,%pos,%quat,%scale,%id)
{
   echo("loading static from script! " @ %filename @ " pos " @ %pos @ " quat " @ %quat @ " scale " @ %scale);
   %temp = new TSStatic() {
      shapeName = %filename;
      dbID = %id;
      position =%pos;
      rotation = %quat;
      scale = %scale;
      collisionType = "Visible Mesh";
      decalType = "Visible Mesh";
      allowPlayerStep = "1";
      customAmbientLighting = "0 0 0 1";
      enabled = "1";
      receiveLMLighting = "0";
      receiveSunLight = "0";
      useCustomAmbientLighting = "0";
   };
   StaticGroup.add(%temp);   
   MissionGroup.add(%temp);   
}

function WorldServerReloadSkybox()
{
   WS_SkyboxCubemap.updateFaces();//HERE: deal with different cubemap names?  Naw...  
}