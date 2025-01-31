import hou
import sys

parent_node = hou.node("/obj").createNode("geo")

input_node = parent_node.createNode("testgeometry_pighead")
test_node = parent_node.createNode("attribconnectivity")
export_node = parent_node.createNode("rop_geometry")

test_node.setFirstInput(input_node)
export_node.setFirstInput(test_node)

print("exporting to:", sys.argv[1])
export_node.setParms({"sopoutput": sys.argv[1]})
export_node.parm("execute").pressButton()


