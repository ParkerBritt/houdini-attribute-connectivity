import unittest
import pytest
import hou
import os

def setup_test_nodes(input_geo="testgeometry_pighead", make_connections=True):
    hou.hipFile.clear(suppress_save_prompt=True)
    parent_node = hou.node("/obj").createNode("geo")

    input_node = parent_node.createNode(input_geo)
    test_node = parent_node.createNode("attribconnectivity")

    if(make_connections):
        test_node.setFirstInput(input_node)

    nodes = {
            "test_node":test_node,
            "input_node":input_node,
            "parent_node":parent_node,
            }
    return nodes

def export_scene(export_path=None):
    if(export_path is None):
        export_path = os.path.join(os.getenv("HOME"), "tmp" ,"exportfile.hip")
    print("exporting to:", export_path)
    hou.hipFile.save(export_path)

def test_ctor_no_input():
    # test constructor
    test_node = setup_test_nodes()["test_node"]
    try:
        test_node.cook()
    except hou.OperationFailed as e:
        # test errors
        assert test_node.errors() == ("Not enough sources specified.",)


def test_ctor():
    test_node = setup_test_nodes()["test_node"]

    # test type
    assert isinstance(test_node, hou.SopNode)

    # test name
    assert test_node.name() == "attribconnectivity1"



def test_ctor_input():
    parent_node = hou.node("/obj").createNode("geo")

    input_node = parent_node.createNode("testgeometry_pighead")
    test_node = parent_node.createNode("attribconnectivity")

    test_node.setFirstInput(input_node)

    test_node.cook()

def test_ctor_output():
    parent_node = hou.node("/obj").createNode("geo")

    input_node = parent_node.createNode("testgeometry_pighead")
    test_node = parent_node.createNode("attribconnectivity")

    test_node.setFirstInput(input_node)

    input_pt_cnt = len(input_node.geometry().iterPoints())
    test_node_pt_cnt = len(test_node.geometry().iterPoints())
    assert len(input_node.geometry().iterPoints()) > 0
    assert test_node_pt_cnt == input_pt_cnt

# def test_attribute_set(self):
#     test_node = setup_test_nodes(input_geo="box")["test_node"]

#     for pt in test_node.geometry().iterPoints():
#         self.assertEqual(pt.intAttribValue("index"), 1)

def test_attribute_connectivity():
    input_geo_list = ["box"]
    for geo in input_geo_list: # test multiple types of geometry
        for i in range(5): # multiple iterations to check for an inconsistent bug I was getting
            nodes = setup_test_nodes(input_geo=geo, make_connections=False)
            input_node = nodes["input_node"]
            test_node = nodes["test_node"]
            acreate_node = nodes["parent_node"].createNode("attribcreate::2.0")


            acreate_node.setFirstInput(nodes["input_node"])
            acreate_node.setParms({
                "group":"0 6",
                "grouptype":"points",
                "name1":"density",
                "value1v1":1
                })
            test_node.setFirstInput(acreate_node)
            test_node.setParms({
                "attribname":"density"
            })


            test_node.cook()
            if(test_node.warnings()!=()):
                print("warning on run:", i)
                pytest.fail("node has warnings:"+ str(test_node.warnings()))

def test_before_after_geo():
    # setup nodes
    nodes = setup_test_nodes(input_geo="file")
    before_node = nodes["input_node"]
    after_node = nodes["parent_node"].createNode("file")
    test_node = nodes["test_node"]
    test_node.setParms({
        "attribname":"density"
    })

    file_path = os.path.realpath(__file__)
    tests_path = os.path.normpath(os.path.join(file_path, "../.."))
    # import geometry
    before_node.setParms({
        "file":os.path.join(tests_path, "unit-tests/geometry/grid_10x10_before.geo")
    })
    after_node.setParms({
        "file":os.path.join(tests_path, "unit-tests/geometry/grid_10x10_after.geo")
    })

    # test geometry is valid
    test_geo = test_node.geometry()
    assert test_geo.isValid()
    reference_geo = after_node.geometry()
    assert test_geo.isValid()

    test_geo_points = test_geo.points()
    reference_geo_points = reference_geo.points()

    # test same amount of points
    len(test_geo_points) == len(reference_geo_points)

    # get attribs
    density_attrib = reference_geo.findPointAttrib("density")
    index_attrib = reference_geo.findPointAttrib("index")

    # test attribs exist on reference
    assert density_attrib is not None
    assert index_attrib is not None
    # test attribs exist on connectivity sop
    assert test_geo.findPointAttrib("index")  is not None
    assert test_geo.findPointAttrib("density")  is not None

    for i, test_pt in enumerate(test_geo_points):
        reference_pt = reference_geo_points[i]
        assert test_pt.attribValue("density") == reference_pt.attribValue("density")
        assert test_pt.attribValue("index") == reference_pt.attribValue("index")




        
