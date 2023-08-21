# image-processing
C++ Extension to Center Professional Headshots Relying on Face and Eye Detection

## Requirements:
- Opencv (found on latest release from their [GitHub Repo](https://github.com/opencv/opencv/tree/4.8.0))
- Opencv must be installed on C drive or changed in the code (check out this [installation tutorial](https://docs.opencv.org/4.x/dd/d6e/tutorial_windows_visual_studio_opencv.html))

## Input:
### Commandline arguments
- ```-name``` (person names separated by ';')
- ```-imageName``` (image names including file extension separated by ';')
- ```-sourceDir``` (where the image is stored)
- ```-finalDir``` (output directory)
- ```-threads``` (threads, must be less than available CPU threads)

## Output:
- Cropped PNG with "cropped_\<name\>.png" format
