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


#include "console/SQLiteObject.h"

#define TM_MAX_STATICS  1000 
#define TM_MAX_TERRAINS  1000 
#define TM_MAX_BLOBS  1000  //FIX, this is a VERY short term solution to the problem of efficient checking on the fly
//for whether or now we have loaded a static in the DB.  Whole current system is deeply flawed, however, need zones or 
//something to avoid all the constant querying.

/// The TerrainMaster organizes multiple terrains to cover an infinite grid.
class TerrainMaster : public SimObject
{
   typedef SimObject Parent;

public:
   F32 mSquareSize;

	TerrainBlock *mCurrentTerrain;//This is the terrain the player or camera is currently occupying. 
	Vector<TerrainBlock *> mActiveTerrains;//This is a filled vector array containing numRows*numColumns entries,
	//all of which will start as NULL pointers, and are filled by position in the grid as new terrains are added.
	Vector<bool> mLoadingTerrains;//This also sparse array of numRows*numColumns.
	Vector<TerrainBlock *> mLoadedTerrains;//This will contain only as many as are currently loaded.
	TerrainBlock *mMegaTerrain;//For far distance only, a ten times scale terrain
	//with the local blocks cut out of it.

   FileName mMatrixFile;
   FileName mOutputDirectory;
   FileName mInputDirectory;

	SQLiteObject *mSQL;

	Point2I mNeighTerrAdj[8];//To describe the ring of neighbor terrains, [0] = N =(0,1), goes clockwise.

	String mClientUsername;
	Point3F mClientPos;

	U32 mCheckCycle;//May not need this in the engine, but it's used in script for the checkTerrain schedule.
	F32 mLoadDistance;
	F32 mDropDistance;

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

	U32 mCurrentTick;

	F32 mMinHeight;
	F32 mMaxHeight;

	F32 mWorldBlockSize;

	bool mFirstTerrain;
	bool mAllTerrains;

	bool mStatics[TM_MAX_STATICS];//TEMP, sparsely populated array, to see if this id is loaded.
	//bool mTerrains[TM_MAX_TERRAINS];//TEMP, sparsely populated array, to see if this id is loaded.
	bool mBlobs[TM_MAX_BLOBS];//TEMP, sparsely populated array, to see if this id is loaded.

   TerrainMaster();
   virtual ~TerrainMaster();

   bool onAdd();
   static void initPersistFields();

	void setTerrainHeights(S32,S32,S32);
	void checkTerrain();
	void checkEntities(Point3F,F32);
	void loadTerrain(Point3F,Point2I);
	void setClientPos(Point3F pos);

   DECLARE_CONOBJECT( TerrainMaster );
};

#endif // _TERRMASTER_H_
