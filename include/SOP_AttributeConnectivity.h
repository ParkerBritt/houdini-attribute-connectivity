#ifndef SOP_ATTRIBUTE_CONNECTIVITY_
#define SOP_ATTRIBUTE_CONNECTIVITY_

#include <SOP/SOP_Node.h>
#include <unordered_set>

// node class
class SOP_AttributeConnectivity : public SOP_Node 
{
public:
    // NOTE: use of static variables, in this case, adheres to SideFX HDK usage examples
    // use of static methods is necessary for unit testing without a full node definition,
    // which is only handled in a live Houdini session


    // default constructor and destructor
    SOP_AttributeConnectivity(OP_Network *net, const char *name, OP_Operator *op);
    ~SOP_AttributeConnectivity() override;

    // getneighbours
    static GA_OffsetArray getNeighbourPtOffsets(const GU_Detail *gdp, const GA_Offset ptoff);

    // flood fill islands
    static std::unordered_set<GA_Offset> floodFill(const GU_Detail *_gdp, const GA_Offset _seedOffset, std::shared_ptr<GA_ROHandleF> _attribHandle, const float _attrThresh, const GA_PointGroup *_group=nullptr);

    // constructor method
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);

    // parameter template list
    static PRM_Template m_templateList[];

    // parameter names
    static PRM_Name m_parm_names[];
    static PRM_Default m_defaults[];

protected:
    // protocols for cooking geometry
    OP_ERROR cookMySop(OP_Context &context) override;

    // group to iterate on
    const GA_PointGroup *m_ptGrp = nullptr;
};


#endif