# tests different values for the input attribute
def test_inattr_parameters():
    in_names = {"density", "foo", "bar", "hello_world"}
    out_names = {"index", "foo", "bar", "hello_world"}
    # setup nodes
    geo="box"
    nodes = setup_test_nodes(input_geo=geo, make_connections=False)
    input_node = nodes["input_node"]
    test_node = nodes["test_node"]
    acreate_node = nodes["parent_node"].createNode("attribcreate::2.0")


    acreate_node.setFirstInput(nodes["input_node"])
    acreate_node.setParms({
        "group":"0 6",
        "grouptype":"points",
        "value1v1":1
        })
    test_node.setFirstInput(acreate_node)

    for in_name in in_names:
        acreate_node.setParms({"name1":in_name})
        test_node.setParms({"attribname":in_name})
        # test geometry is valid
        test_geo = test_node.geometry()
        assert test_geo.isValid()
        assert test_geo.isValid()

        # test attribs exist on connectivity sop
        assert test_geo.findPointAttrib("index") is not None
        assert test_geo.findPointAttrib(in_name) is not None

    in_name = "attrib"
    test_node.setParms({"attribname":in_name})
    for out_name in out_names:
        test_node.setParms({"outputindexattrib":out_name})

        # test geometry is valid
        test_geo = test_node.geometry()
        assert test_geo.isValid()
        assert test_geo.isValid()

        # test attribs exist on connectivity sop
        assert test_geo.findPointAttrib(out_name) is not None

# test that the appropriate error appears when output parm is empty
def test_empty_index_parm():
    in_name = "density"
    out_names = ("")
    # setup nodes
    geo="box"
    nodes = setup_test_nodes(input_geo=geo, make_connections=False)
    input_node = nodes["input_node"]
    test_node = nodes["test_node"]
    acreate_node = nodes["parent_node"].createNode("attribcreate::2.0")


    acreate_node.setFirstInput(nodes["input_node"])
    acreate_node.setParms({
        "group":"0 6",
        "grouptype":"points",
        "value1v1":1
        })
    test_node.setFirstInput(acreate_node)

    acreate_node.setParms({"name1":in_name})
    test_node.setParms({"attribname":in_name})

    for out_name in out_names:
        # set output to blank
        test_node.setParms({"outputindexattrib":out_name})
        try:
            test_node.cook()
        except hou.OperationFailed as e:
            # test errors
            assert test_node.errors() == ('Invalid attribute specification: "Index name cannot be empty".',)

def test_grp():
    in_name = "density"
    # setup nodes
    geo="box"
    nodes = setup_test_nodes(input_geo=geo, make_connections=False)
    input_node = nodes["input_node"]
    test_node = nodes["test_node"]
    acreate_node = nodes["parent_node"].createNode("attribcreate::2.0")


    acreate_node.setFirstInput(nodes["input_node"])
    acreate_node.setParms({
        "group":"0 6",
        "grouptype":"points",
        "value1v1":1
        })
    test_node.setFirstInput(acreate_node)

    acreate_node.setParms({"name1":in_name})
    test_node.setParms({
        "attribname":in_name,
        "group":""})

    test_geo = test_node.geometry()
    assert test_geo.point(0).intAttribValue("index") == 1
    assert test_geo.point(6).intAttribValue("index") == 2

    test_node.setParms({
        "attribname":in_name,
        "group":"0"})

    assert test_geo.point(0).intAttribValue("index") == 1
    assert test_geo.point(6).intAttribValue("index") == 0 


