#include "visualizer.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>

using namespace std;
using namespace cv;

bool Visualizer::readGroundTruthFromFile(const string& filename, int to_frame_num) {
	if(filename.empty()) {
		cerr << "Filename empty" << endl;
		return false;
	}
	ifstream infile(filename.c_str());
	if(!infile.is_open()){
		cerr << "Failed to open file " << filename << endl;
		return false;
	}

	// Read trajectory data
	std::string line;
	int last_obj_id = -1;	
	TrajVec tmp_trajectory;

	while(!infile.eof()){
		// if the string keep content of last frame
		// use the last one without readling a new line
		std::getline(infile, line);
		if(line.empty())	continue;

		int tmp_obj_id, tmp_frame_id, tmp_occluded = 0;
		Rect tmp_box;
		string tmp_cls;
        // do not read objects appeared after to_frame_num
		readGroundTruthLine(line, tmp_obj_id, tmp_frame_id, tmp_box, tmp_cls, tmp_occluded);
        if(last_obj_id < 0)
            last_obj_id = tmp_obj_id;
        //cout << " frame_id " << tmp_frame_id << " object " << tmp_obj_id << endl;

        // if the current frame exceeds the to_frame_num, skip it for other objects
        if( to_frame_num > 0 && tmp_frame_id > to_frame_num )
            continue;

        // If the line is of next object, insert the trajectory to the map
        if( tmp_obj_id != last_obj_id ){
            // do not insert objects appeared after to_frame_num
            if( !tmp_trajectory.empty() ){
                    ground_truth.insert(std::make_pair(last_obj_id, tmp_trajectory));
                // clear tmp_trajectory for next object
                tmp_trajectory.clear();
            }
        }// add trajectory unit

        // append trajectory record for the current object
        if(tmp_frame_id <= to_frame_num || to_frame_num == -1) 
            tmp_trajectory.push_back(TrajUnit(tmp_frame_id, tmp_box, tmp_occluded, tmp_cls));
        last_obj_id = tmp_obj_id;
	}

	if(!tmp_trajectory.empty())
		ground_truth.insert(std::make_pair(last_obj_id, tmp_trajectory));

	infile.close();
	cout << "read " << ground_truth.size() << " objects to ground truth" << endl;
	// end of file, return true
	return true;
}


void Visualizer::readGroundTruthLine(const string& line, int& obj_id, int& frame_id, Rect& box, string& cls, int & occluded)	const {
	char* tokens = NULL;
	const char* delims = " ";
	int x = 0, y = 0, w = 0, h = 0;

	tokens = strtok( strdup(line.c_str()), delims );
	int j = 0;
    while( tokens != NULL ){
        // [obj_num, xmin, ymin, xmax, ymax, frame_num]
        switch(j){
            // trajectory format: obj_id, x_top_left, y_top_left, width, height, frame_id
            // changed to be the same with 
            case 0: obj_id = atoi(tokens); break;
            case 1: x = atoi(tokens);	break;
            case 2: y = atoi(tokens);	break;
            case 3: w = atoi(tokens);	break;
            case 4: h = atoi(tokens);	break;
            case 5: frame_id = atoi(tokens);	break;
			case 6: occluded = atoi(tokens);
			// old ground truth has if_occluded in the 8th column, 
			// new ground truth has if_occluded in the 7th column
			case 7: if(isdigit(tokens[0])) occluded = atoi(tokens); else cls = tokens;	break;
			case 9: cls = tokens; break;
            default:	break;
        }

        tokens = strtok(NULL, delims);
		j++;
	}
	box = Rect(x,y,w,h);
}

