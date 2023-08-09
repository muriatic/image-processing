# image-processing
C++ Extension to Center Professional Headshots Relying on Face and Eye Detection

## Requirements:
- Opencv
- Opencv must be installed on C drive or changed in the code. 

## Input:
### Commandline arguments
- ```-name``` (person names separated by ';')
- ```-imageName``` (image names including file extension separated by ';')
- ```-sourceDir``` (where the image is stored)
- ```-finalDir``` (output directory)
- ```-threads``` (threads, must be less than available CPU threads)

## Output:
- Cropped PNG with "cropped_\<name\>.png" format
