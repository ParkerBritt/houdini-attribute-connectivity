<h1 align="center">Attribute Connectivity</h1>

<div align="center">
  <a href="https://www.sidefx.com/"><img src="https://parkerbritt.com/badge?label=HDK&icon=houdini&color=FF4713"></a>
  <a href="https://github.com/ParkerBritt?tab=repositories&q=&type=&language=c%2B%2B&sort="><img src="https://parkerbritt.com/badge?label=C%2B%2B&icon=cplusplus&color=00599C"></a>
  <a href="https://github.com/ParkerBritt?tab=repositories&q=&type=&language=python&sort="><img src="https://parkerbritt.com/badge?label=Python&icon=python&color=3776AB"></a><br>
  <img src="https://parkerbritt.com/jenkins_badge?job=HDK_attributeconnectivity">
</div><br>

<p align="center"> Attribute Connectivity is a <strong>Houdini</strong> surface operator for <strong>indexing attributes</strong> by connectivity.</p>     
<img src="screenshots/thumbnail.png">

# Overview
The Attribute Connectivity sop provides **connected component labeling** for Houdini attributes using flood fill analysis.
In other words it provides an index based on attribute values.
It achieves this by separating foreground and background components based on the threshold parameter, then assigning a unique index to each discontinuous island.


This node was written in *C++* using the *HDK*.

# Parameters
| **Parameter**              | **Type** | **Description**                                                                                                  |
|----------------------------|----------|---------------------------------------------------------------------------------------------------------|
| **Group**                  |String    | A subset of points in the input geometry to run the program on. Leave this blank to affect all points in the input. |
| **Include Whole Islands**  |Toggle    | Include islands that have at least one point in the group.                                                        |
| **Threshold**              |Float     | A threshold that separates the background and foreground values.                                                    |
| **Attribute Name**         |String    | Name of the attribute to analyze.                                                                                   |
| **Output Index Attribute** |String    | Name of the label attribute.                                                                                     |


# Build
### Dependencies
#### For Build
- Cmake
- ninja
- Houdini 20.5.X (other version are likely to work but untested)

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

# Example
### Input attribute visualized as color
![image](https://github.com/user-attachments/assets/f3cbe2d5-4c8a-4b47-9dc0-a1a58dcf44fb)
### Output indexing visualized as color
![image](https://github.com/user-attachments/assets/ce375d98-a9ba-4c87-bcd3-7aab488f2579)
### Output indexing values
![segmentation_example](https://github.com/user-attachments/assets/b99279e2-d80c-49e8-a11b-00a8b7753126)
