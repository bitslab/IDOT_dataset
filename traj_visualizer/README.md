C++ implementation of ground truth visualization.
This program includes reading ground truth trajectories and displaying them.

To run the visualizer:
- First complile
```
make
```

- Run the visualizer
```
./visualizer -i <video_path> -g <ground_truth_path> -v <output_video_path> -f <to_frame_num> -m
```

`video_path` and `ground_truth_path` are not optional.  
`-m` means batch mode without visualization.


Run `./visualizer` without parameter will print detailed parameter explanation.
