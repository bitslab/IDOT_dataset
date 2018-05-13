#include <iostream>
#include <string>
#include <unistd.h>
#include "visualizer.hpp"

using namespace std;

// input path
std::string video_path,		
			ground_truth_path, 
			output_video_name;
int to_frame_num = -1;
bool batch_mode = false;

void print_help() {
	cout << "./visualizer\n\t\t-i <video_name>\n\t\t"
		 << "-g <ground_truth_file>\n\t\t"
		 // output files
		 << "-v <output_video_path> [output video path]"
		 << "-f <to_frame_num> [number of frame to process]\n\t\t"
		 << "-m [batch mod]\n\t\t"
		 << endl;
}

void parse_opts(int argc, char ** argv, Visualizer & visualizer) {

	if(argc < 3){
		print_help();
		exit(1);
	}

	int opt;
	
	while((opt = getopt(argc, argv, "i:g:f:v:m")) != -1) {
		switch(opt) {
		case 'i':
			video_path = optarg;
			break;
		case 'g':
			ground_truth_path = optarg;
			visualizer.readGroundTruthFromFile(ground_truth_path);
			break;
		case 'v':
			output_video_name = optarg;
			break;
		case 'f':
			to_frame_num = atoi(optarg);
			break;
		case 'm':
			batch_mode = true;
			break;
		default:
			break;
		}
	}

}


int main(int argc, char** argv){

	Visualizer visualizer;
	parse_opts(argc, argv, visualizer);

	// write display video to file
	if(!output_video_name.empty())	
		visualizer.saveToVideo(video_path, output_video_name, to_frame_num);


	string line;
	if(!batch_mode) {
		while(1){
			cout << "\n"
				 << "1. Visualize ground truth\n   To visualize a certain object, type [1 <start_frame> <obj_id>]\n"
				 << "2. Print ground truth summary for all objects.\n"
				 << "3. Print ground truth detail.\n"
				 << "4. save to video file.\n   If no output video path provided, write to the current directoty with the input video name.\n" 
				 << "0. Exit" << endl;
			int gt, n, s = 0, id = -1;
			getline(std::cin, line);
			stringstream ss(line);
			ss >> n;
			switch(n){
				case 1:
					if(ss)	ss >> s;
					if(ss)	ss >> id;
					if(!video_path.empty())
						visualizer.visualize(video_path, s, id);
					else
						cerr << "No video file input" << endl;
					break;
				case 2:
					visualizer.printGroundTruthSummary();    
					break;
				case 3:
					cout << "Please enter a ground truth number." << endl;
					cin >> gt;
					cin.ignore();
					visualizer.printGroundTruthSummary(gt); 
					break;
				case 4:
					visualizer.saveToVideo(video_path, output_video_name, to_frame_num);
					break;
				case 0:
					return 0;

			}

		}
	}

}
