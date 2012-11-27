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
#include <stdio.h>



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
	mCurrentTick = 0;
	//mSQL = new SQLiteObject();
	mClientPos.zero();
	mWorldBlockSize = 2560.0;//2.5km squares, by default.
	mFirstTerrain = false;
	mAllTerrains = false;

	for (U32 i=0;i<TM_MAX_STATICS;i++) mStatics[i]=false;
	//for (U32 i=0;i<TM_MAX_TERRAINS;i++) mTerrains[i]=false;
	for (U32 i=0;i<TM_MAX_BLOBS;i++) mBlobs[i]=false;

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

   addField( "worldBlockSize", TypeF32, Offset( mWorldBlockSize, TerrainMaster ), "Terrain Matrix world block size" );

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

	mCurrentTerrain = NULL;

	SimSet* scopeAlwaysSet = Sim::getGhostAlwaysSet();
	for(SimSet::iterator itr = scopeAlwaysSet->begin(); itr != scopeAlwaysSet->end(); itr++)
	{
		TerrainBlock* block = dynamic_cast<TerrainBlock*>(*itr);
		if( block )
			mCurrentTerrain = block;
	}
	//Now, fill your active terrains array with NULLs, so you can use it to determine whether or 
	//not you have a terrain loaded into the grid location in question.
	for (U32 i=0;i<mNumGridRows;i++)
	{
		for (U32 j=0;j<mNumGridColumns;j++)
		{
			mActiveTerrains.increment();
			mActiveTerrains.last() = NULL;
			mLoadingTerrains.increment();
			mLoadingTerrains.last() = false;
		}
	}
	//HERE: find the player starting point, find that point in the full grid of possible terrains.
	//mActiveTerrains[0] = mCurrentTerrain;//But for now, we know where we're starting, work with it.

	Con::executef(this, "onAdd", getIdString());
   return true;
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
		return;//If an error condition results in clientPos = (0,0,0), bail.

	//clientPos = clientPosVec[0];//TEMP!  Need to do loop below for each clientPos, but for now just do it for 
	//the first.
	//////////////////////////////////////////////////////
	Con::printf("client pos: %f %f %f  cameraInControl: %d numCameras %d",
			clientPos.x,clientPos.y,clientPos.z,cameraInControl,kCameras.size());

	F32 blockSize = mWorldBlockSize - mSquareSize;
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

			S32 terrGridColumn = (S32)(terrPos.x/(mWorldBlockSize-mSquareSize));
			S32 terrGridRow = (S32)(terrPos.y/(mWorldBlockSize-mSquareSize));		
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
	mCurrentTick++;
}

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

ConsoleMethod(TerrainMaster, checkTerrain,void, 2, 2, "")
{
	object->checkTerrain();
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
