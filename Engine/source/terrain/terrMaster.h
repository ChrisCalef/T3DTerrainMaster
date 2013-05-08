//-----------------------------------------------------------------------------
// Copyright (c) 2012 BrokeAss Games, LLC
//-----------------------------------------------------------------------------

#ifndef _TERRMASTER_H_
#define _TERRMASTER_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif
#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif

//#include "console/SQLiteObject.h"

//SOCKETS: first pass, TEMP, this is actually from the server side, just making sure the socket works. 
#include <sys/types.h> 
#include <sys/stat.h>
#include "winsock2.h"


#define TM_MAX_STATICS  1000 
#define TM_MAX_TERRAINS  1000 
#define TM_MAX_BLOBS  1000  //FIX, this is a VERY short term solution to the problem of efficient checking on the fly
//for whether or now we have loaded a static in the DB.  Whole current system is deeply flawed, however, need zones or 
//something to avoid all the constant querying.

/// The TerrainMaster organizes multiple terrains to cover an infinite grid.
class TerrainMaster : public SimObject, public virtual ITickable
{
	typedef SimObject Parent;

public:

	//OBSOLETE: this was for the old system, which dealt with a very large array of small terrain tiles, saved on disk.
	//Now we only have the current nine terrains in existence at any time, the rest of the data lives in flightgear tiles.

	TerrainBlock *mCurrentTerrain;//This is the terrain the player or camera is currently occupying. 
	Vector<TerrainBlock *> mActiveTerrains;//This is a filled vector array containing numRows*numColumns entries,
	//all of which will start as NULL pointers, and are filled by position in the grid as new terrains are added.
	Vector<bool> mLoadingTerrains;//This also sparse array of numRows*numColumns.
	Vector<TerrainBlock *> mLoadedTerrains;//This will contain only as many as are currently loaded.
	//TerrainBlock *mMegaTerrain;//For far distance only, a ten times scale terrain
	//with the local blocks cut out of it.
	

	////THE WORLD SERVER WAY:
	TerrainBlock *mTerrains[3][3];//HERE: just hard coding this for the first pass.
	TerrainBlock *mTerrainsLayout[3][3];//If people really want 5x5 or more, fix it later.
	bool mLoadTerrains[3][3];

	FileName mMatrixFile;
	FileName mOutputDirectory;
	FileName mInputDirectory;

	SOCKET sockfd;
	
	FileName mSkyboxPath;
	FileName mTerrainPath;
	FileName mSkyboxLockfile;
	FileName mTerrainLockfile;
	FileName mTerrainHeightsBinFile;//One tile's worth of heights
	FileName mTerrainTexturesBinFile;//and textures.
	FileName skybox_files[5];//Filenames for the five skybox textures, because we 
	//are going to have to flip and rotate them before they can be used in Torque.

	//SQLiteObject *mSQL;

	//OBSOLETE
	Point2I mNeighTerrAdj[8];//To describe the ring of neighbor terrains, [0] = N =(0,1), goes clockwise.

	String mClientUsername;
	Point3F mClientPos;

	U32 mCheckCycle;//May not need this in the engine, but it's used in script for the checkTerrain schedule.
	F32 mLoadDistance;
	F32 mDropDistance;

	F32 mPlayerLongitude;
	F32 mPlayerLatitude;
	F32 mPlayerElevation;
	F32 mPlayerLongitudeLast;
	F32 mPlayerLatitudeLast;

	F32 mCenterLatitude;//this is the geographic center of the whole map
	F32 mCenterLongitude;//get this from the mission object?
		
	F32 mTileWidth;
	F32 mTileWidthLongitude;
	F32 mTileWidthLatitude;

	F32 mStartLongitude;//These all refer to the bottom left corner 
	F32 mStartLatitude;//  of the current nine-tile grid.
	Point3F mStartPos;

	//Put these in the GUI.		
	String mWorldServerIP;
	U32 mWorldServerPort;
	U32 mWorldServerTick;//Last time we ticked the world server.

	U32 mNumHeightBinArgs;// lat, long, tileWidth, heightmapRes, textureRes - these are to tell you where to
							//put your terrains (lat/long) as well as whether the resolutions are still valid.
		
	U32 mTerrainsXCount;
	U32 mTerrainsZCount;
	U32 mTerrainsXMid;
	U32 mTerrainsZMid;
		  
	F32 mMetersPerDegreeLong;
	F32 mDegreesPerMeterLong;
	F32 mMetersPerDegreeLat;
	F32 mDegreesPerMeterLat;

	F32 mMinElev;
	F32 mMaxElev;
	F32 mDeltaElev;
	F32 mSquareSize;

	U32 mHeightmapRes;
	U32 mTextureRes;
	U32 mLightmapRes;

	U32 mLeftMapX;
	U32 mLowerMapY;
	U32 mRightMapX;
	U32 mUpperMapY;

	U32 mNumRows;
	U32 mNumColumns;

	U32 mNumGridRows;
	U32 mNumGridColumns;

	U32 mCurrentGridRow;
	U32 mCurrentGridColumn;

	U32 mTick;
	U32 mTickInterval;

	F32 mMinHeight;
	F32 mMaxHeight;


	bool mFirstTerrain;
	bool mAllTerrains;

	bool mFreeCamera;
	bool mSkyboxCreating;
	bool mTerrainCreating;
	bool mSkyboxLoading;//See if these end up being necessary in Torque...
	bool mTerrainLoaded;//In Unity they made sense.

	bool mStatics[TM_MAX_STATICS];//TEMP, sparsely populated array, to see if this id is loaded.
	//bool mTerrains[TM_MAX_TERRAINS];//TEMP, sparsely populated array, to see if this id is loaded.
	bool mBlobs[TM_MAX_BLOBS];//TEMP, sparsely populated array, to see if this id is loaded.

	TerrainMaster();
	virtual ~TerrainMaster();

	bool onAdd();
	static void initPersistFields();	
	void processTick();
	void interpolateTick(F32);
	void advanceTime(F32);

	void setTerrainHeights(S32,S32,S32);
	//void checkTerrain();
	//void checkEntities(Point3F,F32);
	void loadTerrain(Point3F,Point2I);
	void setClientPos(Point3F pos);

	void loadTerrainData();
	void pingWorldServer(bool);//bool fullRebuild
	void reloadSkybox();

	void findClientPos();

	DECLARE_CONOBJECT( TerrainMaster );
};

#endif // _TERRMASTER_H_