void Visualizer::printGroundTruthSummary(int obj_id)	const{
    cout << "Ground Truth" << endl;
    // obj_id == -1, print summary
    if(obj_id == -1){
		for(const auto& traj: ground_truth)
            cout << "object " << traj.first << " (" << traj.second.front().frame_id << "-" 
                << traj.second.back().frame_id << "|" << traj.second.size() << ")" << endl;

        cout << "----------------------------------------------------------------\n" 
            << ground_truth.size() << " objects in total " << endl;
    }// print object trajectory
    else{
		int k = -1;
		float max_dist = FLT_MIN;
        std::map<int, TrajVec>::const_iterator it = ground_truth.find(obj_id);
        if( it != ground_truth.end() ){
			for(unsigned int i = 0; i < it->second.size(); ++i) {
				if(i > 0) {
					float s = boxDist(it->second.at(i-1).box, it->second.at(i).box);
					cout << "-- " << s << " --> ";
					if(s > max_dist) {
						max_dist = s;
						k = i;
					}
				}
                cout << it->second.at(i).frame_id << " " << it->second.at(i).box;
			}
            cout << endl;

			cout << "\nLifetime: " << it->second.size();
			if(it->second.empty())
				cout << endl;
			else
				cout << " Frame " << it->second.front().frame_id << "-" << it->second.back().frame_id << endl;
			if( k > 0)
				cout << "max dist " << it->second.at(k-1).frame_id << "-" << it->second.at(k).frame_id << ": " << max_dist << endl;
        }
    }
}

void Visualizer::drawNumBox(cv::Mat& img, const Rect& r, const Scalar& color, int num, int location)	const {
	Mat overlay = img.clone();
	float alpha = 0.6;
    Point p;
    if(location == TOP_LEFT){
        p = r.tl();
        p.x += 3;
        p.y += 8;
    }else{
        p = r.br();
        p.x -= 8;
        p.y -= 8;
    }
    // draw number
    stringstream ss;
    ss << num;

    rectangle(img, r, color, -1);
	addWeighted(overlay, alpha, img, 1-alpha, 0, img);
	if(num >= 0)
    putText(img, ss.str(), p, FONT_HERSHEY_SIMPLEX, 0.4, color);
}

int Visualizer::drawObject( cv::Mat& img, const std::map<int, TrajVec>::const_iterator it, int to_frame_num, const cv::Scalar& color, int thickness, int location)	const {
   
    // do not draw objects have not appeared or left 
    if( it->second.size() < 2 || it->second.front().frame_id > to_frame_num 
            || it->second.back().frame_id < to_frame_num )
        return -1;
    TrajVec::const_iterator it_traj = it->second.begin(), it_last_traj = it_traj++;
    for(; it_traj != it->second.end(); ++it_traj){ 
        if( it_traj->box != Rect() && it_last_traj->box != Rect() ){
            line(img, center(it_traj->box), center(it_last_traj->box), color, thickness); 
            it_last_traj = it_traj;
        }// only draw trajcetory up to to_frame_num
        if(it_traj->frame_id >= to_frame_num)
            break;
    }
    // draw box
    if( it_traj != it->second.end() && it_traj->frame_id == to_frame_num) {
		drawNumBox(img, it_traj->box, color, it->first, location);
	}
#if DEBUG
    // first box
    if( it->second.front().frame_id == to_frame_num )
        cout << "\nobject " << it->first << "(" << it->second.front().frame_id << "-" << it->second.back().frame_id << "):\t" << it->second.front().box << endl;
#endif

    // return the index of object at to_frame_num
    return it_last_traj - it->second.begin();
}

