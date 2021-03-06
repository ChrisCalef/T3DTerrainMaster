//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------


new TerrainMaterial()
{
   internalName = "rocktest";
   diffuseMap = "art/terrains/Example/rocktest";
   detailMap = "art/terrains/Example/rocktest_d";
   detailSize = "10";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "400";
};

new TerrainMaterial()
{
   internalName = "grass1-dry";
   diffuseMap = "art/terrains/Example/grass1-dry";
   detailMap = "art/terrains/Example/grass1-dry_d";
   detailSize = "10";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "250";
   detailStrength = "2";
};

new TerrainMaterial()
{
   internalName = "dirt_grass";
   diffuseMap = "art/terrains/BAG_TP/512/Grass/Grass12";
   detailMap = "art/terrains/BAG_TP/512/Grass/Grass12_d";
   detailSize = "5";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "200";
   normalMap = "art/terrains/BAG_TP/512/Grass/Grass12_NRM";
};

new TerrainMaterial()
{
   internalName = "sand";
   diffuseMap = "art/terrains/Example/sand";
   detailMap = "art/terrains/Example/sand_d";
   detailSize = "10";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "200";
};


new TerrainMaterial()
{
   diffuseMap = "art/terrains/BAG_TP/512/Ice_Snow/Snow02";
   diffuseSize = "200";
   detailMap = "art/terrains/Example/grass1_d";
   detailSize = "10";
   internalName = "snowtop";
   isManaged = "1";
   detailBrightness = "1";
   enabled = "1";
};

new TerrainMaterial()
{
   diffuseMap = "art/terrains/WorldServer/city1";
   internalName = "city1";
};

new TerrainMaterial()
{
   diffuseMap = "art/terrains/WorldServer/forest1a";
   internalName = "forest1a";
};

new TerrainMaterial()
{
   diffuseMap = "art/terrains/WorldServer/drycrop1";
   internalName = "drycrop1";
};

new TerrainMaterial()
{
   diffuseMap = "art/terrains/WorldServer/sand_hires";
   internalName = "sand_hires";
};
