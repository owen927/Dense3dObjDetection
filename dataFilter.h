#ifndef DATAFILTER_H
#define DATAFILTER_H
#include "Sample.h"

// Include Dense3D API header file
#include <Dense3D.h>
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>



double objDetection(Mat3f data); /**basical averaging filter taking the center 60x60 data points then returns the average **/


double objDetection1(Mat3f data);/**return the average depth of points over limit if there are over limit_cap amount of data points over limit, 
                                 otherwise returns the average depth not over limit **/

double objDetection2(Mat3f data);/**divide the data into many different sets and return the average depth of the points over limit if a set has more than 30 samples over limit, 
                                 otherwise returns the average depth not over limit **/

double objDetection3(Mat3f data);/**search over the data and look for object/data point cluster thats over limit with size over x using DBSCAN, then returns the average depth of the data point cluster,
                                 therwise returns the average depth not over limit **/

#endif