//-----------------------------------------------------------------------------
// Copyright (c) 2012 BrokeAss Games, LLC
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "terrain/terrMaster.h"
#include "console/consoleTypes.h"
#include "core/stream/fileStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/camera.h"
#include "T3D/player.h"
#include "gfx/bitmap/gBitmap.h"
#include "gui/worldEditor/terrainEditor.h"
#include <stdio.h>


#include <iostream>
#include <sstream>
#include <fstream>


#include <vector>
#include <string>

using std::vector;
using std::string;




//float mapCenterLong = -123.10f;//FIX: find this from TerrainMaster object(?)
//float mapCenterLat = 44.00f;

//float skyboxDistance = 20.0f;

//void skyboxSocketConnect();
//void skyboxSocketListen();

//void skyboxSocketStart();
//void skyboxSocketDraw();
//void terrainSocketListen();

//int mTick=0;//Better way to measure time?

//END SOCKETS



IMPLEMENT_CONOBJECT( TerrainMaster );

ConsoleDocClass( TerrainMaster,
	"@brief The TerrainMaster class organizes terrains in a grid ");

TerrainMaster::TerrainMaster()
{
	mNumRows = 0;
	mNumColumns = 0;
	mCurrentGridRow = 0;
	mCurrentGridColumn = 0;
	mLeftMapX = 0;
	mLowerMapY = 0;
	mRightMapX = 0;
	mUpperMapY = 0;
	mCheckCycle = 1000;
	mTick = 0;
	mTickInterval = 150;
	mNumHeightBinArgs = 5;//lat, long, tileWidth, heightmapRes, textureRes 

	//mSQL = new SQLiteObject();
	mClientPos.zero();
	mStartPos.zero();

	mTileWidth = 0.0f;//2.5km squares, by default.
	mFirstTerrain = false;
	mAllTerrains = false;

	mPlayerLongitude = 0.0f;
	mPlayerLatitude = 0.0f;
	mPlayerElevation = 0.0f;

	//HERE: this is the geographic center of the whole map
	mCenterLatitude = 44.00f;//FIX!!  persist fields!
	mCenterLongitude = -123.10f;//FIX!!  persist fields!

	mSkyboxPath = "art/skies/WorldServer/";//FIX!!  persist fields!
	mTerrainPath = "art/terrains/WorldServer/";//FIX!!  persist fields!	
	mSkyboxLockfile = mSkyboxPath + "lockfile.skybox.tmp";
	mTerrainLockfile = mTerrainPath + "lockfile.terrain.tmp";
	mWorldServerIP = "127.0.0.1";//FIX!!  persist fields!
	//mWorldServerIP = "192.168.1.14";
	mWorldServerPort = 9934;//FIX!!  persist fields!

	//mTerrainServerIP = "50.112.216.154";
	//mTerrainServerPort = 9935;   //TEMP, combine all these
	//mTerrainTexturePort = 9936;  //TEMP, combine all these

	mNumHeightBinArgs = 5;// lat, long, tileWidth, heightmapRes, textureRes - these are to tell you where to
	//put your terrains (lat/long) as well as whether the resolutions are still valid.
		
	mTerrainsXCount = 3;//Probably unnecessary complexity, but keep it.
	mTerrainsZCount = 3;//Need to go through and fix the paging logic to deal with this though
	mTerrainsXMid = (mTerrainsXCount-1)/2;//where (PlayerGridX=2), etc, it needs to be 
	mTerrainsZMid = (mTerrainsZCount-1)/2;//(PlayerGridX=mTerrainsXCount-1), still zero on low end.

	mMetersPerDegreeLong = 80389.38609f;//FIX!!  Compute:  mTileWidth / mTileWidthLongitude ;//REMOVE: get mTileWIdthLongitude 
	mDegreesPerMeterLong = 0.000012439f;//FIX!!  Compute:  mTileWidthLongitude / mTileWidth;//and mTileWidth specified
	mMetersPerDegreeLat  = 111169.0164f;//FIX!!  Compute:  mTileWidth / mTileWidthLatitude ;//in input script (mission object).
	mDegreesPerMeterLat  = 0.000008995f;//FIX!!  Compute:  mTileWidthLatitude / mTileWidth;//Compute these from there.

	mMinElev = 88.0f;//FIX!!  persist fields!
	mMaxElev = 1175.0f;//FIX!!  persist fields!
	mDeltaElev = mMaxElev - mMinElev;  
	mSquareSize = 10.0f;//FIX!!  Terrain properties!

	mFreeCamera = false;
	mTerrainCreating = false;

	for (U32 i=0;i<TM_MAX_STATICS;i++) mStatics[i]=false;
	//for (U32 i=0;i<TM_MAX_TERRAINS;i++) mTerrains[i]=false;
	for (U32 i=0;i<TM_MAX_BLOBS;i++) mBlobs[i]=false;

	//////////// OBSOLETE ///////////////////
	//Fill in the mNeighTerrAdj array, to describe relative position of each array element.
	//[0] is N, so (0,1), [1] is NE, go clockwise, so [4] is (0,-1) and [5] is (-1,-1), etc. 
	mNeighTerrAdj[0].set(0,1);
	mNeighTerrAdj[1].set(1,1);
	mNeighTerrAdj[2].set(1,0);
	mNeighTerrAdj[3].set(1,-1);
	mNeighTerrAdj[4].set(0,-1);
	mNeighTerrAdj[5].set(-1,-1);
	mNeighTerrAdj[6].set(-1,0);
	mNeighTerrAdj[7].set(-1,1);
	//////////// OBSOLETE ///////////////////
	
	//sockTimeout=0;//seconds
	//skyboxStage=0;
	//skyboxNum=0;
	//skyboxLoopInterval=10;//How many main loops we skip between socket listens.
	//skyboxReady=false;
	//mFullRebuild=false;

	//if (mSQL->OpenDatabase("Ecopocalypse.db"))//HERE: open this up to a script variable maybe?
	//{
	//	Con::printf("Opened the database!!!!!");
	//}
}

TerrainMaster::~TerrainMaster()
{	
	//mSQL->CloseDatabase();
	//delete mSQL;
}

void TerrainMaster::initPersistFields()
{
	addField( "squareSize", TypeF32, Offset( mSquareSize, TerrainMaster ), "Base grid unit size, in meters." );
	addField( "matrixFile", TypeStringFilename, Offset( mMatrixFile, TerrainMaster ), "Terrain Matrix File" );
	addField( "outputDirectory", TypeStringFilename, Offset( mOutputDirectory, TerrainMaster ), "Directory for .ter file output, when creating maps (no trailing slash)" );
	addField( "inputDirectory", TypeStringFilename, Offset( mInputDirectory, TerrainMaster ), "Directory for .ter file input, when loading maps (no trailing slash)" );

	addField( "checkCycle", TypeS32, Offset( mCheckCycle, TerrainMaster ), "checkTerrain cycle time, in MS" );
	addField( "loadDistance", TypeF32, Offset( mLoadDistance, TerrainMaster ), "Distance in meters at which to load terrain." );
	addField( "dropDistance", TypeF32, Offset( mDropDistance, TerrainMaster ), "Distance in meters at which to drop terrain." );

	addField( "left_map_x", TypeS32, Offset( mLeftMapX, TerrainMaster ), "" );
	addField( "lower_map_y", TypeS32, Offset( mLowerMapY, TerrainMaster ), "" );
	addField( "right_map_x", TypeS32, Offset( mRightMapX, TerrainMaster ), "" );
	addField( "upper_map_y", TypeS32, Offset( mUpperMapY, TerrainMaster ), "" );

	addField( "numRows", TypeS32, Offset( mNumRows, TerrainMaster ), "Terrain Matrix Rows" );
	addField( "numColumns", TypeS32, Offset( mNumColumns, TerrainMaster ), "Terrain Matrix Columns" );

	addField( "numGridRows", TypeS32, Offset( mNumGridRows, TerrainMaster ), "Terrain Matrix Grid Rows" );
	addField( "numGridColumns", TypeS32, Offset( mNumGridColumns, TerrainMaster ), "Terrain Matrix Grid Columns" );

	addField( "minHeight", TypeF32, Offset( mMinHeight, TerrainMaster ), "Terrain Matrix min height" );
	addField( "maxHeight", TypeF32, Offset( mMaxHeight, TerrainMaster ), "Terrain Matrix max height" );

	//addField( "tileWidth", TypeF32, Offset( mTileWidth, TerrainMaster ), "Terrain Matrix world block size" );

	Parent::initPersistFields();
}

bool TerrainMaster::onAdd()
{
	if ( !Parent::onAdd() )
		return false;
	
	//if (!mSQL) return false;

	//char query[512];
	//int result;//WARNING body_id,joint_id,
	//sqlite_resultset *resultSet;

	//sprintf(query,"SELECT username,start_x,start_y,start_z FROM clientConnection;");
	//result = mSQL->ExecuteSQL(query);
	//resultSet = mSQL->GetResultSet(result);
	//if (resultSet->iNumRows==0)
	//{
	//	mSQL->CloseDatabase();
	//	delete mSQL;
	//	return false;
	//} else {
	//	mClientUsername = resultSet->vRows[0]->vColumnValues[0];
	//	Point3F pos = Point3F(dAtof(resultSet->vRows[0]->vColumnValues[1]),dAtof(resultSet->vRows[0]->vColumnValues[2]),dAtof(resultSet->vRows[0]->vColumnValues[3]));
	//	mClientPos = pos;
	//	Con::printf("Found a client connection in the database! username: %s  pos %f %f %f",mClientUsername.c_str(),pos.x,pos.y,pos.z);
	//}

	//mCurrentTerrain = NULL;
	
	SimSet* scopeAlwaysSet = Sim::getGhostAlwaysSet();
	const TerrainBlock *tempTerrain;
	for(SimSet::iterator itr = scopeAlwaysSet->begin(); itr != scopeAlwaysSet->end(); itr++)
	{
		TerrainBlock* block = dynamic_cast<TerrainBlock*>(*itr);
		if( block )
		{
			Con::printf("found a terrain!! %s",block->getName());
			//mCurrentTerrain = block;
			char terrName[255];
			for (U32 x=0;x<mTerrainsXCount;x++) {
				for (U32 z=0;z<mTerrainsZCount;z++) {
					sprintf(terrName,"Terrain_%d_%d",z,x);					
					if (!strcmp(block->getName(),terrName))
					{
						mTerrains[z][x] = block;
						mTerrainsLayout[z][x] = block;
						Point3F pos = block->getPosition();
						Con::printf("terrain block assigned %d  %d  pos %f %f %f",z,x,pos.x,pos.y,pos.z);
					}
				}
			}			
		}
	}
	
	mSquareSize = mTerrains[1][1]->getSquareSize();
	mTileWidth = mTerrains[1][1]->getWorldBlockSize();
	mHeightmapRes = mTerrains[1][1]->getBlockSize();
	mTextureRes = mTerrains[1][1]->mBaseTexSize;
	mLightmapRes = mTerrains[1][1]->getLightMapSize();

	mTileWidthLongitude = mDegreesPerMeterLong * mTileWidth;//0.031845f;
	mTileWidthLatitude  = mDegreesPerMeterLat * mTileWidth;//0.023028f;

	//HERE: let's do the rest of the startup logic.  Find the camera pos, find out if we have
	//a functional terrain_1_1.heights.bin (assume the rest of them are there if that one is.)


	//setupSkyboxTextures();//However this ends up working out in Torque...

	Con::executef(this, "onAdd", getIdString());

	return true;
}


	//HERE: this mCurrentTerrain block needs to be mTerrains[1][1], and we need to create the ones around that.
	//Instead of a vector, I need two arrays, the actual mTerrains[][] and then mTerrainsLayout[][].
	
	//HMMM - this isn't going to work out so smoothly in Torque.  Would like to just generate the other eight terrains 
	//based on the propterties of the base terrain in the mission, but it looks like maybe we should just start
	//with all nine in the mission for now.  Come back here if desirable, later.  See if we can base all nine 
	//off the same .ter file so it will still be easy to change resolution etc.
	//for (U32 x=0;x<mTerrainsXCount;x++)
	//{
	//	for (U32 z=0;z<mTerrainsZCount;z++)//"Z" to stay consistent with FlightGear and Unity, which are both Y up.
	//	{
	//		if ((x!=1)||(z!=1))
	//		{
	//			//HMM, well, not this way anyway.  Crash!  Come back to this in a bit.
	//			mTerrains[z][x] = new TerrainBlock();//HERE: either call constructor with (const TerrainBlock &) of [1][1]
	//			mTerrains[z][x]->mSquareSize = mTerrains[1][1]->mSquareSize;
	//			mTerrains[z][x]->mBaseTexSize = mTerrains[1][1]->mBaseTexSize;

	//			mTerrainsLayout[z][x] = mTerrains[z][x];//or set all the relevant properties now if that doesn't work.
	//			Con::printf("Added a terrain block!  %d %d   square size %d",z,x,mTerrainsLayout[z][x]->mSquareSize);
	//			mTerrains[z][x]->setPosition(Point3F(x*mTileWidth,z*mTileWidth,0.0f));
	//		}
	//	}
	//}