void Visualizer::drawGroundTruth(cv::Mat& img, int to_frame_num, const cv::Scalar& color, int thickness, int location, int obj_id)	const {
    
	stringstream ss;
    ss << "frame " << to_frame_num;
    putText(img, ss.str(), Point(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, GREEN);

	// return if the trajectory is empty
	if(ground_truth.empty())	return;

    // draw all the objects
    if(obj_id == -1 ){
        for(std::map<int, TrajVec>::const_iterator it = ground_truth.begin(); 
                it != ground_truth.end(); it++ ) {
			Scalar c = color;
			// if the trajectory is matched to a ground truth
			if(!it->second.empty() && it->second.front().cls == "\"people\"")
				c = PINK;

            drawObject(img, it, to_frame_num, c, thickness, location);
		}
    }// draw only one object
    else{
        const std::map<int, TrajVec>::const_iterator it = ground_truth.find(obj_id);
        if(it != ground_truth.end()) {
			Scalar c = color;
			if(!it->second.empty() && it->second.front().cls == "\"people\"")
				c = PINK;

            drawObject(img, it, to_frame_num, c, thickness, location);
		}
    }
}



string Visualizer::getFileNameFromPath(const string& file_path)   const{
    size_t s1 = file_path.find_last_of("/");
    size_t s2 = file_path.rfind(".");
    return file_path.substr(s1+1, s2-s1-1);
}


bool Visualizer::saveToVideo(const string& video_path, string& out_video_name, int to_frame)	const {
    VideoCapture capture(video_path);
    if(!capture.isOpened()){
        cerr << "Failed to open video " << video_path << endl;
        return false;
    }

    int frame_num = capture.get(CAP_PROP_FRAME_COUNT);
    double frame_rate = capture.get(CAP_PROP_FPS);
    int width = capture.get(CAP_PROP_FRAME_WIDTH);
	int height = capture.get(CAP_PROP_FRAME_HEIGHT);

	if(out_video_name.empty())
		out_video_name = getFileNameFromPath(video_path)+".mp4";

	VideoWriter writer(out_video_name, VideoWriter::fourcc('D', 'I', 'V', 'X'), frame_rate, Size(width, height), true);
    //VideoWriter writer(out_video_name, capture.get(CAP_PROP_FOURCC), frame_rate, Size(width, height), true);
    if(!writer.isOpened()){
        cerr << "Failed to write to video " << out_video_name << endl;
        return false;
    }

    Mat img;
	to_frame = (to_frame < 0 ? frame_num : to_frame);

    for(int i = 0; i < to_frame; ++i){
        capture >> img;

		if(img.empty())	continue;
		
		drawGroundTruth(img, i, GREEN, 1, BOTTOM_RIGHT);
        writer.write(img);
    }
    cout << "Write " << frame_num << " frames to video " << out_video_name << endl;

    return true;
}

bool Visualizer::visualize(const string& video_name, int start_frame, int obj_id)	const {
	VideoCapture capture(video_name);
	if(!capture.isOpened()){
		cerr << "Failed to open video " << video_name << endl;
		return false;
	}

	int frame_num = capture.get(CAP_PROP_FRAME_COUNT);
	int frame_rate = capture.get(CAP_PROP_FPS);

	bool play = false;
	int end_frame = frame_num-1;

	Mat img;
	std::map<int, TrajVec>::const_iterator it = ground_truth.find(obj_id);

	// if the object is found, set the start and end frame of the object life
	if(it != ground_truth.end()) {
		start_frame = it->second.front().frame_id;
		end_frame = it->second.back().frame_id;
	}else{	// if the object is not found, display all the objects
		cout << "Object " << obj_id << " is not found" << endl;
		obj_id = -1;
	}

	cout << frame_num << " frames, start from " << start_frame << endl;

	// if no object is specified, start from the first frame
	// otherwise, show the object frame by frame
	capture.set(CAP_PROP_POS_FRAMES, start_frame);
	for(int i = start_frame; i <= end_frame;){
		if(play || i == start_frame){
			capture >> img;
			// draw objects
			drawGroundTruth(img, i++, BLUE, 1, TOP_LEFT);
		}

		imshow("video", img);
		// if only show one object, show it frame by frame
		if(obj_id != -1)
			play = false;

		// keyboard input
		char c = waitKey(frame_rate);
		if(c == ' ')
			play = !play;
		if(c == 27)
			break;
	}
	cv::destroyWindow("video");
	return true;
}

