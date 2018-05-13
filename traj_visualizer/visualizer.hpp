#ifndef _GT_VISUALIZER_H
#define _GT_VISUALIZER_H

#define RED cv::Scalar(0, 0, 255)
#define GREEN cv::Scalar(0, 255, 0)
#define BLUE cv::Scalar(255, 0, 0)
#define YELLOW cv::Scalar(0, 255, 255)
#define PINK cv::Scalar(255, 0, 255)
#define CYAN cv::Scalar(255, 255, 0)

#define DEBUG 1

#define TOP_LEFT 0
#define BOTTOM_RIGHT 1
#define STATIONARY_DIST 8

#include <vector>
#include <map>
#include <string>
#include <opencv2/opencv.hpp>


struct TrajUnit{
	TrajUnit(int f_id = -1, const cv::Rect& r = cv::Rect(), int occluded = 0, const std::string& c = ""):frame_id(f_id), box(r), if_occluded(occluded), cls(c){}
	int frame_id;
	cv::Rect box;
	int if_occluded;
	std::string cls;
};


typedef std::vector<TrajUnit> TrajVec;

class Visualizer{
protected:
	// draw ground truth and box
    // obj_id = -1, draw all the objects
    void drawGroundTruth(cv::Mat& img, int to_frame_num, const cv::Scalar& color, int thickness, int location = TOP_LEFT, int obj_id = -1)	const;
    
	// draw ground truth and box of a certain object
    // return the index of the object at to_frame_num
    int drawObject(cv::Mat& img, const std::map<int, TrajVec>::const_iterator it, int to_frame_num, const cv::Scalar& color, int thicknes = 1,  int location = TOP_LEFT)	const;
    
	void drawNumBox(cv::Mat& img, const cv::Rect& r, const cv::Scalar& color, int num,  int location = TOP_LEFT)	const;

	void readGroundTruthLine(const std::string& line, int& obj_id, int& frame_id, cv::Rect& box, std::string& cls, int& occluded)	const;
    
    
	int getObjectNum()	const {	return ground_truth.size();	}

	float boxDist(const cv::Rect& r1, const cv::Rect& r2)	const {	return cv::norm(center(r1)-center(r2));	}

    cv::Point2f center(const cv::Rect& r) const{  return cv::Point2f(r.x+r.width/2, r.y+r.height/2); }

    // variables
	std::map<int, TrajVec> ground_truth;

public:

    // read functions
	// ground truth is written in object id order
	// to_frame_num = -1 means read until end of file
	bool readGroundTruthFromFile(const std::string& filename, int to_frame_num = -1);
	// print function
    void printGroundTruthSummary(int obj_id = -1)	const; 

	std::string getFileNameFromPath(const std::string& file_path) const;
	// save to video file
    bool saveToVideo(const std::string& video_name, std::string& out_video_name, int to_frame = -1)	const;

	bool visualize(const std::string& video_name, int start_frame, int obj_id)	const;
};
	


#endif
