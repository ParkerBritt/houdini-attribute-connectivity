#include <GA/GA_Types.h>
#include <gtest/gtest.h>
#include "SOP_AttributeConnectivity.h"
#include <set>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <GA/GA_Primitive.h>
#include <GU/GU_Grid.h>

// set up paths
std::filesystem::path project_directory = "..";

std::filesystem::path geometry_path_rel = "tests/unit-tests/geometry";
std::filesystem::path geometry_path_full = project_directory / geometry_path_rel;

TEST(Setup, geometry){
    ASSERT_TRUE(std::filesystem::exists(geometry_path_full));
}

TEST(HDK, createGDP){
    GU_Detail gdp;

    UT_Vector3 pos(1.0, 2.0, 3.0);
    GA_Offset ptoff = gdp.appendPoint();
    gdp.setPos3(ptoff, pos);
    ASSERT_EQ(gdp.getPos3(ptoff).x(), pos.x());
    ASSERT_EQ(gdp.getPos3(ptoff).y(), pos.y());
    ASSERT_EQ(gdp.getPos3(ptoff).z(), pos.z());


    GA_RWHandleF attrHandle = gdp.addFloatTuple(GA_ATTRIB_POINT, "example", 1);
    attrHandle.set(ptoff, 0, 1.0f);

    float val = attrHandle.get(ptoff, 0);
    ASSERT_EQ(val, 1.0f);

    // gdp.save("/home/s5709975/MyRepos/ASE-assignment/build/export.bgeo", nullptr);
}

void makeGrid(GU_Detail *gdp, uint width, uint height){
    auto addPt = [&gdp](float posx, float posy, float posz) {
        GA_Offset ptoff = gdp->appendPoint();
        UT_Vector3 pos(posx, posy, posz);
        gdp->setPos3(ptoff, pos);
        return ptoff;
    };

    GA_Offset pt1 = addPt(0, 0, 0);
    GA_Offset pt2 = addPt(1, 0, 0);
    GA_Offset pt3 = addPt(0, 1, 0);
    GA_Offset pt4 = addPt(1, 1, 0);
    
    GEO_PrimPoly *prim_poly_ptr = static_cast<GEO_PrimPoly*>(gdp->appendPrimitive(GA_PRIMPOLY));
    prim_poly_ptr->appendVertex(pt1);
    prim_poly_ptr->appendVertex(pt3);
    prim_poly_ptr->appendVertex(pt4);
    prim_poly_ptr->appendVertex(pt2);
    prim_poly_ptr->close();
};

TEST(SOP_AttributeConnectivity, getNeighbourPtOffsets){
    GU_Detail *gdp = new GU_Detail();

    // load file
    std::filesystem::path import_geo_path = geometry_path_full / "grid_10x10_before.geo";
    std::filesystem::path export_geo_path = project_directory / "build/export.bgeo";
    ASSERT_TRUE(std::filesystem::exists(import_geo_path));
    gdp->load(import_geo_path.c_str());

    std::vector<std::pair<int, std::vector<int>>> expectedIndexPairs{
        {0, { 1, 10, 11 }},
        {1, { 0, 2 , 10 ,11, 12}}
    };

    for(std::pair<int, std::vector<int>> pair : expectedIndexPairs){
        int currentPointNumber = pair.first;
        std::vector<int> expectedIndices = pair.second;
        GA_Offset ptoff = gdp->pointOffset(currentPointNumber);
        GA_OffsetArray neighbourPtOffsets = SOP_AttributeConnectivity::getNeighbourPtOffsets(gdp, ptoff);

        size_t i = 0;
        ASSERT_EQ(neighbourPtOffsets.size(), expectedIndices.size());

        for(auto it = neighbourPtOffsets.begin(); it!=neighbourPtOffsets.end(); ++it){
            GA_Index ptNum = gdp->pointIndex(*it);
            size_t expectedNum = expectedIndices[i];

            ASSERT_EQ(ptNum, expectedNum);

            ++i;
        }
    }



    // gdp->save(export_geo_path.c_str(), nullptr);
}

TEST(SOP_AttributeConnectivity, floodFill){
    GU_Detail *gdp = new GU_Detail();

    // load file
    std::filesystem::path import_geo_path = geometry_path_full / "grid_10x10_before.geo";
    ASSERT_TRUE(std::filesystem::exists(import_geo_path));
    gdp->load(import_geo_path.c_str());

    // get in attribute
    GA_Attribute *inAttr = gdp->findPointAttribute("density");
    auto inAttrHandle = std::make_shared<GA_ROHandleF>(inAttr);
    ASSERT_TRUE(inAttrHandle->isValid());

    GA_Offset seedOffset = gdp->pointOffset(0);
    
    std::unordered_set<GA_Offset> resultOffsets = SOP_AttributeConnectivity::floodFill(gdp, seedOffset, inAttrHandle, 1.0f);
    std::unordered_set<GA_Offset> expectedOffsets{
        0, 1 , 2, 3,
        10,11,12,13,
        20,21,22,23,
        30,31,32,33
    };

    ASSERT_EQ(resultOffsets.size(), expectedOffsets.size());
    // check result matches expected values
    for(GA_Offset offset: expectedOffsets){
        ASSERT_TRUE(resultOffsets.count(offset)); 
    }

    // test under thresh
    resultOffsets = SOP_AttributeConnectivity::floodFill(gdp, seedOffset, inAttrHandle, 0.9f);
    ASSERT_EQ(resultOffsets.size(), expectedOffsets.size());

    // test over thresh
    resultOffsets = SOP_AttributeConnectivity::floodFill(gdp, seedOffset, inAttrHandle, 1.1f);
    ASSERT_EQ(resultOffsets.size(), 1);


}

TEST(SOP_AttributeConnectivity, floodFillGroup){
    GU_Detail *gdp = new GU_Detail();

    // load file
    std::filesystem::path import_geo_path = geometry_path_full / "grid_10x10_activegrp.geo";
    ASSERT_TRUE(std::filesystem::exists(import_geo_path));
    gdp->load(import_geo_path.c_str());

    // create group
    auto ptGrp = gdp->findPointGroup("active");

    // get in attribute
    GA_Attribute *inAttr = gdp->findPointAttribute("density");
    auto inAttrHandle = std::make_shared<GA_ROHandleF>(inAttr);
    ASSERT_TRUE(inAttrHandle->isValid());

    GA_Offset seedOffset = gdp->pointOffset(0);
    
    std::unordered_set<GA_Offset> resultOffsets;
    std::unordered_set<GA_Offset> expectedOffsets;

    resultOffsets = SOP_AttributeConnectivity::floodFill(gdp, seedOffset, inAttrHandle, 1.0f);

    expectedOffsets = {
        0, 1 , 2, 3,
        10,11,12,13,
        20,21,22,23,
        30,31,32,33
    };

    ASSERT_EQ(resultOffsets.size(), expectedOffsets.size());
    // check result matches expected values
    for(GA_Offset offset: expectedOffsets){
        ASSERT_TRUE(resultOffsets.count(offset)); 
    }

    expectedOffsets =  {
        0, 1 ,
        10,11,
    };

    resultOffsets = SOP_AttributeConnectivity::floodFill(gdp, seedOffset, inAttrHandle, 1.0f, ptGrp);
    ASSERT_EQ(resultOffsets.size(), expectedOffsets.size());
    // check result matches expected values
    for(GA_Offset offset: expectedOffsets){
        ASSERT_TRUE(resultOffsets.count(offset)); 
    }
}
