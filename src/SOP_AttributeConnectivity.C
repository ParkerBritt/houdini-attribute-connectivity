#include "SOP_AttributeConnectivity.h"
#include <string>
#include <set>
#include <stack>
#include <unordered_set>

#include <GU/GU_Detail.h>
#include <OP/OP_Operator.h>
#include <OP/OP_AutoLockInputs.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <SYS/SYS_Math.h>
#include <stddef.h>
#include <memory>

void newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(
        // use of raw pointers to adhere to sidefx standards
        new OP_Operator(
            "attribconnectivity",
            "Attribute Connectivity",
            SOP_AttributeConnectivity::myConstructor,
            SOP_AttributeConnectivity::m_templateList,
            1,
            1,
            0
        )
    );
}

// constructor definition
SOP_AttributeConnectivity::SOP_AttributeConnectivity(OP_Network *net, const char *name, OP_Operator *op) : SOP_Node(net, name, op)
{
    mySopFlags.setManagesDataIDs(true);
}

// destructor definition
SOP_AttributeConnectivity::~SOP_AttributeConnectivity() {}


// constructor static member function
OP_Node *SOP_AttributeConnectivity::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    // use of raw pointers to adhere to sidefx standards
    return new SOP_AttributeConnectivity(net, name, op);
}

GA_OffsetArray SOP_AttributeConnectivity::getNeighbourPtOffsets(const GU_Detail *_gdp, const GA_Offset _ptoff){
    // ------
    // as a side effect of using sets the output will be ordered
    // ------

    GA_OffsetArray primitives;
    _gdp->getPrimitivesReferencingPoint(primitives, _ptoff);
    std::set<GA_Offset> pointOffsetBuffer;

    for(int i=0; i<primitives.size(); i++){
        GA_Offset prim_offset = primitives[i];

        const GA_Primitive *prim = _gdp->getPrimitive(prim_offset);

        GA_Range vertRange = prim->getVertexRange();
        for(auto it = vertRange.begin(); it!=vertRange.end(); ++it){
            GA_Offset vertOffset = *it;
            GA_Offset pointOffset = _gdp->vertexPoint(vertOffset);
            if(pointOffset == _ptoff) continue; // dont include self as a neighbour
            pointOffsetBuffer.insert(pointOffset);
        }

    }
    
    // populate output array with set objects
    GA_OffsetArray neighbourPtOffsets;
    for( GA_Offset pointOffset : pointOffsetBuffer ){
        neighbourPtOffsets.append(pointOffset);
    }
    return neighbourPtOffsets;
}

// TODO: alternate return value if seedOffset doesn't match thresh
std::unordered_set<GA_Offset> SOP_AttributeConnectivity::floodFill(const GU_Detail *_gdp, const GA_Offset _seedOffset, std::shared_ptr<GA_ROHandleF> _attribHandle, const float _attrThresh, const GA_PointGroup *_group){
    // this set is used to keep track of aready traversed points to avoid extra calculation
    std::unordered_set<GA_Offset> traversedOffsets;

    bool useGrps = _group != nullptr;

    std::stack<GA_Offset> offsetBuffer;

    // set starting point in buffer
    offsetBuffer.push(_seedOffset);
    traversedOffsets.insert(_seedOffset);

    // iterate over all recursive neighbours
    while(!offsetBuffer.empty())
    {
        // fetch current pt off stack
        GA_Offset curOffset = offsetBuffer.top();
        offsetBuffer.pop();

        
        // get neighbours around current point
        GA_OffsetArray neighbours = getNeighbourPtOffsets(_gdp, curOffset);
        // add valid neighbours to stack
        for(auto it = neighbours.begin(); it!=neighbours.end(); ++it)
        {
            GA_Offset neighbourOff = *it;
            float neighbourAttr = _attribHandle->get(neighbourOff);

            // skip invalid points
            if(
                neighbourAttr < _attrThresh || // skip neighbouring points that aren't in the island
                traversedOffsets.count(neighbourOff)==1 || // or already traversed
                (useGrps && !_group->contains(neighbourOff)) // or not in group
            )
            {
                continue;
            }   
            
            // remember point has been seen
            traversedOffsets.insert(neighbourOff);
            // add neighbour to be calculated later
            offsetBuffer.push(neighbourOff);



        }
    }

    return traversedOffsets;
}


