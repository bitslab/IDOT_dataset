This is a dataset containing trajectory of traffic videos from Illinois Department of Transportation.  
It contains 13 5-mins long videos with ground truth.  
Stored by Git Large File Storage (LFS). 

The annotation tool is [Vatic](https://github.com/jinyanzi/vatic.git) with a little modification of data format.

Authors:  
Yanzi Jin (yjin25@uic.edu)  
Jakob Eriksson (jakob@uic.edu)  
Computer Science Department  
University of Illinois at Chicago



The format of the ground truth is 
```
object_id x y width height frame_id if_lost if_occluded if_interpolated label
```
, where `if_interpolated` comes from the annotation tool and may not be useful.


To use this dataset, please cite the paper  
```
@article{yanziVehicleTracker, 
	 title={Fully Automatic, Real-Time Vehicle Tracking for Surveillance Video}, 
	 author={Jin, Yanzi and Jakob, Eriksson},
	 journal={Computer and Robotic Vision},
	 year={2017},
}
```
