<h1 align="center">Attribute Connectivity</h1>

<div align="center">
  <a href="https://www.sidefx.com/"><img src="https://img.shields.io/badge/-HDK-FF4713?style=for-the-badge&logo=houdini&logoColor=FF4713&labelColor=282828"></a>
  <a href="https://github.com/ParkerBritt?tab=repositories&q=&type=&language=c%2B%2B&sort="><img src="https://img.shields.io/badge/-C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=00599C&labelColor=282828"></a>
  <a href="https://github.com/ParkerBritt?tab=repositories&q=&type=&language=python&sort="><img src="https://img.shields.io/badge/-Python-3776AB?style=for-the-badge&logo=python&logoColor=3776AB&labelColor=282828"></a><br>
  <img src="https://parkerbritt.com/jenkins/buildStatus/icon?job=HDK_AttributeConnectivity&style=flat-square">
</div><br>

<p align="center"> Attribute connectivity is a <strong>Houdini</strong> SOP for <strong>indexing attributes</strong> by connectivity</p>     
<img src="https://github.com/user-attachments/assets/9357d6c7-57d7-4770-b05f-cf0ef25bf223">

# Overview
The Attribute Connectivity sop provides **connected component labeling** for Houdini attributes using flood fill analysis.
In other words it provides an index based on attribute values.
It achieves this by separating foreground and background components based on the threshold parameter, then assigning a unique index to each discontinuous island.


This node was written in C++ using the HDK.

# Parameters
| **Parameter**              | **Description**                                                                                                  |
|--------------------------|--------------------------------------------------------------------------------------------------------------------|
| **Group**               | A subset of points in the input geometry to run the program on. Leave this blank to affect all points in the input. |
| **Include Whole Islands** | Include islands that have at least one point in the group.                                                        |
| **Threshold**           | A threshold that separates the background and foreground values.                                                    |
| **Attribute Name**      | Name of the attribute to analyze.                                                                                   |
| **Output Index Attribute** | Name of the label attribute.                                                                                     |


# Build
### Dependencies
#### For Build
- Cmake
- ninja
- Houdini 20.5.X
> [!NOTE]
> other versions will very likely work but are untested
#### For Tests
- gtest
- python 3.11
- pytest

### Installation
To compile the program you can run the build script that's included.
```sh
./build.sh
```
The resulting .so file should be automatically placed in your ```$HOUDINI_USER_PREF_DIR/dso/``` directory.

Next copy ```./static/SOP_attribconnectivity.svg``` to ```$HOUDINI_USER_PREF_DIR/config/icons/```

# Class Diagram

```mermaid
classDiagram
    class SOP_AttributeConnectivity {
        - const GA_PointGroup* m_ptGrp
        + SOP_AttributeConnectivity(OP_Network*, const char*, OP_Operator*)
        + ~SOP_AttributeConnectivity()
        + static OP_Node* myConstructor(OP_Network*, const char*, OP_Operator*)
        + static GA_OffsetArray getNeighbourPtOffsets(const GU_Detail*, const GA_Offset)
        + static unordered_set<GA_Offset> floodFill(...)
        # OP_ERROR cookMySop(OP_Context&)
    }

    SOP_AttributeConnectivity --|> SOP_Node
```

# Example
### Input attribute visualized as color
![image](https://github.com/user-attachments/assets/f3cbe2d5-4c8a-4b47-9dc0-a1a58dcf44fb)
### Output indexing visualized as color
![image](https://github.com/user-attachments/assets/ce375d98-a9ba-4c87-bcd3-7aab488f2579)
### Output indexing values
![segmentation_example](https://github.com/user-attachments/assets/b99279e2-d80c-49e8-a11b-00a8b7753126)


# Reference
The HDK documentation and examples were used extensively in the development of this project.