void TerrainMaster::processTick()
{
	
	if (mTick++ % mTickInterval == 0)
	{
		findClientPos();

		Con::printf("TERRAIN MASTER TICKING  mStartPos %f %f  clientPos  %f %f,  start long/lat %f %f",
			mStartPos.x,mStartPos.y,mClientPos.x,mClientPos.y,mStartLongitude,mStartLatitude);

		if (mClientPos.len()==0.0)//ditch until we get a valid clientPos, otherwise we are in the time between
			return;//when the terrainmaster gets created and when the player gets created.
		
		//////////  FIRST TIME  /////////////////////////////////////////////////////////
		if (mStartPos.len()==0.0)//Now, this is the first time since we found a player position.
		{
			reloadSkybox();

			//We know we need to reload all terrains, since they come in flat...
			for(int x=0; x<mTerrainsXCount; x++)//First time, we're going to have to load all terrains for sure.
				for(int y=0; y<mTerrainsZCount; y++)
					mLoadTerrains[y][x] = true; 

			//NOW: figure out if we have terrain height/texture files, and if so are they from the right area?
			//First pass, just check for existence, assume it's the right place.
			mTerrainHeightsBinFile = mTerrainPath + "terrain_1_1.heights.bin";
			bool fullRebuild = false;
			//FILE *fp = fopen(mTerrainHeightsBinFile.c_str(),"r+b");
			//if (fp==NULL)
			FileStream fs;
			if (!fs.open(mTerrainHeightsBinFile.c_str(),Torque::FS::File::Read))		
			{ //FIRST TIME - actually I don't need anything here, doing that in onAdd now.
				//Con::printf("heights bin doesn't exist!!!!!!!!  %s",mTerrainHeightsBinFile.c_str());
				mStartLongitude = mPlayerLongitude - (mTileWidthLongitude * 1.5f);//One and a half tiles away, start
				mStartLatitude = mPlayerLatitude - (mTileWidthLatitude * 1.5f);//     your nine-tile square.
				mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
				mStartPos.y = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;
				Con::printf("heights bin doesn't exist!!!!!!!  %s  startPos %f %f  centerLong/Lat  %f %f",
					mTerrainHeightsBinFile.c_str(),mStartPos.x,mStartPos.y,mCenterLongitude,mCenterLatitude);
				fullRebuild = true;
				pingWorldServer(fullRebuild);
			} else {
				//HERE: next step, read the initial arguments and find out if we're in the grid area.
				F32 data;
				fs.setPosition(0*sizeof(float)); fs.read(&data);
				F32 fileLong = data;
				fs.setPosition(1*sizeof(float)); fs.read(&data);
				F32 fileLat = data;
				fs.setPosition(2*sizeof(float)); fs.read(&data);
				F32 fileTileWidth = data;
				fs.setPosition(3*sizeof(float)); fs.read(&data);
				U32 fileHeightmapRes = (U32)data;
				fs.close();//Not the most efficient code in the world, but hey, it's done.
				if ((fileTileWidth!=mTileWidth)||(fileHeightmapRes!=mHeightmapRes))//Unlikely, but this is in case the 
				{//terrains have changed since last time we called the world server.
					Con::printf("terrain bin files have incorrect width/resolution settings!  width %f  heightmapres %d",
						fileTileWidth,fileHeightmapRes);		
					mStartLongitude = mPlayerLongitude - (mTileWidthLongitude * 1.5f);//One and a half tiles away, start
					mStartLatitude = mPlayerLatitude - (mTileWidthLatitude * 1.5f);//     your nine-tile square.
					mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
					mStartPos.y = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;
					fullRebuild=true;
					pingWorldServer(fullRebuild);
				} else if (((mPlayerLongitude - fileLong)<mTileWidthLongitude)&&//This is the 99% of the time normal case,
					((mPlayerLatitude - fileLat)<mTileWidthLatitude))//we are within the bounds of terrain_1_1, so we're good.
				{			
					mStartLongitude = fileLong - mTileWidthLongitude;//One tile width away from the center one is the start one.
					mStartLatitude = fileLat - mTileWidthLatitude;//     your nine-tile square.
					mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
					mStartPos.y = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;
					fullRebuild=false;				
					Con::printf("heights bin all good!!!!!!!   startPos %f %f  centerLong/Lat  %f %f  fileLong/Lat  %f %f",
								mStartPos.x,mStartPos.y,mCenterLongitude,mCenterLatitude,fileLong,fileLat);
				} else {//else we're not, could make this smarter about being in one of the surrounding tiles, but for now, rebuild.
					mStartLongitude = mPlayerLongitude - (mTileWidthLongitude * 1.5f);//One and a half tiles away, start
					mStartLatitude = mPlayerLatitude - (mTileWidthLatitude * 1.5f);//     your nine-tile square.
					mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
					mStartPos.y = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;
					fullRebuild=true;
					Con::printf("heights bin too far away!!!!!!!   startPos %f %f  centerLong/Lat  %f %f  fileLong/Lat  %f %f",
								mStartPos.x,mStartPos.y,mCenterLongitude,mCenterLatitude,fileLong,fileLat);
				}				
			}

			loadTerrainData();
			//SO... this gets the ball rolling.  Now, in process tick, we need to either wait
			//for the terrains and skyboxes to get created, or load them up if they're already there.

			//mTerrainCreating = true;//Either way, this first time we want to call loadTerrainData, and this
			//will trigger it to do so, but wait if there are lockfiles.
			return;
		}
		//////////  END FIRST TIME  /////////////////////////////////////////////////////

		//If a terrain lockfile exists, this means world server is busy creating terrain tiles, so make a note of 
		//it and then do nothing else until it's done.  (Inordinately long time for 256 terrains.)
		FILE *fp = fopen(mTerrainLockfile.c_str(),"r");
		if (fp!=NULL)
		{
			fclose(fp);
			fp = NULL;
			if (!mTerrainCreating)
			{
				Con::printf("Terrain lockfile exists... terrain creating!");
				mTerrainCreating = true;
				return;
			}
			return;//As long as there's any lockfile, just do nothing and let the world server finish.
		} else if (mTerrainCreating) {//here, we were creating, and now the lockfile is gone, so reload.

			if (mClientPos.len()==0.0)
				return;
			
			U32 playerGridX,playerGridZ;
			playerGridX = (int)((mClientPos.x - mStartPos.x) / mTileWidth);
			playerGridZ = (int)((mClientPos.y - mStartPos.y) / mTileWidth);//Keeping it Z to remember that's
			// what it is in flightgear and unity...
			Con::printf("player grid: %d  %d  clientPos  %f  %f  startPos %f %f",
				playerGridX,playerGridZ,mClientPos.x,mClientPos.y,mStartPos.x,mStartPos.y);

			if ((playerGridX<0)||(playerGridX>mTerrainsXCount-1)||(playerGridZ<0)||(playerGridZ>mTerrainsZCount-1))
				return;//error condition, bail before crashing.

			if ((playerGridX==mTerrainsXMid)&&(playerGridZ==mTerrainsZMid))//(1,1)
				return;//should never happen, if we're using the lockfile correctly.
			
			if (playerGridX==0)
				mStartLongitude -= mTileWidthLongitude;
			else if (playerGridX==2)
				mStartLongitude += mTileWidthLongitude;
			if  (playerGridZ==0)
				mStartLatitude -= mTileWidthLatitude;
			else if (playerGridZ==2)
				mStartLatitude += mTileWidthLatitude;

			mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
			mStartPos.z = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;

			TerrainBlock *tempTerrains[3][3];
			if ((playerGridX==0)&&(playerGridZ==0)) {//lower left
				
				tempTerrains[2][2] = mTerrainsLayout[1][1];
				tempTerrains[2][1] = mTerrainsLayout[1][0];
				tempTerrains[1][2] = mTerrainsLayout[0][1];
				tempTerrains[1][1] = mTerrainsLayout[0][0];
				
				//and load the other five from the server.	
				tempTerrains[2][0] = mTerrainsLayout[2][0];
				mLoadTerrains[2][0] = true;				
				tempTerrains[1][0] = mTerrainsLayout[2][1];	
				mLoadTerrains[1][0] = true;					
				tempTerrains[0][0] = mTerrainsLayout[2][2];	
				mLoadTerrains[0][0] = true;					
				tempTerrains[0][1] = mTerrainsLayout[1][2];	
				mLoadTerrains[0][1] = true;					
				tempTerrains[0][2] = mTerrainsLayout[0][2];	
				mLoadTerrains[0][2] = true;	
				
			} else if ((playerGridX==0)&&(playerGridZ==1)) {//mid left
				
				tempTerrains[2][2] = mTerrainsLayout[2][1];
				tempTerrains[1][2] = mTerrainsLayout[1][1];
				tempTerrains[0][2] = mTerrainsLayout[0][1];
				tempTerrains[2][1] = mTerrainsLayout[2][0];
				tempTerrains[1][1] = mTerrainsLayout[1][0];	
				tempTerrains[0][1] = mTerrainsLayout[0][0];	
				
				//and load the other three from the server.	
				tempTerrains[2][0] = mTerrainsLayout[2][2];	
				mLoadTerrains[2][0] = true;						
				tempTerrains[1][0] = mTerrainsLayout[1][2];	
				mLoadTerrains[1][0] = true;					
				tempTerrains[0][0] = mTerrainsLayout[0][2];		
				mLoadTerrains[0][0] = true;	
				
			} else if ((playerGridX==0)&&(playerGridZ==2)) {//upper left
				
				tempTerrains[0][2] = mTerrainsLayout[1][1];
				tempTerrains[0][1] = mTerrainsLayout[1][0];
				tempTerrains[1][2] = mTerrainsLayout[2][1];
				tempTerrains[1][1] = mTerrainsLayout[2][0];	
				
				//and load the other five from the server.	
				tempTerrains[0][0] = mTerrainsLayout[0][0];	
				mLoadTerrains[0][0] = true;					
				tempTerrains[1][0] = mTerrainsLayout[0][1];	
				mLoadTerrains[1][0] = true;					
				tempTerrains[2][0] = mTerrainsLayout[0][2];	
				mLoadTerrains[2][0] = true;					
				tempTerrains[2][1] = mTerrainsLayout[1][2];	
				mLoadTerrains[2][1] = true;					
				tempTerrains[2][2] = mTerrainsLayout[2][2];	
				mLoadTerrains[2][2] = true;	
				
			} else if ((playerGridX==1)&&(playerGridZ==2)) {//upper middle

				tempTerrains[0][0] = mTerrainsLayout[1][0];
				tempTerrains[0][1] = mTerrainsLayout[1][1];	
				tempTerrains[0][2] = mTerrainsLayout[1][2];					
				tempTerrains[1][0] = mTerrainsLayout[2][0];
				tempTerrains[1][1] = mTerrainsLayout[2][1];	
				tempTerrains[1][2] = mTerrainsLayout[2][2];	
				
				//and load the other three from the server.	
				tempTerrains[2][0] = mTerrainsLayout[0][0];	
				mLoadTerrains[2][0] = true;					
				tempTerrains[2][1] = mTerrainsLayout[0][1];		
				mLoadTerrains[2][1] = true;					
				tempTerrains[2][2] = mTerrainsLayout[0][2];	
				mLoadTerrains[2][2] = true;		
				
			} else if ((playerGridX==2)&&(playerGridZ==2)) {//upper right
				
				tempTerrains[0][0] = mTerrainsLayout[1][1];
				tempTerrains[0][1] = mTerrainsLayout[1][2];
				tempTerrains[1][0] = mTerrainsLayout[2][1];
				tempTerrains[1][1] = mTerrainsLayout[2][2];
				
				//and load the other five from the server.	
				tempTerrains[0][2] = mTerrainsLayout[0][2];	
				mLoadTerrains[0][2] = true;					
				tempTerrains[1][2] = mTerrainsLayout[0][1];	
				mLoadTerrains[1][2] = true;					
				tempTerrains[2][2] = mTerrainsLayout[0][0];	
				mLoadTerrains[2][2] = true;					
				tempTerrains[2][1] = mTerrainsLayout[1][0];	
				mLoadTerrains[2][1] = true;					
				tempTerrains[2][0] = mTerrainsLayout[2][0];	
				mLoadTerrains[2][0] = true;	
				
			} else if ((playerGridX==2)&&(playerGridZ==1)) {//mid right
				
				tempTerrains[2][0] = mTerrainsLayout[2][1];
				tempTerrains[1][0] = mTerrainsLayout[1][1];
				tempTerrains[0][0] = mTerrainsLayout[0][1];
				tempTerrains[2][1] = mTerrainsLayout[2][2];
				tempTerrains[1][1] = mTerrainsLayout[1][2];	
				tempTerrains[0][1] = mTerrainsLayout[0][2];		
				
				//and load the other three from the server.	
				tempTerrains[2][2] = mTerrainsLayout[2][0];
				mLoadTerrains[2][2] = true;
				tempTerrains[1][2] = mTerrainsLayout[1][0];
				mLoadTerrains[1][2] = true;					
				tempTerrains[0][2] = mTerrainsLayout[0][0];		
				mLoadTerrains[0][2] = true;
				
			} else if ((playerGridX==2)&&(playerGridZ==0)) {//lower right
				
				tempTerrains[2][0] = mTerrainsLayout[1][1];
				tempTerrains[2][1] = mTerrainsLayout[1][2];
				tempTerrains[1][0] = mTerrainsLayout[0][1];
				tempTerrains[1][1] = mTerrainsLayout[0][2];	
				
				//and load the other five from the server.	
				tempTerrains[2][2] = mTerrainsLayout[2][2];	
				mLoadTerrains[2][2] = true;					
				tempTerrains[1][2] = mTerrainsLayout[2][1]; 	
				mLoadTerrains[1][2] = true;					
				tempTerrains[0][2] = mTerrainsLayout[2][0];	
				mLoadTerrains[0][2] = true;							
				tempTerrains[0][1] = mTerrainsLayout[1][0];	
				mLoadTerrains[0][1] = true;					
				tempTerrains[0][0] = mTerrainsLayout[0][0];	
				mLoadTerrains[0][0] = true;			
				
			} else if ((playerGridX==1)&&(playerGridZ==0)) {//lower middle
				
				tempTerrains[2][0] = mTerrainsLayout[1][0];
				tempTerrains[2][1] = mTerrainsLayout[1][1];	
				tempTerrains[2][2] = mTerrainsLayout[1][2];					
				tempTerrains[1][0] = mTerrainsLayout[0][0];
				tempTerrains[1][1] = mTerrainsLayout[0][1];	
				tempTerrains[1][2] = mTerrainsLayout[0][2];	
				
				//and load the other three from the server.	
				tempTerrains[0][0] = mTerrainsLayout[2][0];	
				mLoadTerrains[0][0] = true;
				tempTerrains[0][1] = mTerrainsLayout[2][1];
				mLoadTerrains[0][1] = true;
				tempTerrains[0][2] = mTerrainsLayout[2][2];		
				mLoadTerrains[0][2] = true;			
			} 
			
			//Now, copy all the new positions back into our mTerrainsLayout array, so we're 
			for(int i=0; i<mTerrainsXCount; i++)//good to go for next time.
				for(int j=0; j<mTerrainsZCount; j++)
					mTerrainsLayout[j][i] = tempTerrains[j][i];
					
			//for(int i=0; i<mTerrainsXCount; i++)//Now, do the actual load and move on the modified terrains.
			//{
			//	for(int j=0; j<mTerrainsZCount; j++)			
			//	{
			//		if (mLoadTerrains[j][i])
			//		{
			//			//Debug.Log("Reload terrain ["+j+","+i+"]:  " + mTerrainsLayout[j][i].transform.position);
			//		} else {
			//			//Debug.Log("Existing terrain ["+j+","+i+"]: " + mTerrainsLayout[j][i].transform.position);
			//		}
			//	}
			//}	

			loadTerrainData();
			mTerrainCreating = false;
			return;
		}

		fp = fopen(mSkyboxLockfile.c_str(),"r");
		if (fp!=NULL)
		{
			fclose(fp);
			fp = NULL;
			if (!mSkyboxCreating)
			{
				Con::printf("Skybox lockfile exists.");
				mSkyboxCreating = true;
				return;
			}
			return;//As long as there's any lockfile, just do nothing and let the world server finish.
		} else if (mSkyboxCreating) { //Now, if mSkyboxCreating and no lockfile, reload and reset.
			reloadSkybox();
			mSkyboxCreating = false;
		}
	}
}