// cook instructions
OP_ERROR SOP_AttributeConnectivity::cookMySop(OP_Context &context)
{


    fpreal t = context.getTime();

    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    duplicatePointSource(0, context);

    // point group
    cookInputPointGroups(
        context, // this is needed for cooking the group parameter, and cooking the input if alone.
        m_ptGrp, // the group (or NULL) is written to m_ptGrp if not alone.
        0   // this is true if called outside of cookMySop to update handles.
    );

    // read attribute name from parameter
    UT_String in_attrib_name;
    evalString(
        in_attrib_name, // where to store parameter value
        m_parm_names[0], // parameter name or input
        0,
        t // time to evaluate parameter
    );
    // read attribute name from parameter
    UT_String index_attrib_name;
    evalString(
        index_attrib_name, // where to store parameter value
        m_parm_names[1], // parameter name or input
        0,
        t // time to evaluate parameter
    );
    bool grp_whole_islands = evalInt(
        m_parm_names[2], // where to store parameter value
        0, // vi
        t // time to evaluate parameter
    );
    float threshold_parm_val = evalFloat(
        m_parm_names[3], // where to store parameter value
        0, // vi 
        t // time to evaluate parameter
    );

    // check for empty name 
    if(index_attrib_name==""){
        
        addError(SOP_ATTRIBUTE_INVALID, std::string("Index name cannot be empty").c_str());
        return error();
    } 
    // create index attribute
    GA_RWHandleI indexAttrHandle = gdp->addIntTuple(GA_ATTRIB_POINT, index_attrib_name, 1);
    if (!indexAttrHandle.isValid())
    {
        addError(SOP_ATTRIBUTE_INVALID, std::string("Attribute "+index_attrib_name.toStdString()+" is invalid").c_str());
        return error();
    }
    // get in attribute
    GA_Attribute *inAttr = gdp->findPointAttribute(in_attrib_name);
    auto inAttrHandle = std::make_shared<GA_ROHandleF>(inAttr);
    if (!inAttrHandle->isValid())
    {
        addWarning(SOP_ATTRIBUTE_INVALID, std::string("Attribute "+in_attrib_name.toStdString()+" either not supplied or unrecognized").c_str());
        return error();
    }


    std::unordered_set<GA_Offset> traversedOffsets;

    // flood fill doesn't take groups into account if grp_whole_islands is true
    const GA_PointGroup *fillGrp = (grp_whole_islands) ? nullptr : m_ptGrp;

    // iterate over all points
    GA_Offset ptOff;
    float inThresh = threshold_parm_val;
    int maxIndex = 1;
    GA_FOR_ALL_GROUP_PTOFF(gdp, m_ptGrp, ptOff)
    {
        

        float densityVal = inAttrHandle->get(ptOff);
        if(
            densityVal<inThresh || 
            traversedOffsets.count(ptOff)
        )
        {
            continue;
        }

        // remember that the point has been traversed
        traversedOffsets.insert(ptOff);

        std::unordered_set<GA_Offset> islandOffsets = floodFill(gdp, ptOff, inAttrHandle, inThresh, fillGrp);
        for(auto it = islandOffsets.begin(); it!=islandOffsets.end(); ++it){
            indexAttrHandle.set(*it, maxIndex);
        }
        ++maxIndex;
        traversedOffsets.merge(islandOffsets);

    }
    indexAttrHandle.bumpDataId();


    return error();
}

PRM_Name SOP_AttributeConnectivity::m_parm_names[] = {
    PRM_Name("attribname",  "Attribute Name"),
    PRM_Name("outputindexattrib",  "Output Index Attribute"),
    PRM_Name("groupincludewholeisland",  "Include Whole Islands"),
    PRM_Name("threshold",  "Threshold"),
};

PRM_Default SOP_AttributeConnectivity::m_defaults[] = {
    PRM_Default(0, "index"),
    PRM_Default(1),
};

// blank template list
PRM_Template SOP_AttributeConnectivity::m_templateList[] = {
    PRM_Template(
        PRM_STRING,                 // parm type
        1,                          // number of items
        &PRMgroupName,              // parameter name
        0,                          // default value
        &SOP_Node::pointGroupMenu,  // drop down meu
        0,                          // range
        0,                          // callback
        SOP_Node::getGroupSelectButton(GA_GROUP_POINT) // select points button
    ),
    PRM_Template(PRM_TOGGLE, 1, &m_parm_names[2]),
    PRM_Template(PRM_FLT, 1, &m_parm_names[3], &m_defaults[1]),
    PRM_Template(PRM_STRING, 1, &m_parm_names[0], 0, &SOP_Node::pointAttribMenu),
    PRM_Template(PRM_STRING, 1, &m_parm_names[1], &m_defaults[0]),
    PRM_Template(),
};