def test_include_island_parm_off():
    # setup nodes
    geo="file"
    nodes = setup_test_nodes(input_geo=geo)
    input_node = nodes["input_node"]

    # load comparison file
    file_path = os.path.realpath(__file__)
    tests_path = os.path.normpath(os.path.join(file_path, "../.."))
    geo_path = os.path.join(tests_path, "unit-tests/geometry/grid_10x10_singlepatch_activegrp.geo")
    input_node.setParms({
        "file":geo_path
    })

    # test file exists
    assert os.path.exists(geo_path) is True

    test_node = nodes["test_node"]
    test_node.setParms({"attribname":"density", "group":"0 1 11 10"})
    include_island_parm = test_node.parm("groupincludewholeisland")

    # check for errors or warnings
    test_node.cook()
    assert test_node.warnings() == ()
    assert test_node.errors() == ()


    # check initial parm state
    assert include_island_parm is not None
    assert include_island_parm.evalAsInt() == 0

    assert test_node.geometry().findPointAttrib("index") is not None

    expected_pts = {
        0, 1 ,
        10,11,
    }
    indexed_pts = set()


    test_pts = test_node.geometry().points()
    for pt in test_pts:
        if(pt.intAttribValue("index")!=0):
            indexed_pts.add(pt.number())

    assert indexed_pts == expected_pts

def test_include_island_parm_on():
    # setup nodes
    geo="file"
    nodes = setup_test_nodes(input_geo=geo)
    input_node = nodes["input_node"]

    # load comparison file
    file_path = os.path.realpath(__file__)
    tests_path = os.path.normpath(os.path.join(file_path, "../.."))
    geo_path = os.path.join(tests_path, "unit-tests/geometry/grid_10x10_singlepatch_activegrp.geo")
    input_node.setParms({
        "file":geo_path
    })

    # test file exists
    assert os.path.exists(geo_path) is True

    test_node = nodes["test_node"]
    test_node.setParms({"attribname":"density", "group":"0 1 11 10"})
    include_island_parm = test_node.parm("groupincludewholeisland")

    # check for errors or warnings
    test_node.cook()
    assert test_node.warnings() == ()
    assert test_node.errors() == ()


    # check initial parm state
    assert include_island_parm is not None
    include_island_parm.set(1)
    assert include_island_parm.evalAsInt() == 1

    assert test_node.geometry().findPointAttrib("index") is not None

    expected_pts = {
        0, 1 , 2, 3,
        10,11,12,13,
        20,21,22,23,
        30,31,32,33
    }
    indexed_pts = set()


    test_pts = test_node.geometry().points()
    for pt in test_pts:
        if(pt.intAttribValue("index")!=0):
            indexed_pts.add(pt.number())

    assert indexed_pts == expected_pts

def test_threshold_parm():
    # setup nodes
    geo="file"
    nodes = setup_test_nodes(input_geo=geo)
    input_node = nodes["input_node"]

    # load comparison file
    file_path = os.path.realpath(__file__)
    tests_path = os.path.normpath(os.path.join(file_path, "../.."))
    geo_path = os.path.join(tests_path, "unit-tests/geometry/grid_10x10_singlepatch_activegrp.geo")
    input_node.setParms({
        "file":geo_path
    })

    # test file exists
    assert os.path.exists(geo_path) is True

    test_node = nodes["test_node"]
    test_node.setParms({"attribname":"density"})

    # test parm exists
    threshold_parm = test_node.parm("threshold")
    assert threshold_parm is not None
    # assert threshold_parm.evalAsFloat() == 1 # check default

    # check for errors or warnings
    test_node.cook()
    assert test_node.warnings() == ()
    assert test_node.errors() == ()

    # check attrib exists
    assert test_node.geometry().findPointAttrib("index") is not None

    test_pts = test_node.geometry().points()
    

    threshold_parm.set(0.9)
    expected_pts = {
        0, 1 , 2, 3,
        10,11,12,13,
        20,21,22,23,
        30,31,32,33
    }
    indexed_pts = set()
    for pt in test_pts:
        if(pt.intAttribValue("index")!=0):
            indexed_pts.add(pt.number())

    assert indexed_pts == expected_pts


    threshold_parm.set(1.1)
    indexed_pts = set()
    for pt in test_pts:
        if(pt.intAttribValue("index")!=0):
            indexed_pts.add(pt.number())
    assert len(indexed_pts) == 0