void TerrainMaster::loadTerrainData()
{
	Con::printf("Loading terrain data!");
	//int array_size = (mHeightmapRes * mHeightmapRes) + mNumHeightBinArgs;
	for(int x=0; x<mTerrainsXCount; x++)//horizontal
	{
		for(int y=0; y<mTerrainsZCount; y++)//vertical
		{
			if (mLoadTerrains[y][x])
			{
				F32 data;
				char heightsfilename[255],texturesfilename[255];
				sprintf(heightsfilename,"terrain_%d_%d.heights.bin",y,x);
				sprintf(texturesfilename,"terrain_%d_%d.textures.bin",y,x);
				mTerrainHeightsBinFile = mTerrainPath + heightsfilename;
				mTerrainTexturesBinFile = mTerrainPath + texturesfilename;
				
				//FILE *fp = fopen(mTerrainHeightsBinFile.c_str(),"r+b");
				FileStream fs;
				if (fs.open(mTerrainHeightsBinFile.c_str(),Torque::FS::File::Read))
				{
					fs.setPosition(0*sizeof(float)); fs.read(&data);
					F32 fileLong = data;
					fs.setPosition(1*sizeof(float)); fs.read(&data);
					F32 fileLat = data;
					fs.setPosition(2*sizeof(float)); fs.read(&data);
					F32 fileTileWidth = data;
					fs.setPosition(3*sizeof(float)); fs.read(&data);
					U32 fileHeightmapRes = (U32)data;
					fs.setPosition(4*sizeof(float)); fs.read(&data);
					U32 fileTextureRes = (U32)data;
					if (fileHeightmapRes!=mHeightmapRes)
					{
						Con::printf("Wrong heightmap resolution in file: %s",heightsfilename);
						for(int i=0; i<mTerrainsXCount; i++)//First time, we're going to have to load all terrains for sure.
							for(int j=0; j<mTerrainsZCount; j++)
								mLoadTerrains[j][i] = true; 
						pingWorldServer(true);
						return;
					}
					Con::printf("Loading terrain data from:  %s, long/lat: %f %f  heightmapres %d  tileWidth %f",
									heightsfilename,fileLong,fileLat,fileHeightmapRes,mTileWidth);
					for (U32 xx=0;xx<mHeightmapRes;xx++)
					{	
						for (U32 yy=0;yy<mHeightmapRes;yy++)
						{
							fs.setPosition((yy*mHeightmapRes*sizeof(float)) + (xx*sizeof(float)) + (mNumHeightBinArgs*sizeof(float)) );
							fs.read(&data);
							mTerrainsLayout[y][x]->setHeight(Point2I(xx,yy),data);					
						}
					}
					fs.close();

					//Now, set terrain position to where it should be based on our previously determined startPos.
					Point3F terrPos(mStartPos.x+((float)(x)*(mTileWidth-mSquareSize)),mStartPos.y+((float)(y)*(mTileWidth-mSquareSize)),0.0);
					//Hmm, not sure if it's a bug in Torque or not, but WorldBlockSize should be 2550 should it not?  Because
					//the last data point is shared with the next terrain, it doesn't have a square of its own.
					mTerrainsLayout[y][x]->setPosition(terrPos);
				}
				
				//Here, not sure if it will help to do this twice, but it seems like once isn't enough.  Maybe another function...
				mTerrainsLayout[y][x]->updateGrid(Point2I(0,0),
					Point2I(mTerrainsLayout[y][x]->getBlockSize()-1,mTerrainsLayout[y][x]->getBlockSize()-1),true);

				U8 texData;
				if (fs.open(mTerrainTexturesBinFile.c_str(),Torque::FS::File::Read))
				{
					TerrainFile *file = mTerrainsLayout[y][x]->getFile();
					for (U32 xx=0;xx<mTextureRes;xx++)
					{	
						for (U32 yy=0;yy<mTextureRes;yy++)
						{
							fs.setPosition((yy*mTextureRes*sizeof(U8)) + (xx*sizeof(U8)));
							fs.read(&texData);
							U8 index = texData - 1;//Flightgear puts out 1-based numbers, here we are 0-based.
							if ((index>=0)&&(index<=2))
								file->setLayerIndex( xx, yy, index );
						}
					}
					fs.close();
				}

				//Here, second time's a charm?
				mTerrainsLayout[y][x]->updateGrid(Point2I(0,0),
					Point2I(mTerrainsLayout[y][x]->getBlockSize()-1,mTerrainsLayout[y][x]->getBlockSize()-1),true);



				//} else Con::errorf("ERROR: can't open terrain heights bin file: %s",mTerrainHeightsBinFile.c_str());
			}
		}
	}
}

void TerrainMaster::pingWorldServer(bool fullRebuild)
{
	//HERE: Now that we have a working socket, go ahead and find all the necessary player 
	//and terrain information, and transmit it.
	
	findClientPos();//This searches through available players and cameras and determines whether we
	//have a first person controller or a free camera, and sets mClientPos and mFreeCamera as appropriate.

	if (mClientPos.len()==0.0)
	{
		Con::printf("client pos  =	0 0 0 !!!!!");//On real terrain (0,0,0) shouldn't be possible, is error.
		return;//If an error condition results in clientPos = (0,0,0), bail.
	}

	mPlayerLongitude = mCenterLongitude + (mClientPos.x * mDegreesPerMeterLong);
	mPlayerLatitude = mCenterLatitude + (mClientPos.y * mDegreesPerMeterLat);
	mPlayerElevation = mClientPos.z + mMinElev + 30.0f;//get 30m above ground for better skybox.

	if (mStartLongitude==0.0)
	{//Should not happen...
		mStartLongitude = mPlayerLongitude - (mTileWidthLongitude * 1.5f);
		mStartLatitude = mPlayerLatitude - (mTileWidthLatitude * 1.5f);
	}
	/////////////////////////
	//Now, we know we have a valid clientPos, so let's go ahead and talk to the world server.
	struct sockaddr_in serv_addr;
    //struct hostent *server;//apparently unnecessary, what with inet_addr() function & all.
	
	const U32 packetsize = 1024;//Turns out this isn't necessary as an actual packet size, TCP handles that.
	unsigned char *buffer;//[packetsize];//I do need a number for my input buffer size though.
	//char *buffer;
	float *floatBuffer;
	const int num_args = 11;
	floatBuffer = new float[11];
	buffer = new unsigned char[packetsize];
	//mPlayerElevation = mTileWidth;
	floatBuffer[0] = mPlayerLongitude;
	floatBuffer[1] = mPlayerLatitude;
	floatBuffer[2] = mPlayerElevation;
	floatBuffer[3] = mTileWidth;
	floatBuffer[4] = (float)mHeightmapRes;
	floatBuffer[5] = (float)mTextureRes;
	floatBuffer[6] = mTileWidthLongitude;
	floatBuffer[7] = mTileWidthLatitude;
	floatBuffer[8] = mStartLongitude;
	floatBuffer[9] = mStartLatitude;
	floatBuffer[10] = (float)fullRebuild;
	
	Con::printf("sent to server: %f %f %f %f %f %f %f %f %f %f %f",floatBuffer[0],floatBuffer[1],
		floatBuffer[2],floatBuffer[3],floatBuffer[4],floatBuffer[5],floatBuffer[6],floatBuffer[7],
		floatBuffer[8],floatBuffer[9],floatBuffer[10]);

	sockfd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
    if (sockfd < 0) 
        Con::printf("ERROR opening socket");
 //   server = gethostbyname(mWorldServerIP.c_str());
	//Con::printf("called for server %s, got %s",mWorldServerIP.c_str(),server->h_name);
	////server = getaddrinfo();
 //   if (server == NULL) {
 //       Con::printf("ERROR, no such host\n");
 //       exit(0);
 //   }
    //bzero((char *) &serv_addr, sizeof(serv_addr));
	ZeroMemory((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	//serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;//BINGO!
	serv_addr.sin_addr.s_addr = inet_addr( mWorldServerIP.c_str() );
    serv_addr.sin_port = htons(mWorldServerPort);

	Con::printf("server address:  %s  port %d",mWorldServerIP.c_str(),mWorldServerPort);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
        Con::printf("ERROR connecting");
		return;
	}
	mTerrainPath = "C:/HardReality/projects/T3DTerrainMaster/T3DTerrainMaster/Templates/Full/game/art/terrains/WorldServer/";
	//strcpy(&buffer[0], reinterpret_cast<char*>(floatBuffer));//,
	buffer = reinterpret_cast<unsigned char*>(floatBuffer);
	char charBuffer[packetsize];
	strcpy(charBuffer,mTerrainPath.c_str());
	memcpy(&buffer[num_args * sizeof(float)],(void *)charBuffer,strlen(charBuffer)+1);
	//buffer[num_args * sizeof(float)] = (unsigned char *)mTerrainPath.c_str();
	//for (U32 i=0;i<num_args;i++) 
	//	Con::printf("bytes: %d %d %d %d",buffer[(i*4)],buffer[(i*4)+1],buffer[(i*4)+2],buffer[(i*4)+3]);		
	
	//int n = send(sockfd,buffer,sizeof(float)*num_args,0);
    int n = send(sockfd,(const char *)buffer,packetsize,0);
	closesocket(sockfd);
	delete floatBuffer;
}


void TerrainMaster::interpolateTick(F32 f)
{
	//Need this because virtual in parent class.
}

void TerrainMaster::advanceTime(F32 f)
{
	//Need this because virtual in parent class.
}

void TerrainMaster::setTerrainHeights(S32 start_x,S32 start_y,S32 sample)	
{
	if (!mCurrentTerrain)
	{
		SimSet* scopeAlwaysSet = Sim::getGhostAlwaysSet();
		for(SimSet::iterator itr = scopeAlwaysSet->begin(); itr != scopeAlwaysSet->end(); itr++)
		{
			TerrainBlock* block = dynamic_cast<TerrainBlock*>(*itr);
			if( block )
				mCurrentTerrain = block;
		}
	}
	if (!mCurrentTerrain) Con::printf("couldn't find my terrain!");

	if (!mCurrentTerrain) return;

	Con::printf("Terrain Master:  matrixFile %s  minHeight %f  maxHeight %f  numRows %d blocksize: %d  file %s",
		mMatrixFile.c_str(),mMinHeight,mMaxHeight,mNumRows,mCurrentTerrain->getBlockSize(),mMatrixFile.c_str());

	FileStream fs;
	char filename[512];

	if (fs.open(mMatrixFile.c_str(),Torque::FS::File::Read))
	{
		Con::printf("Opened the matrix file: %s",mMatrixFile.c_str());
		//char filename[255];
		//sprintf(filename,"%s",mMatrixFile.c_str());
		//FILE *fp = fopen(mMatrixFile.c_str(),"r+b");
		//if (fp==NULL) Con::errorf("ERROR: can't open terrain matrix file: %s",mMatrixFile.c_str());
		//else
		//{
		if ((start_y < (mNumRows - mCurrentTerrain->getBlockSize())) &&
			 (start_x < (mNumColumns - mCurrentTerrain->getBlockSize())))
		{
			for (U32 i=0;i<mCurrentTerrain->getBlockSize();i++)
			{	
				for (U32 j=0;j<mCurrentTerrain->getBlockSize();j++)
				{
					//Heh, oops, everything has to be multiplied by four because we're dealing with raw bytes,
					fs.setPosition(((start_y+j)*mNumColumns*4) + ((start_x+i)*4) );//not floats.
					F32 data;
					fs.read(&data);
					mCurrentTerrain->setHeight(Point2I(i,j),data);
					//Q: does it advance the cursor automatically, or do you have to call setPosition?
					//Con::printf("value: %f",data);
				}
			}
			mCurrentTerrain->updateGrid(Point2I(0,0),
				Point2I(mCurrentTerrain->getBlockSize()-1,mCurrentTerrain->getBlockSize()-1),true);

			Con::printf("saving terrain: %s",filename);
			sprintf(filename,"%s/%d_%d.ter",mOutputDirectory.c_str(),start_x,start_y);
			mCurrentTerrain->save(filename);//FIX - get MatrixFile minus the .bin extension
		} else {
			Con::printf("Terrain Matrix starting position (%d,%d) is out of range, numRows %d, numColumns %d",
				start_x,start_y,mNumRows,mNumColumns);
		}
		fs.close();
	}

   return;
}


//OBSOLETE
void TerrainMaster::loadTerrain(Point3F pos,Point2I grid)
{
	//NOW, given the naming convention we've adopted, and the inputDirectory variable we've assigned,
	//   all we have to do is look in the right directory for "(pos.x/10)_(pos.y/10).ter".

	//BUT FIRST: do the safety checking here, so you don't have to do it a million times in the
	//   functions that call this function. 
	S32 index = (grid.y*mNumGridColumns)+grid.x;
	if ( (mActiveTerrains[index]!=NULL) || (mLoadingTerrains[index]) ||
		(grid.x<0)||(grid.y<0)||(grid.x>=mNumGridColumns)||(grid.y>=mNumGridRows))
		return;

	char filename[255],txtPos[255];
	S32 x = (S32)(pos.x / mSquareSize);
	S32 y = (S32)(pos.y / mSquareSize);
	sprintf(filename,"%s/%d_%d.ter",mInputDirectory.c_str(),x,y);
	sprintf(txtPos,"%f %f %f",pos.x,pos.y,pos.z);
	Con::printf("calling out to script for terrain, pos %5.0f %5.0f  loaded %d",
						pos.x,pos.y,mLoadedTerrains.size());
	Con::executef("loadTerrainBlock",filename,txtPos);
	//mActiveTerrains[(grid.y*mNumGridColumns)+grid.x] = mCurrentTerrain;//FIX - need pointer to new terrain, but we just 
	//called out to script, it's going to be a while.  Move the terrain creation to here and then grab the terrain.
	//For now this makes it non null, which is all we need.
}

void TerrainMaster::reloadSkybox()
{
	//HERE:  The one big trick to making skyboxes work in Torque is flipping them all around so they match the cubemap 
	//configuration with the sky in the center and other four images aligned around it, so sky is always "touching".
	//        up
	//        |N
	//    W   v   E
	// up -> SKY <- up
	//        ^
	//        |S
	//        up
	FileName skyboxes[10];//first five for input, second five for output. skybox3_nn vs skybox_nn for first pass.
	//Order is:  up, 00, 90, 180, 270
	skyboxes[0] = mTerrainPath + "skybox3_up.png";
	//skyboxes[0] = mTerrainPath + "SouthernWillamette.jpg";
	skyboxes[1] = mTerrainPath + "skybox3_00.png";
	skyboxes[2] = mTerrainPath + "skybox3_90.png";
	skyboxes[3] = mTerrainPath + "skybox3_180.png";
	skyboxes[4] = mTerrainPath + "skybox3_270.png";
	skyboxes[5] = mTerrainPath + "skybox_up.png";
	skyboxes[6] = mTerrainPath + "skybox_00.png";
	skyboxes[7] = mTerrainPath + "skybox_90.png";
	skyboxes[8] = mTerrainPath + "skybox_180.png";
	skyboxes[9] = mTerrainPath + "skybox_270.png";
	FileStream fs;
	U32 skyOffset = 20;//Amount we raise all side panels (to compensate for camera height?)
	GBitmap *bitmap = new GBitmap;
	GBitmap *bitmap2 = new GBitmap;
	//We're going to do very different things per bitmap, so not bothering to loop this.
	if (fs.open(skyboxes[0].c_str(),Torque::FS::File::Read))// UP
	{
		bitmap->readBitmap("png",fs);
		bitmap2->allocateBitmap(bitmap->getWidth(),bitmap->getHeight());
		fs.close();
		for (U32 x=0;x<bitmap->getWidth();x++) {
			for (U32 y=0;y<bitmap->getHeight();y++) {
				ColorI color;
				bitmap->getColor(x,y,color);
				//bitmap2->setColor(x,y,color);//Direct copy
				bitmap2->setColor(x,(bitmap->getHeight()-1)-y,color);//Flip vertically
			}
		}
		if (fs.open(skyboxes[5].c_str(),Torque::FS::File::Write))
		{
			bitmap2->writeBitmap("png",fs);
			fs.close();
		}
	}
	if (fs.open(skyboxes[1].c_str(),Torque::FS::File::Read))// NORTH  00
	{
		bitmap->readBitmap("png",fs);
		fs.close();
		for (U32 x=0;x<bitmap->getWidth();x++) {
			for (U32 y=0;y<bitmap->getHeight();y++) {
				ColorI color;
				U32 newY = y+skyOffset;
				if (newY>bitmap->getHeight()) newY=bitmap->getHeight();
				bitmap->getColor(x,newY,color);
				bitmap2->setColor(x,(bitmap->getHeight()-1)-y,color);//Flip vertically
			}
		}
		if (fs.open(skyboxes[6].c_str(),Torque::FS::File::Write))
		{
			bitmap2->writeBitmap("png",fs);
			fs.close();
		}
	}
	if (fs.open(skyboxes[2].c_str(),Torque::FS::File::Read))// EAST  90
	{
		bitmap->readBitmap("png",fs);
		fs.close();
		for (U32 x=0;x<bitmap->getWidth();x++) {
			for (U32 y=0;y<bitmap->getHeight();y++) {
				ColorI color;
				U32 newY = y+skyOffset;
				if (newY>bitmap->getHeight()) newY=bitmap->getHeight();
				bitmap->getColor(x,newY,color);
				bitmap2->setColor(y,x,color);//Flip diagonally, across axis from upper left to lower right.
			}
		}
		if (fs.open(skyboxes[7].c_str(),Torque::FS::File::Write))
		{
			bitmap2->writeBitmap("png",fs);
			fs.close();
		}
	}
	if (fs.open(skyboxes[3].c_str(),Torque::FS::File::Read))// SOUTH  180
	{
		bitmap->readBitmap("png",fs);
		fs.close();
		for (U32 x=0;x<bitmap->getWidth();x++) {
			for (U32 y=0;y<bitmap->getHeight();y++) {
				ColorI color;
				U32 newY = y+skyOffset;
				if (newY>bitmap->getHeight()) newY=bitmap->getHeight();
				bitmap->getColor(x,newY,color);
				bitmap2->setColor((bitmap->getWidth()-1)-x,y,color);//Flip horizontally
			}
		}
		if (fs.open(skyboxes[8].c_str(),Torque::FS::File::Write))
		{
			bitmap2->writeBitmap("png",fs);
			fs.close();
		}
	}
	if (fs.open(skyboxes[4].c_str(),Torque::FS::File::Read))// WEST  270
	{
		bitmap->readBitmap("png",fs);
		fs.close();
		for (U32 x=0;x<bitmap->getWidth();x++) {
			for (U32 y=0;y<bitmap->getHeight();y++) {
				ColorI color;
				U32 newY = y+skyOffset;
				if (newY>bitmap->getHeight()) newY=bitmap->getHeight();
				bitmap->getColor(x,newY,color);
				bitmap2->setColor((bitmap->getHeight()-1)-y,(bitmap->getWidth()-1)-x,color);//Flip diagonally, across axis from lower left to upper right.
			}
		}
		if (fs.open(skyboxes[9].c_str(),Torque::FS::File::Write))
		{
			bitmap2->writeBitmap("png",fs);
			fs.close();
		}
	}
	Con::executef("WorldServerReloadSkybox");
	//WS_SkyboxCubemap.updateFaces();
}

void TerrainMaster::setClientPos(Point3F pos)
{
	mClientPos.set(pos);
	//if (!mSQL) return;

	//char query[512];
	//int id,result;

	//Holding off on this for now, was saving bad data, (0,0,512) somehow.
	//sprintf(query,"UPDATE clientConnection SET start_x=%f,start_y=%f,start_z=%f WHERE username='%s';",
	//	pos.x,pos.y,pos.z,mClientUsername.c_str());
	//result = mSQL->ExecuteSQL(query);
	//Con::printf(" clientConnection pos: %f %f %f for username %s",pos.x,pos.y,pos.z,mClientUsername.c_str());
}

void TerrainMaster::findClientPos()
{
	Vector<SceneObject*> kCameras;
	Vector<SceneObject*> kPlayers;

	Box3F bounds;
	bounds.set(Point3F(-1024000,-1024000,0),Point3F(1024000,1024000,10000));
	gServerContainer.findObjectList(bounds, CameraObjectType, &kCameras);
	gServerContainer.findObjectList(bounds, PlayerObjectType, &kPlayers);

	Con::printf("seeking client pos... cameras %d  players %d",kCameras.size(),kPlayers.size());
	mClientPos.zero();
	for (U32 i=0;i<kPlayers.size();i++)
	{
		Player *myPlayer = (Player *)(kPlayers[i]);
		Point3F playerPos = myPlayer->getPosition();
		if (kCameras.size()>0)
		{
			Camera *myCamera = dynamic_cast<Camera *>(kCameras[i]);//... sort out which belongs to controlling client.
			Point3F cameraPos = myCamera->getPosition();
			GameConnection *cameraClient = myCamera->getControllingClient();
			GameConnection *playerClient = myPlayer->getControllingClient();
			if (cameraClient) 
			{
				mFreeCamera = true;
				mClientPos = cameraPos;
			} else if (playerClient) {
				mFreeCamera = false;
				mClientPos = playerPos;
			} 
		} else {
			mClientPos = playerPos;
		}
	} 
	if (mClientPos.len()>0.0)
	{
		mPlayerLongitude = mCenterLongitude + (mClientPos.x * mDegreesPerMeterLong);
		mPlayerLatitude = mCenterLatitude + (mClientPos.y * mDegreesPerMeterLat);
	}
}

ConsoleMethod(TerrainMaster, getBlockSize, S32, 2, 2, "getBlockSize()")
{
	if (object->mCurrentTerrain)
		return object->mCurrentTerrain->getBlockSize();

	return 0;
}

ConsoleMethod(TerrainMaster, showTerrainBlockDetails,void, 2, 2, "")
{
   SimSet* scopeAlwaysSet = Sim::getGhostAlwaysSet();
   for(SimSet::iterator itr = scopeAlwaysSet->begin(); itr != scopeAlwaysSet->end(); itr++)
   {
      TerrainBlock* block = dynamic_cast<TerrainBlock*>(*itr);
      if( block )
         object->mCurrentTerrain = block;
   }
	if (object->mCurrentTerrain)
	{
		Con::printf("Terrain Master found a current terrain.  worldblocksize = %f",
			object->mCurrentTerrain->getWorldBlockSize());
	}
   return;
}

ConsoleMethod(TerrainMaster, setTerrainHeights, void, 4, 5, "setTerrainBlockHeights(int start_x,int start_y,int sample=1)")
{
	U32 start_x = dAtoi(argv[2]);
	U32 start_y = dAtoi(argv[3]);
	U32 sample = 1;
	if (argc==5)
		sample = dAtoi(argv[4]);

	object->setTerrainHeights(start_x,start_y,sample);

	return;
}

//ConsoleMethod(TerrainMaster, checkTerrain,void, 2, 2, "")
//{
	//object->checkTerrain();
//}

ConsoleMethod(TerrainMaster, ping, void, 2, 3, "ping(bool) - pings world server, with fullRebuild or not")
{
	bool rebuild=false;
	if (argc==3) 
		rebuild = dAtob(argv[2]);
	object->pingWorldServer(rebuild);
}

ConsoleMethod(TerrainMaster, reloadSkybox, void, 2, 2, "")
{
	object->reloadSkybox();
}

ConsoleMethod(TerrainMaster, getGridRow,S32, 2, 2, "")
{
	return object->mCurrentGridRow;
}

ConsoleMethod(TerrainMaster, getGridColumn,S32, 2, 2, "")
{
	return object->mCurrentGridColumn;
}

ConsoleMethod(TerrainMaster, getClientPos,const char *, 2, 2, "getClientPos()")//first pass assumes only one client
{
	char* ret = Con::getReturnBuffer(256);
	Point3F pos = object->mClientPos;
	dSprintf(ret, 255, "%g %g %g",pos.x,pos.y,pos.z);
	return ret;
}

ConsoleMethod(TerrainMaster, setClientPos,void, 3, 3, "setClientPos(Point3F)")//first pass assumes only one client
{
	Point3F pos;
	dSscanf(argv[2],"%g %g %g",&pos.x,&pos.y,&pos.z);
	object->setClientPos(pos);
}

ConsoleMethod(TerrainMaster, setStatic,void, 3, 3, "setStatic(S32)")
{
	S32 id;
	id = dAtoi(argv[2]);
	Con::printf("trying to set static id=%d",id);
	if ((id>=0)&&(id<TM_MAX_STATICS))
	{
		object->mStatics[id]=true;
		Con::printf("terrain master set static: %d",id);
	}
}

/*
void TerrainMaster::checkTerrain()
{
	//NetConnection * toClient = NetConnection::getLocalClientConnection();

	//S32 index = toClient->getGhostIndex(obj);
	//NetObject *obj = toClient->getScopeObject();//DAMMIT TORQUE!  Found just the func I need, but it doesn't actually exist. :-\
	//SceneObject *obj = dynamic_cast<SceneObject *>(toClient->getScopeObject());

	Vector<SceneObject*> kCameras;
	Vector<SceneObject*> kPlayers;
	Vector<SceneObject*> kTerrains;

	Box3F bounds;
	bounds.set(Point3F(0,0,0),Point3F(1024000,1024000,10000));
	gServerContainer.findObjectList(bounds, CameraObjectType, &kCameras);
	gServerContainer.findObjectList(bounds, PlayerObjectType, &kPlayers);
	gServerContainer.findObjectList(bounds, TerrainObjectType, &kTerrains);

	Point3F clientPos;
	clientPos.zero();

	//Now, determine whether we are free camera or first person/third person mode, use appropriate position.
	bool cameraInControl = false;
	bool playerInControl = false;

	//But meanwhile, the player is actually the player, player's camera doesn't show up as a camera.

	//if (kPlayers.size()==1)//FIX!  This will fail as soon as there are bots.  
	//{                      //Need to find only players WITH controlling clients.

	//Vector<Point3F> clientPosVec; 

	for (U32 i=0;i<kPlayers.size();i++)
	{

		Player *myPlayer = (Player *)(kPlayers[i]);
		Point3F playerPos = myPlayer->getPosition();
		if (kCameras.size()>0)
		{
			Camera *myCamera = dynamic_cast<Camera *>(kCameras[i]);//... sort out which belongs to controlling client.
			Point3F cameraPos = myCamera->getPosition();


			//clientPosVec.increment();
			GameConnection *cameraClient = myCamera->getControllingClient();
			GameConnection *playerClient = myPlayer->getControllingClient();
			if (cameraClient) 
			{
				cameraInControl = true;
				clientPos = cameraPos;
				//clientPosVec.last() = cameraPos;
			} else if (playerClient) {
				playerInControl = true;
				clientPos = playerPos;
				//clientPosVec.last() = playerPos;
			} 
		} else {
			clientPos = playerPos;
		}
	} 
	if (clientPos.len()==0.0)
	{
		Con::printf("client pos  =	0 0 0 !!!!!");
		return;//If an error condition results in clientPos = (0,0,0), bail.
	}
	//clientPos = clientPosVec[0];//TEMP!  Need to do loop below for each clientPos, but for now just do it for 
	//the first.
	//////////////////////////////////////////////////////
	//Con::printf("client pos: %f %f %f  cameraInControl: %d numCameras %d",
	//		clientPos.x,clientPos.y,clientPos.z,cameraInControl,kCameras.size());

	F32 blockSize = mTileWidth - mSquareSize;
	if (!mCurrentTerrain)
	{
		mCurrentGridColumn = (S32)(clientPos.x/blockSize);
		mCurrentGridRow = (S32)(clientPos.y/blockSize);
		if (!mFirstTerrain)
		{//HERE: we are on our first pass, we don't even have any terrains loaded, so load up a whole block around the player.

			Point2I grid(mCurrentGridColumn,mCurrentGridRow);
			Point3F pos;

			//CENTER -- Now, with new NeighborTerrain strategy, we are going to start with the center again...
			pos.set(grid.x*(blockSize),grid.y*(blockSize),0.0);
			loadTerrain(pos,grid); 
			mLoadingTerrains[(grid.y*mNumGridColumns)+grid.x] = true;
			//Only this time, we are going to load just this tile on this pass, and when we come back we will start
			//from the idea that we have a tile loaded and it is on an edge, so check its neighbors. 

			mFirstTerrain = true;

		} else {//HERE: now we are on our second pass, first terrain should be loaded by now, but mCurrentTerrain not yet assigned to it.

			//Now, with the NeighborTerrain method, we should only have one terrain loaded at this stage.
			if (kTerrains.size() != 1)
			{
				Con::errorf("More or less than one terrain loaded in first stage checkTerrains %d.",kTerrains.size());
				return;
			}

			TerrainBlock *terrain = (TerrainBlock*)(kTerrains[0]);
			terrain->mIsEdge = true;
			Point3F terrPos = terrain->getPosition();
			for (U32 j=0;j<8;j++)
				terrain->mNeighborTerrainPos[j] = terrPos + Point3F(mNeighTerrAdj[j].x*blockSize,mNeighTerrAdj[j].y*blockSize,0);

			S32 terrGridColumn = (S32)(terrPos.x/(mTileWidth-mSquareSize));
			S32 terrGridRow = (S32)(terrPos.y/(mTileWidth-mSquareSize));		
			S32 index = (terrGridRow*mNumGridColumns)+terrGridColumn;
			mActiveTerrains[index] = terrain;
			mLoadingTerrains[index] = false;
			mLoadedTerrains.increment();//This list gets added to and deleted from as we add and drop terrains.
			mLoadedTerrains.last() = terrain;
			if ((mCurrentGridColumn==terrGridColumn)&&(mCurrentGridRow==terrGridRow))
			{
				mCurrentTerrain = terrain; //now, with this loaded we won't come back here, 
				//from now on it's up to the regular loop to find new neighbors.
			}
		}
		return;
	}


	/////////////////////////
	//checkEntities(clientPos,500.0);//(pos,detectRange)
	/////////////////////////

	//Okay, first thing: let's check to see if we have any terrains to load, if so do that.
	for (U32 i=0;i<mLoadedTerrains.size();i++)
	{
		if (mLoadedTerrains[i]->mIsEdge)
		{
			for (U32 j=0;j<8;j++)
			{
				if (mLoadedTerrains[i]->mNeighborTerrainLoaded[j]==false)
				{
					Point3F diff = clientPos - (mLoadedTerrains[i]->mNeighborTerrainPos[j]);
					if (diff.len()<mLoadDistance)
					{
						Point3F pos = mLoadedTerrains[i]->mNeighborTerrainPos[j];
						Point2I grid = Point2I((S32)(pos.x/blockSize),(S32)(pos.y/blockSize));
						S32 index = (grid.y*mNumGridColumns)+grid.x;
						if (!mLoadingTerrains[index] && !mActiveTerrains[index] )
						{
							//Con::printf("loading terrain neighbor at: %d %d  diff len %f",grid.x,grid.y,diff.len());
							loadTerrain(pos,grid); 
							mLoadingTerrains[index] = true;
						}
						//mLoadedTerrains[i]->mNeighborTerrainLoaded[j] = true;
					}
				}
			}
		}
	}

	//Second thing:  lets run through kTerrains, and make sure that mActiveTerrains and mLoadedTerrains are up to date.
	//Point3F terrainPos = mCurrentTerrain->getPosition();
	for (U32 i=0;i<kTerrains.size();i++)
	{
		TerrainBlock *terrain = (TerrainBlock*)(kTerrains[i]);
		bool isThisTerrain = (terrain==mCurrentTerrain);
		Point3F terrPos = terrain->getPosition();
		Point2I grid = Point2I((S32)(terrPos.x/blockSize),(S32)(terrPos.y/blockSize));
		S32 index = (grid.y*mNumGridColumns)+grid.x;
		//Con::printf("terrain %d %d found:  position %f %f %f  isCurrentTerrain %d dropDist %f",
		//	gridX,gridY,thisTerrainPos.x,thisTerrainPos.y,thisTerrainPos.z,isThisTerrain,mDropDistance);	
		Point3F diff = terrain->getPosition() - clientPos;
		if ((mActiveTerrains[index]==NULL) &&
			(diff.len() < mLoadDistance))
		{
			//Con::printf("found a new ActiveTerrain: %d %d",grid.x,grid.y); 
			mActiveTerrains[index] = terrain;
			mLoadingTerrains[index] = false;
			mLoadedTerrains.increment();
			mLoadedTerrains.last() = terrain;
			for (U32 j=0;j<8;j++)
				terrain->mNeighborTerrainPos[j] = terrPos + Point3F(mNeighTerrAdj[j].x*blockSize,mNeighTerrAdj[j].y*blockSize,0);
		}
		else if ((mActiveTerrains[index])&&(diff.len() > mDropDistance))
		{
			Con::printf("deleting terrain %d %d, loadedTerrains %d, diff %f, dropdist %f clientPos %f %f %f",
				grid.x,grid.y,mLoadedTerrains.size(),diff.len(),mDropDistance,clientPos.x,clientPos.y,clientPos.z);
			//mActiveTerrains[index]->deleteObject();//Hm, apparently not the right way.
			mActiveTerrains[index] = NULL;//Maybe call out to script?  Or just figure it out.
			mLoadedTerrains.remove(terrain);
			char txtPos[255];
			sprintf(txtPos,"%f %f %f",terrPos.x,terrPos.y,terrPos.z);
			Con::executef("dropTerrainBlock",txtPos);			
			//Con::printf("Called out to dropTerrainBlock, loadedTerrains %d",mLoadedTerrains.size());
		}
	}

	//Third thing: go through current list of mLoadedTerrains, and update mNeighborTerrainLoaded and mIsEdge.
	for (U32 i=0;i<mLoadedTerrains.size();i++)
	{
		TerrainBlock *terrain = mLoadedTerrains[i];

		//First, clear all, don't know we're loaded until we know, some could have deleted.
		for (U32 j=0;j<8;j++) terrain->mNeighborTerrainLoaded[j] = false;
		terrain->mIsEdge = false;

		Point3F terrPos = terrain->getPosition();
		Point2I grid = Point2I((S32)(terrPos.x/blockSize),(S32)(terrPos.y/blockSize));
		for (U32 j=0;j<8;j++)
		{
			Point2I testGrid = grid + mNeighTerrAdj[j];
			S32 index = (testGrid.y*mNumGridColumns)+testGrid.x;
			if ( mActiveTerrains[index] || mLoadingTerrains[index] )
			{
				terrain->mNeighborTerrainLoaded[j] = true;
			}
		}
		for (U32 j=0;j<8;j++)//Now, if any edges are NULL, isEdge = true.
		{		
			if (terrain->mNeighborTerrainLoaded[j]==NULL)
				terrain->mIsEdge = true;
		}
	}

	//Finally, let's check to see if we have crossed a terrain border, and should change mCurrentTerrain. 
	U32 newGridColumn = (S32)(clientPos.x/blockSize);
	U32 newGridRow = (S32)(clientPos.y/blockSize);
	//Con::printf("current column, row: %d %d   new column, row %d %d",
	//	mCurrentGridColumn,mCurrentGridRow,newGridColumn,newGridRow);
	if ((newGridColumn!=mCurrentGridColumn)||(newGridRow!=mCurrentGridRow))
	{
		if (mActiveTerrains[(newGridRow*mNumGridColumns)+newGridColumn])
		{
			mCurrentTerrain = mActiveTerrains[(newGridRow*mNumGridColumns)+newGridColumn];
			mCurrentGridColumn = newGridColumn;
			mCurrentGridRow = newGridRow;
		}
	}

	setClientPos(clientPos);//This is for remembering where you are in the database, but problems developed...
	mTick++;
	
}*/

//void TerrainMaster::checkEntities(Point3F pos,F32 range)
//{
//	//Ultimately, I think we'll have an entity (or sceneobject, or something) class which contains only position data, and search 
//	//for those here, while linking out to all the specific entity types like static shape, interior, aiPlayer, forest, etc.
//	if (!mSQL)
//		return;
//
//	char query[512];
//	int id,result,result2;//WARNING body_id,joint_id,
//	sqlite_resultset *resultSet,*resultSet2;
//
//	//HERE: ultimately, for efficiency's sake, I could really use an internal SQL function to compare vectors, 
//	//before querying the whole table.  (LATER)
//	sprintf(query,"SELECT * FROM entity;");//WHERE VectorDiff(pos_x,pos_y,pos_z,pos.x,pos.y,pos.z)<DETECT_RANGE
//	result = mSQL->ExecuteSQL(query);
//	resultSet = mSQL->GetResultSet(result);
//	//for (U32 i=0;i<resultSet->iNumRows;i++)
//	//{
//	//	Con::printf("Found entity from table %s  id %d  range %f",resultSet->vRows[i]->vColumnValues[1],
//	//					dAtoi(resultSet->vRows[i]->vColumnValues[2]),dAtof(resultSet->vRows[i]->vColumnValues[6]));
//	//}
//	for (U32 i=0;i<resultSet->iNumRows;i++)
//	{
//		S32 entity_id = dAtoi(resultSet->vRows[i]->vColumnValues[0]);
//		String tablename = resultSet->vRows[i]->vColumnValues[1];
//		S32 table_id = dAtoi(resultSet->vRows[i]->vColumnValues[2]);
//		Point3F entity_pos = Point3F(dAtof(resultSet->vRows[i]->vColumnValues[3]),
//												dAtof(resultSet->vRows[i]->vColumnValues[4]),
//												dAtof(resultSet->vRows[i]->vColumnValues[5]));
//		F32 range = dAtof(resultSet->vRows[i]->vColumnValues[6]);
//		Point3F diff = pos - entity_pos;
//		//Con::printf("diff = %f",diff.len());
//		if ((range==0.0)||(diff.len()<range))
//		{
//			if (!strcmp(tablename.c_str(),"blob"))
//			{
//				if (mBlobs[table_id]==false)
//				{
//					sprintf(query,"SELECT content FROM blob WHERE id=%d;",table_id);
//					result2 = mSQL->ExecuteSQL(query);
//					resultSet2 = mSQL->GetResultSet(result2);
//					if (resultSet2->iNumRows==1)
//					{
//						Con::executef("eval",resultSet2->vRows[0]->vColumnValues[0]);
//						Con::printf("evaluating code from db:  %s",resultSet2->vRows[0]->vColumnValues[0]);
//						mBlobs[table_id] = true;
//					}
//				}
//			} else if (!strcmp(tablename.c_str(),"staticShape")) {
//				if (mStatics[table_id]==false)
//				{
//					sprintf(query,"SELECT * FROM staticShape WHERE id=%d;",table_id);//WHERE VectorDiff(pos_x,pos_y,pos_z,pos.x,pos.y,pos.z)<DETECT_RANGE
//					result2 = mSQL->ExecuteSQL(query);
//					resultSet2 = mSQL->GetResultSet(result2);
//					if (resultSet2->iNumRows==1)
//					{
//						S32 id = dAtoi(resultSet2->vRows[i]->vColumnValues[0]);
//						String filename = resultSet2->vRows[i]->vColumnValues[1];
//						Point3F pos = Point3F(dAtof(resultSet2->vRows[i]->vColumnValues[2]),
//							dAtof(resultSet2->vRows[i]->vColumnValues[3]),
//							dAtof(resultSet2->vRows[i]->vColumnValues[4]));
//						QuatF q = QuatF(dAtof(resultSet2->vRows[i]->vColumnValues[5]),
//							dAtof(resultSet2->vRows[i]->vColumnValues[6]),
//							dAtof(resultSet2->vRows[i]->vColumnValues[7]),
//							dAtof(resultSet2->vRows[i]->vColumnValues[8]));
//						Point3F scale = Point3F(dAtof(resultSet2->vRows[i]->vColumnValues[9]),
//							dAtof(resultSet2->vRows[i]->vColumnValues[10]),
//							dAtof(resultSet2->vRows[i]->vColumnValues[11]));
//
//						char txtPos[128],txtQuat[128],txtScale[128],txtID[12];	
//						Con::printf("Found a static shape! filename: %s  pos %f %f %f  quat ",filename.c_str(),pos.x,pos.y,pos.z,q.x,q.y,q.z,q.w);
//						sprintf(txtPos,"%f %f %f",pos.x,pos.y,pos.z);
//						sprintf(txtQuat,"%f %f %f %f",q.x,q.y,q.z,q.w);
//						sprintf(txtScale,"%f %f %f",scale.x,scale.y,scale.z);
//						sprintf(txtID,"%d",id);
//						Con::executef("loadStaticShape",filename.c_str(),txtPos,txtQuat,txtScale,txtID);
//						mStatics[table_id] = true;
//					}
//				}
//			}
//		}
//	}
//}


///////////////////////////////////////////////////////////////


/////  SOCKET TESTING ///////////////////
/*void skyboxSocketConnect()
{
   const int packetsize = 1024;//Turns out this isn't necessary as an actual packet size, TCP handles that.
   unsigned char buffer[packetsize],buffer2[packetsize];//I do need a number for my input buffer size though.
   
   struct sockaddr_in serv_addr, cli_addr, terr_serv_addr, terr_cli_addr;
   int n;

   //TESTING... nonblocking select set
   struct timeval tv,terr_tv;//just in case we're modifying this in any way...

   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) 
     Con::printf("ERROR opening socket\n");
   else 
     Con::printf("SUCCESS opening socket\n");
      
   terrSock = socket(AF_INET, SOCK_STREAM, 0);
   if (terrSock < 0) 
     Con::printf("ERROR opening terrSock\n");
   else 
     Con::printf("SUCCESS opening terrSock\n");

   ///////////////////
   //int yes=1;
   BOOL bOptVal = TRUE;
   //// lose the pesky "Address already in use" error message
   if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *) &bOptVal,sizeof(BOOL)) == -1) {
     perror("setsockopt");
     exit(1);
   } 
   if (setsockopt(terrSock,SOL_SOCKET,SO_REUSEADDR,(char *) &bOptVal,sizeof(BOOL)) == -1) {
     perror("setsockopt");
     exit(1);
   } 

   Con::printf("setting up buffers, etc.\n");  
   
   //bzero((char *) &serv_addr, sizeof(serv_addr));
   //portno = 9934;//atoi(argv[1]);
   ZeroMemory(&serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(mWorldServerPort);
   
   if (bind(sockfd, (struct sockaddr *) &serv_addr,
	    sizeof(serv_addr)) < 0) 
     Con::printf("ERROR on binding sockfd");

   //ZeroMemory(&terr_serv_addr, sizeof(terr_serv_addr));
   //terr_serv_addr.sin_family = AF_INET;
   //terr_serv_addr.sin_addr.s_addr = INADDR_ANY;
   //terr_serv_addr.sin_port = htons(terrainPort);
   //
   //if (bind(terrSock, (struct sockaddr *) &terr_serv_addr,
	  //  sizeof(terr_serv_addr)) < 0) 
   //  Con::printf("ERROR on binding terrSock");

   
   //int select(int numfds, fd_set *readfds, fd_set *writefds,
   //        fd_set *exceptfds, struct timeval *timeout); 

   //listen(sockfd,5);//5 is backlog number, might want more if we get busy.


   FD_ZERO(&master);
   FD_ZERO(&readfds);
   FD_SET(sockfd, &master);

   tv.tv_sec = sockTimeout;
   tv.tv_usec = 0;

   readfds = master;
   select(sockfd+1, &readfds, NULL, NULL, &tv);
      
   //terr_tv.tv_sec = sockTimeout;//Just in case it's getting modified, &tv makes me nervous...
   //terr_tv.tv_usec = 0;
   //
   //FD_ZERO(&terr_master);
   //FD_ZERO(&terr_readfds);
   //FD_SET(terrSock, &terr_master);
   // 
   //terr_readfds = terr_master;
   //select(terrSock+1, &terr_readfds, NULL, NULL, &terr_tv);
}*/

/*void skyboxSocketListen()
{
	//Con::printf("skyboxSocketListen, stage %d \n",skyboxStage);
	Con::printf(".");
	//if (skyboxStage>0)
	//{//Here, need timing mechanism!  Do not call draw again until image is done.
	//	skyboxSocketDraw();
	//	return;
	//}
	//Con::printf("Listening...\n");
	//int newsockfd;
	int clilen;
	const int packetsize = 1024;//Turns out this isn't necessary as an actual packet size, TCP handles that.
	char buffer[packetsize];//,buffer2[packetsize];//I do need a number for my input buffer size though.

	struct sockaddr_in cli_addr;
	int n;
	FILE *fwp,*fp;

	//TESTING... nonblocking select set
	struct timeval tv;  

	tv.tv_sec = sockTimeout;
	tv.tv_usec = 0;

	listen(sockfd,5);//5 is backlog number, might want more if we get busy.

	readfds = master;
	select(sockfd+1, &readfds, NULL, NULL, &tv);

	if (FD_ISSET(sockfd, &readfds))
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
		if (newsockfd < 0) {
			Con::printf("ERROR on accept");
			return;
		} else {
			Con::printf("accepted socket connection!\n");
		}

		FD_SET(newsockfd,&master);

		ZeroMemory(buffer, sizeof(packetsize));
		n = recv(newsockfd,buffer,packetsize,0);
		if (n < 0) Con::printf("ERROR reading from socket");
		Con::printf("reading...\n");

		int num_args = 11;

		char resourcePathArray[1024];
		char *bytes = &(buffer[0]);
		vector<unsigned char> byteVec(bytes,bytes + sizeof(float) * num_args);
		float *argArray = reinterpret_cast<float*>(bytes);
		bool mLoadTerrains[3][3];

		float player_long = argArray[0];
		float player_lat = argArray[1];
		float player_elev = argArray[2];
		//HERE:  New changes coming!  Now we are going to start writing a large (5x5 terrain tile) area
		//at one time, to a set of bin files (one file for height data, one for textures, so far.)
		float tileWidth = argArray[3];//Total distance covered in meters by one tile, assumed to be square.
		int heightmapRes = (int)argArray[4];
		int textureRes = (int)argArray[5];
		float tileWidthLong = argArray[6];
		float tileWidthLat = argArray[7];
		float base_long = argArray[8];
		float base_lat = argArray[9];
		mFullRebuild = (bool)argArray[10];

		Con::printf("got a socket connection!  heightMapRes: %d\n",heightmapRes);
	}
}
*/


/* //// FROM UNITY C# ////


	
	// Update is called once per frame
	void Update () {
		mPlayer = GameObject.FindGameObjectWithTag("Player");
		if (mPlayer==null)
			return;
		
		Vector3 playerPos = mPlayer.transform.position;
		
		//In case for any reason we have accidentally fallen off the world... 
		if (playerPos.y<-100.0f)//Doesn't work though... :-(
		{
			Debug.Log("PLAYER FALLING!!! " + playerPos.ToString() );
			//mPlayer.transform.position.Set(playerPos.x,mMaxElev+10.0f,playerPos.z);
			mPlayer.transform.Translate(new Vector3(0,mMaxElev+10.0f,0));
			//mPlayer.
		}
		///////////////////////////////////////////////////////////////////////////
		// First time terrain setup logic.
		//   FIX!!  Make this call LoadTerrainFromServer() instead of repeating it here.
		if (mPlayerLongitude==0.0f)
		{				
			reloadSkyboxTextures();
			
			mPlayerLongitude = mCenterLongitude + (playerPos.x * mDegreesPerMeterLong);
			mPlayerLatitude = mCenterLatitude + (playerPos.z * mDegreesPerMeterLat);				
			mPlayerElevation = playerPos.y + mMinElev + 30.0f;
			
			
			for(int i=0; i<mTerrainsXCount; i++)//horizontal
			{
				for(int j=0; j<mTerrainsZCount; j++)//vertical
				{
					if ((i!=1)||(j!=1))//create all except the initial base terrain
					{
						TerrainData tempData = new TerrainData();
						
						CopyTerrainDataFromTo(mTerrainData[1,1], ref tempData);
						mTerrainData[j,i] = tempData;
						mTerrains[j,i] = Terrain.CreateTerrainGameObject(mTerrainData[j,i]);
						mTerrainsLayout[j,i] = mTerrains[j,i]; 
						
						//Vector3 terrPos = new Vector3(mStartPos.x+(float)(i)*mTileDistance,0,mStartPos.z+(float)(j)*mTileDistance);
						//mTerrains[j,i].transform.position = terrPos;

					}//	else {  
					//	Vector3 terrPos = new Vector3(mStartPos.x+(float)(i)*mTileDistance,0,mStartPos.z+(float)(j)*mTileDistance);
					//	mTerrains[j,i].transform.position = terrPos;
					//}
				}                                          
			}
			
			for(int i=0; i<mTerrainsXCount; i++)//good to go for next time.
				for(int j=0; j<mTerrainsZCount; j++)
					mLoadTerrains[j,i] = true; 
			
			loadTerrainData();
	
			return;
		}

		
		///////////////////////////////////////////////////////////////////////////
		//Else, do regular every-time logic.
		
		if ((File.Exists(mTerrainLockfile))&&(!mTerrainCreating))
		{
			Debug.Log("TERRAIN LOCK FILE EXISTS!!");
			mTerrainCreating = true;
			return;
		}
		
		if ((File.Exists(mSkyboxLockfile))&&(!mSkyboxCreating))
		{
			Debug.Log("SKYBOX LOCK FILE EXISTS!!");
			mSkyboxCreating = true;
			return;
		}
				
		if ((mSkyboxCreating)&&(!File.Exists(mSkyboxLockfile))&&(mSkyboxTick>0)&&(mTick>mSkyboxTick+100))
		{
			Debug.Log("Calling setupSkybox, tick=" + mTick + ", skyboxTick=" + mSkyboxTick);
			setupSkyboxTextures();//Make sure we don't do this on the same pass
			mSkyboxCreating = false;
			return;
		}
		
		if (mSkyboxLoading)
		{
			if ((front_load.isDone)&&(left_load.isDone)&&(back_load.isDone)&&
				(right_load.isDone)&&(up_load.isDone))
			{
				reloadSkyboxTextures();
				mSkyboxLoading = false;
			}
			return;
		}
		
		//HERE:  Now, we are going to rely on lockfiles to decide when to load, and let flightgear decide
		//when we have moved far enough or waited long enough to reload skybox and/or terrain.
		
		
		//DON'T FORGET to make the lockfiles at startup now!		
		if ((!File.Exists(mTerrainLockfile))&&(mTerrainCreating==true))//(mSkyboxTick>0)&&(mTick>mSkyboxTick+100))
		{	                     
			//HERE: in a multiplayer environment, we may need to figure out which one is the local client player.
			
			Debug.Log("RELOADING TERRAIN!!");
			mainCamera = Camera.main;
			mainCamera.far = 8000.0f;
			
			int tileSize = (int)((float)mTileSpacing * mSquareSize);
			
			//Hm, first let's figure out which square we're in, real quick.  
			int playerGridX,playerGridZ;
			playerGridX = (int)((playerPos.x - mStartPos.x) / (float)tileSize);
			playerGridZ = (int)((playerPos.z - mStartPos.z) / (float)tileSize);
			
			Debug.Log("Player terrain grid pos: " + playerGridX + " " + playerGridZ + 
						" actual pos " + playerPos.x + " " + playerPos.z);
			
			if ((playerGridX==mTerrainsXMid)&&(playerGridZ==mTerrainsZMid))//(1,1)
				return;//should never happen, if we're using the lockfile correctly.
			
//			GameObject midTerrain = mTerrains[mTerrainsZMid,mTerrainsXMid];
//			double lowX = midTerrain.transform.position.x - ((double)(tileSize)/8.0);
//			double highX = midTerrain.transform.position.x + (double)(tileSize) + ((double)(tileSize)/8.0);
//			double lowZ = midTerrain.transform.position.z - ((double)(tileSize)/8.0);
//			double highZ = midTerrain.transform.position.z + (double)(tileSize) + ((double)(tileSize)/8.0);
//			
//			if ((playerPos.x < lowX)  || 
//				(playerPos.x > highX) ||
//				(playerPos.z < lowZ)  ||
//				(playerPos.z > highZ))
//			{
//				//Debug.Log("Crossing a boundary! " + playerGridX);	
//			} else {
//				return;
//			}
			
			GameObject [,] tempTerrains = new GameObject[mTerrainsZCount,mTerrainsXCount];
			Vector2 terrPos;
			Vector2 terrSize;
			Vector3 startPos;
			terrSize.x = tileSize;  terrSize.y = tileSize;
			terrPos.x = 0.0f;  terrPos.y = 0.0f;
			
			for(int i=0; i<mTerrainsXCount; i++)//good to go for next time.
				for(int j=0; j<mTerrainsZCount; j++)
					mLoadTerrains[j,i] = false; 
			
			
			if (playerGridX==0)
				mStartLongitude -= mTileWidthLongitude;
			else if (playerGridX==2)
				mStartLongitude += mTileWidthLongitude;
			if  (playerGridZ==0)
				mStartLatitude -= mTileWidthLatitude;
			else if (playerGridZ==2)
				mStartLatitude += mTileWidthLatitude;
			
			mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
			mStartPos.z = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;
			//So, if we're still here, we know we crossed a line somewhere.  Now we're going to shift 
			//our grid around, keeping the terrain we're standing on and whichever other ones
			if ((playerGridX==0)&&(playerGridZ==0)) {//lower left
				
				tempTerrains[2,2] = mTerrainsLayout[1,1];
				tempTerrains[2,1] = mTerrainsLayout[1,0];
				tempTerrains[1,2] = mTerrainsLayout[0,1];
				tempTerrains[1,1] = mTerrainsLayout[0,0];
				
				//and load the other five from the server.	
				tempTerrains[2,0] = mTerrainsLayout[2,0];
				mLoadTerrains[2,0] = true;				
				tempTerrains[1,0] = mTerrainsLayout[2,1];	
				mLoadTerrains[1,0] = true;					
				tempTerrains[0,0] = mTerrainsLayout[2,2];	
				mLoadTerrains[0,0] = true;					
				tempTerrains[0,1] = mTerrainsLayout[1,2];	
				mLoadTerrains[0,1] = true;					
				tempTerrains[0,2] = mTerrainsLayout[0,2];	
				mLoadTerrains[0,2] = true;	
				
			} else if ((playerGridX==0)&&(playerGridZ==1)) {//mid left
				
				tempTerrains[2,2] = mTerrainsLayout[2,1];
				tempTerrains[1,2] = mTerrainsLayout[1,1];
				tempTerrains[0,2] = mTerrainsLayout[0,1];
				tempTerrains[2,1] = mTerrainsLayout[2,0];
				tempTerrains[1,1] = mTerrainsLayout[1,0];	
				tempTerrains[0,1] = mTerrainsLayout[0,0];	
				
				//and load the other three from the server.	
				tempTerrains[2,0] = mTerrainsLayout[2,2];	
				mLoadTerrains[2,0] = true;						
				tempTerrains[1,0] = mTerrainsLayout[1,2];	
				mLoadTerrains[1,0] = true;					
				tempTerrains[0,0] = mTerrainsLayout[0,2];		
				mLoadTerrains[0,0] = true;	
				
			} else if ((playerGridX==0)&&(playerGridZ==2)) {//upper left
				
				tempTerrains[0,2] = mTerrainsLayout[1,1];
				tempTerrains[0,1] = mTerrainsLayout[1,0];
				tempTerrains[1,2] = mTerrainsLayout[2,1];
				tempTerrains[1,1] = mTerrainsLayout[2,0];	
				
				//and load the other five from the server.	
				tempTerrains[0,0] = mTerrainsLayout[0,0];	
				mLoadTerrains[0,0] = true;					
				tempTerrains[1,0] = mTerrainsLayout[0,1];	
				mLoadTerrains[1,0] = true;					
				tempTerrains[2,0] = mTerrainsLayout[0,2];	
				mLoadTerrains[2,0] = true;					
				tempTerrains[2,1] = mTerrainsLayout[1,2];	
				mLoadTerrains[2,1] = true;					
				tempTerrains[2,2] = mTerrainsLayout[2,2];	
				mLoadTerrains[2,2] = true;	
				
			} else if ((playerGridX==1)&&(playerGridZ==2)) {//upper middle

				tempTerrains[0,0] = mTerrainsLayout[1,0];
				tempTerrains[0,1] = mTerrainsLayout[1,1];	
				tempTerrains[0,2] = mTerrainsLayout[1,2];					
				tempTerrains[1,0] = mTerrainsLayout[2,0];
				tempTerrains[1,1] = mTerrainsLayout[2,1];	
				tempTerrains[1,2] = mTerrainsLayout[2,2];	
				
				//and load the other three from the server.	
				tempTerrains[2,0] = mTerrainsLayout[0,0];	
				mLoadTerrains[2,0] = true;					
				tempTerrains[2,1] = mTerrainsLayout[0,1];		
				mLoadTerrains[2,1] = true;					
				tempTerrains[2,2] = mTerrainsLayout[0,2];	
				mLoadTerrains[2,2] = true;		
				
			} else if ((playerGridX==2)&&(playerGridZ==2)) {//upper right
				
				tempTerrains[0,0] = mTerrainsLayout[1,1];
				tempTerrains[0,1] = mTerrainsLayout[1,2];
				tempTerrains[1,0] = mTerrainsLayout[2,1];
				tempTerrains[1,1] = mTerrainsLayout[2,2];
				
				//and load the other five from the server.	
				tempTerrains[0,2] = mTerrainsLayout[0,2];	
				mLoadTerrains[0,2] = true;					
				tempTerrains[1,2] = mTerrainsLayout[0,1];	
				mLoadTerrains[1,2] = true;					
				tempTerrains[2,2] = mTerrainsLayout[0,0];	
				mLoadTerrains[2,2] = true;					
				tempTerrains[2,1] = mTerrainsLayout[1,0];	
				mLoadTerrains[2,1] = true;					
				tempTerrains[2,0] = mTerrainsLayout[2,0];	
				mLoadTerrains[2,0] = true;	
				
			} else if ((playerGridX==2)&&(playerGridZ==1)) {//mid right
				
				tempTerrains[2,0] = mTerrainsLayout[2,1];
				tempTerrains[1,0] = mTerrainsLayout[1,1];
				tempTerrains[0,0] = mTerrainsLayout[0,1];
				tempTerrains[2,1] = mTerrainsLayout[2,2];
				tempTerrains[1,1] = mTerrainsLayout[1,2];	
				tempTerrains[0,1] = mTerrainsLayout[0,2];		
				
				//and load the other three from the server.	
				tempTerrains[2,2] = mTerrainsLayout[2,0];
				mLoadTerrains[2,2] = true;
				tempTerrains[1,2] = mTerrainsLayout[1,0];
				mLoadTerrains[1,2] = true;					
				tempTerrains[0,2] = mTerrainsLayout[0,0];		
				mLoadTerrains[0,2] = true;
				
			} else if ((playerGridX==2)&&(playerGridZ==0)) {//lower right
				
				tempTerrains[2,0] = mTerrainsLayout[1,1];
				tempTerrains[2,1] = mTerrainsLayout[1,2];
				tempTerrains[1,0] = mTerrainsLayout[0,1];
				tempTerrains[1,1] = mTerrainsLayout[0,2];	
				
				//and load the other five from the server.	
				tempTerrains[2,2] = mTerrainsLayout[2,2];	
				mLoadTerrains[2,2] = true;					
				tempTerrains[1,2] = mTerrainsLayout[2,1]; 	
				mLoadTerrains[1,2] = true;					
				tempTerrains[0,2] = mTerrainsLayout[2,0];	
				mLoadTerrains[0,2] = true;							
				tempTerrains[0,1] = mTerrainsLayout[1,0];	
				mLoadTerrains[0,1] = true;					
				tempTerrains[0,0] = mTerrainsLayout[0,0];	
				mLoadTerrains[0,0] = true;			
				
			} else if ((playerGridX==1)&&(playerGridZ==0)) {//lower middle
				
				tempTerrains[2,0] = mTerrainsLayout[1,0];
				tempTerrains[2,1] = mTerrainsLayout[1,1];	
				tempTerrains[2,2] = mTerrainsLayout[1,2];					
				tempTerrains[1,0] = mTerrainsLayout[0,0];
				tempTerrains[1,1] = mTerrainsLayout[0,1];	
				tempTerrains[1,2] = mTerrainsLayout[0,2];	
				
				//and load the other three from the server.	
				tempTerrains[0,0] = mTerrainsLayout[2,0];	
				mLoadTerrains[0,0] = true;
				tempTerrains[0,1] = mTerrainsLayout[2,1];
				mLoadTerrains[0,1] = true;
				tempTerrains[0,2] = mTerrainsLayout[2,2];		
				mLoadTerrains[0,2] = true;			
			} 
			
			//Now, copy all the new positions back into our mTerrainsLayout array, so we're 
			for(int i=0; i<mTerrainsXCount; i++)//good to go for next time.
				for(int j=0; j<mTerrainsZCount; j++)
					mTerrainsLayout[j,i] = tempTerrains[j,i];
					
			for(int i=0; i<mTerrainsXCount; i++)//Now, do the actual load and move on the modified terrains.
			{
				for(int j=0; j<mTerrainsZCount; j++)			
				{
					if (mLoadTerrains[j,i])
					{
						Debug.Log("Reload terrain ["+j+","+i+"]:  " + mTerrainsLayout[j,i].transform.position);
					} else {
						Debug.Log("Existing terrain ["+j+","+i+"]: " + mTerrainsLayout[j,i].transform.position);
					}
				}
			}	
			
			loadTerrainData();
			
			
//			for(int i=0; i<mTerrainsXCount; i++)//Now, do the actual load and move on the modified terrains.
//			{
//				for(int j=0; j<mTerrainsZCount; j++)			
//				{
//					if (mLoadTerrains[j,i])
//					{
//						terrPos.Set(mStartLongitude+((float)(i)*mTileWidthLongitude),
//									mStartLatitude+((float)(j)*mTileWidthLatitude));
//						//LoadTerrainFromServer(mTerrainsLayout[j,i],terrPos,terrSize);	
//						startPos.z = mStartPos.z+((float)(j)*mTileDistance);	
//						startPos.x = mStartPos.x+((float)(i)*mTileDistance);
//						mTerrainsLayout[j,i].transform.position = startPos;
//						Debug.Log("Positioning terrain ["+j+","+i+"]:  " + startPos.x + " " + startPos.z );
//					} else {
//						Debug.Log("Existing terrain position["+j+","+i+"]: " + mTerrainsLayout[j,i].transform.position);
//					}
//				}
//			}
			
			mTerrainCreating = false;
			return;
		}
		
		//Important: do this AFTER all other options have been exhausted, ie don't do it unless nothing else is going on.
		if (  Input.GetKeyDown("p") ||(mTick++>mSkyboxTick+4000))
		{//NOW: this is the only call we need to make, server will handle terrain & skyboxes from here.
			Debug.Log("calling loadSkybox!!!!!!!!!!!  " + mTick + " skyboxTick " + mSkyboxTick);
			loadSkyboxFromServer(false);//(false) = not fullRebuild, so don't wait for it.
			mSkyboxTick = mTick;
		}
	}
	
	void loadTerrainData()
	{		
		Debug.Log("Calling Load Terrain Data!");
		int area_X = (int)mTileDistance;// * mTerrainsXCount;
		int area_Z = (int)mTileDistance;// * mTerrainsZCount;
		int area_X_int = (area_X/10)+1;
		int area_Z_int = (area_Z/10)+1;
		int array_size = (area_X_int * area_Z_int) + mNumHeightBinArgs;
		int char_array_size = array_size * sizeof(float);
		byte[] bytes_received = new byte[char_array_size];
		float [] inputValues = new float[array_size];			
		
		int tex_area_X_int = mTextureRes;// * mTerrainsXCount;
		int tex_area_Z_int = mTextureRes;// * mTerrainsZCount;			
		int tex_array_size = tex_area_X_int * tex_area_Z_int;
		byte[] tex_bytes_received = new byte[tex_array_size];
		
		bool fullRebuild=false;
		
		mTerrainLoaded = false;
		if (!File.Exists(mHeightsBinFile))
		{
			Debug.Log("no terrain loaded, calling for full rebuild.");
			fullRebuild = true;
			
			mStartLatitude = mPlayerLatitude - (mTileWidthLatitude * 1.5f);//One and a half tiles away, start 
			mStartLongitude = mPlayerLongitude - (mTileWidthLongitude * 1.5f);//     your nine-tile square.
			mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
			mStartPos.z = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;
			
			loadSkyboxFromServer(fullRebuild);
			if (!File.Exists(mSkyboxLockfile))
			{		
				FileStream fsw = new FileStream(mSkyboxLockfile, FileMode.Create,FileAccess.ReadWrite);
				BinaryWriter bw = new BinaryWriter(fsw);
				bw.Close();
				fsw.Close();
			}		
			
			int tick=0;
			while (File.Exists(mSkyboxLockfile))
			{
				if (tick++%1000==0) Debug.Log("Waiting for terrain data.");
			}		
		}

	
		//HERE: loop through nine files, instead of one.
		for(int x=0; x<mTerrainsXCount; x++)//horizontal
		{
			for(int y=0; y<mTerrainsZCount; y++)//vertical
			{
				if (mLoadTerrains[y,x])
				{
					mHeightsBinFile = mResourcePath + "terrain_" + y + "_" + x +".heights.bin";
					mTexturesBinFile = mResourcePath + "terrain_" + y + "_" + x +".textures.bin";
					
					FileStream fs = new FileStream(mHeightsBinFile, FileMode.Open,FileAccess.Read);
					if (fs.CanRead)
					{
						BinaryReader br = new BinaryReader(fs);
						bytes_received = br.ReadBytes(char_array_size);
						br.Close();
						fs.Close();		
						
						for (int i=0;i<array_size;i++)
				    	{
							float final;
							float altitude = BitConverter.ToSingle(bytes_received,i*sizeof(float));
							if (i < mNumHeightBinArgs) {
								final = altitude;//these are actual numbers, not height data.
							} else {
								if (altitude>0.0f)//This could bite you in Death Valley, but otherwise a good sanity check.
									final = (altitude - mMinElev) / mDeltaElev;
								else final = 0.0f;
								if (final<0.0f) {
									//Debug.Log("Height value too small " + final);
									final = 0.0f;
								} else if (final>1.0f) {
									//Debug.Log("Height value too large " + final);
									final = 0.0f;
								}
							}	
							if ((i>=mNumHeightBinArgs)&&((final<0.0f)||(final>1.0f)))
								Debug.Log("FINAL final is still out of whack: " + final);
							inputValues[i] = final;
						}			
						
						float fileLong = inputValues[0];
						float fileLat = inputValues[1];
						float fileTileWidth = inputValues[2];
						int fileHeightmapRes = (int)inputValues[3];
						int fileTextureRes = (int)inputValues[4];
						
						if ((x==0)&&(y==0))
						{//This matters a lot on the first load, with saved tiles.  After first load we 
							//shouldn't need to grab it from the tile, we should be able to manage it ourselves
							//on this side of things.  Which is important, because half the time we don't need to
							//reload tile [0,0].
							mStartLatitude = fileLat;//One and a half tiles away, start 
							mStartLongitude = fileLong;//     your nine-tile square.
							mStartPos.x = (mStartLongitude - mCenterLongitude) * mMetersPerDegreeLong;
							mStartPos.z = (mStartLatitude - mCenterLatitude) * mMetersPerDegreeLat;
						}
						
						if ((fileTileWidth!=mTileDistance)||
							(fileHeightmapRes!=mHeightmapRes)||
							(fileTextureRes!=mTextureRes))
						{
							fullRebuild = true;
							loadSkyboxFromServer(fullRebuild);
							return;
						}
			
						mTerrainLoaded = true;
						
						Vector3 terrPos = new Vector3(mStartPos.x+(float)(x)*mTileDistance,0,mStartPos.z+(float)(y)*mTileDistance);
						mTerrainsLayout[y,x].transform.position = terrPos;
						Debug.Log("Terrain " + y + " " + x + " position = " + terrPos + " startPos " + mStartPos);
						TerrainData tData = mTerrainsLayout[y,x].GetComponent<Terrain>().terrainData;
						
						float [,] Tiles = new float[mHeightmapRes,mHeightmapRes];
						for (int i=0;i<mHeightmapRes;i++)//vertical
			    		{	
							for (int j=0;j<mHeightmapRes;j++)//horizontal
			    			{	
								//int count = (((i+(y*mTileSpacing))*area_X_int) + (j+(x*mTileSpacing))) + mNumHeightBinArgs;
								int count = ((i*area_X_int) + j) + mNumHeightBinArgs;
								Tiles[i,j] = inputValues[count];							
							}
			    		}
						tData.SetHeights(0,0,Tiles);	
						
						FileStream tfs = new FileStream(mTexturesBinFile, FileMode.Open,FileAccess.Read);
						BinaryReader tbr = new BinaryReader(tfs);
						tex_bytes_received = tbr.ReadBytes(tex_array_size);
						tbr.Close();
						tfs.Close();
						
						float[,,] splatmapData = mTerrainData[y,x].GetAlphamaps(0, 0, mTextureRes, mTextureRes);	
						for (int i=0;i<mTextureRes;i++) 
						{
							for (int j=0;j<mTextureRes;j++) 
							{
								//int count = (((j+(y*mTextureRes))*tex_area_X_int) + (i+(x*mTextureRes)));
								int count = ((j*tex_area_X_int) + i);
								byte t = tex_bytes_received[count];
								SplatPrototype[] splatPrototypes = tData.splatPrototypes;
								for (int k=0;k<splatPrototypes.Length;k++) 
								{
									if (t==k)
										splatmapData[j, i, k] = 1.0f;
									else
										splatmapData[j, i, k] = 0.0f;
								}
							}
						}					
						tData.SetAlphamaps(0,0, splatmapData);
					}
				}
			}
		}
		
		

		
				
		//mStartLatitude = mPlayerLatitude - (mTileWidthLatitude * 1.5f);//One and a half tiles away, start 
		//mStartLongitude = mPlayerLongitude - (mTileWidthLongitude * 1.5f);//     your nine-tile square.
		
		//mStartPos.x = playerPos.x - (mTileDistance * 1.5f); 
		//mStartPos.y = 0.0f; 
		//mStartPos.z = playerPos.z - (mTileDistance * 1.5f);
		
//		if (true)//(fullRebuild)
//		{
//			for(int i=0; i<mTerrainsXCount; i++)//horizontal
//			{
//				for(int j=0; j<mTerrainsZCount; j++)//vertical
//				{		
//					if (mLoadTerrains[j,i])
//					{			
//						Vector3 terrPos = new Vector3(mStartPos.x+(float)(i)*mTileDistance,0,mStartPos.z+(float)(j)*mTileDistance);
//						mTerrainsLayout[j,i].transform.position = terrPos;
//						Debug.Log("Terrain[" + j + "," + i + "] position " + terrPos );
//					}
//				}                                          
//			}
//		}
		
	

//		float [,] Tiles = new float[mHeightmapRes,mHeightmapRes];		
//    	for (int i=0;i<mHeightmapRes;i++)
//    	{
//			for (int j=0;j<mHeightmapRes;j++)
//    		{
//				Tiles[i,j] = 0.0f;
//			}
//		}
		

//		for(int x=0; x<mTerrainsXCount; x++)//horizontal
//		{
//			for(int y=0; y<mTerrainsZCount; y++)//vertical
//			{
//				if (mLoadTerrains[y,x])
//				{
//					for (int i=0;i<mHeightmapRes;i++)//vertical
//			    	{	
//						for (int j=0;j<mHeightmapRes;j++)//horizontal
//			    		{	
//							int count = (((i+(y*mTileSpacing))*area_X_int) + (j+(x*mTileSpacing))) + mNumHeightBinArgs;
//							Tiles[i,j] = inputValues[count];							
//						}
//			    	}	
//					////mTerrainData[y,x]..SetHeights(0,0,Tiles);//OOPS! terrainsLayout[], not terrains[]
//					TerrainData tData = mTerrainsLayout[y,x].GetComponent<Terrain>().terrainData;
//					tData.SetHeights(0,0,Tiles);		
//				}
//			}
//		}

		//Now, the textures.

		    	
//		for(int x=0; x<mTerrainsXCount; x++)//horizontal
//		{
//			for(int y=0; y<mTerrainsZCount; y++)//vertical
//			{
//				if (mLoadTerrains[y,x])
//				{
//					TerrainData tData = mTerrainsLayout[y,x].GetComponent<Terrain>().terrainData;
//					float[,,] splatmapData = mTerrainData[y,x].GetAlphamaps(0, 0, mTextureRes, mTextureRes);	
//					for (int i=0;i<mTextureRes;i++) 
//					{
//						for (int j=0;j<mTextureRes;j++) 
//						{
//							int count = (((j+(y*mTextureRes))*tex_area_X_int) + (i+(x*mTextureRes)));
//							byte t = tex_bytes_received[count];
//							SplatPrototype[] splatPrototypes = tData.splatPrototypes;
//							for (int k=0;k<splatPrototypes.Length;k++) 
//							{
//								if (t==k)
//									splatmapData[j, i, k] = 1.0f;
//								else
//									splatmapData[j, i, k] = 0.0f;
//							}
//						}
//					}
//					//mTerrainData[y,x].SetAlphamaps(0,0, splatmapData);
//					
//					tData.SetAlphamaps(0,0, splatmapData);
//				}
//			}
//		}	
		if (fullRebuild)
			setupSkyboxTextures();
	}

*/