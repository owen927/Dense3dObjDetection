#include "Sample.h"
#include "dataFilter.h"
// Include Dense3D API header file
#include <Dense3D.h>
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <iostream>


#define BUFSIZE 512

#define WIDTH    320
#define HEIGHT    240
#define FPS        30

Vec3b HSV2RGB(float hue, float sat, float val)
{
    float x, y, z;

    if (hue == 1) hue = 0;
    else         hue *= 6;

    int i = static_cast<int>(floorf(hue));
    float f = hue - i;
    float p = val * (1 - sat);
    float q = val * (1 - (sat * f));
    float t = val * (1 - (sat * (1 - f)));

    switch (i)
    {
    case 0: x = val; y = t; z = p; break;
    case 1: x = q; y = val; z = p; break;
    case 2: x = p; y = val; z = t; break;
    case 3: x = p; y = q; z = val; break;
    case 4: x = t; y = p; z = val; break;
    case 5: x = val; y = p; z = q; break;
    }
    return Vec3b((uchar)(x * 255), (uchar)(y * 255), (uchar)(z * 255));
}

string doubleToString(double num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

int main(int argc, TCHAR* argv[])
{
    HANDLE hPipe;
    LPTSTR lpvMessage = TEXT("default msg");
    TCHAR  chBuf[BUFSIZE];
    BOOL   fSuccess = FALSE;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

    if (argc > 1)
        lpvMessage = argv[1];

    // Try to open a named pipe; wait for it, if necessary. 

    while (1)
    {
        hPipe = CreateFile(
            lpszPipename,   // pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE,
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file 

        // Break if the pipe handle is valid. 

        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
            return -1;
            system("pause");
        }

        // All pipe instances are busy, so wait for 20 seconds. 

        if (!WaitNamedPipe(lpszPipename, 20000))
        {
            printf("Could not open pipe: 20 second wait timed out.");
            return -1;
            system("pause");
        }
    }

    // build color lookup table for depth display
    Mat colorLut = Mat(cv::Size(256, 1), CV_8UC3);
    for (int i = 0; i < 256; i++)
        colorLut.at <Vec3b> (i) = (i == 0) ? Vec3b(0, 0, 0) : HSV2RGB(i / 256.0f, 1, 1);

    printf("DUOLib Version:       v%s\n", GetLibVersion());
    printf("Dense3D Version:      v%s\n", Dense3DGetLibVersion());

    // Open DUO camera and start capturing
    if (!OpenDUOCamera(WIDTH, HEIGHT, FPS))
    {
        printf("Could not open DUO camera\n");
        return 0;
    }

    Dense3DInstance dense3d;
    if (!Dense3DOpen(&dense3d))
    {
        printf("Could not open Dense3D library\n");
        // Close DUO camera
        CloseDUOCamera();
        return 0;
    }

    if (!SetDense3DLicense(dense3d, "FRYTM-35TU6-O4FQK-Y6APA-OP8V1")) // Get your license from duo3d.com/account
    {
        printf("Invalid Dense3D license\n");
        // Close DUO camera
        CloseDUOCamera();
        // Close Dense3D library
        Dense3DClose(dense3d);
        return 0;
    }
    if (!SetDense3DImageSize(dense3d, WIDTH, HEIGHT))
    {
        printf("Invalid image size\n");
        // Close DUO camera
        CloseDUOCamera();
        // Close Dense3D library
        Dense3DClose(dense3d);
        return 0;
    }
    // Get DUO calibration intrinsics and extrinsics
    DUO_STEREO params;
    if (!GetCameraStereoParameters(&params))
    {
        printf("Could not get DUO camera calibration data\n");
        // Close DUO camera
        CloseDUOCamera();
        // Close Dense3D library
        Dense3DClose(dense3d);
        return 1;
    }
    // Set Dense3D parameters
    SetDense3DScale(dense3d, 3);
    SetDense3DMode(dense3d, 0);
    SetDense3DCalibration(dense3d, &params);
    SetDense3DNumDisparities(dense3d, 4);
    SetDense3DSADWindowSize(dense3d, 6);
    SetDense3DPreFilterCap(dense3d, 28);
    SetDense3DUniquenessRatio(dense3d, 27);
    SetDense3DSpeckleWindowSize(dense3d, 52);
    SetDense3DSpeckleRange(dense3d, 14);

    // Set exposure, LED brightness and camera orientation
    SetExposure(75);
    SetLed(25);
    SetVFlip(true);
    // Enable retrieval of undistorted (rectified) frames
    SetUndistort(true);

    // Create Mat for left & right frames
    Mat left = Mat(Size(WIDTH, HEIGHT), CV_8UC1, NULL);
    Mat right = Mat(Size(WIDTH, HEIGHT), CV_8UC1, NULL);

    // Create Mat for disparity and depth map
    Mat1f disparity = Mat(Size(WIDTH, HEIGHT), CV_32FC1);
    Mat3f depth3d = Mat(Size(WIDTH, HEIGHT), CV_32FC3);

    float z ;
    float count ;
    double average_depth;
    string temp;

    // Run capture loop until  key is pressed
    while ((cvWaitKey(1) & 0xff) != 27)
    {
        // Capture DUO frame
        PDUOFrame pFrameData = GetDUOFrame();
        if (pFrameData == NULL) continue;

        // Set the image data
        left.data = (uchar*)pFrameData->leftData;
        right.data = (uchar*)pFrameData->rightData;

        // Process Dense3D depth map here
        if (Dense3DGetDepth(dense3d, pFrameData->leftData, pFrameData->rightData,
            (float*)disparity.data, (PDense3DDepth)depth3d.data))
        {
            uint32_t disparities;
            GetDense3DNumDisparities(dense3d, &disparities);
            Mat disp8;
            disparity.convertTo(disp8, CV_8UC1, 255.0 / (disparities * 16));
            Mat mRGBDepth;
            cvtColor(disp8, mRGBDepth, COLOR_GRAY2BGR);
            LUT(mRGBDepth, colorLut, mRGBDepth);
            imshow("Dense3D Disparity Map", mRGBDepth);

                     
            //send over processed data via named pipe
            temp = doubleToString(objDetection1(depth3d));

            dwMode = PIPE_READMODE_MESSAGE;
            fSuccess = SetNamedPipeHandleState(
                hPipe,    // pipe handle 
                &dwMode,  // new pipe mode 
                NULL,     // don't set maximum bytes 
                NULL);    // don't set maximum time 
            if (!fSuccess)
            {
                _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
               
            }
            
            // Send a message to the pipe server. 
            lpvMessage = (LPTSTR)temp.c_str();
            cbToWrite = (lstrlen(lpvMessage) + 1)*sizeof(TCHAR);
            _tprintf(TEXT("Sending %d byte message, Depth Vaule: \"%s\"\n"), cbToWrite, lpvMessage);

            fSuccess = WriteFile(
                hPipe,                  // pipe handle 
                lpvMessage,             // message 
                cbToWrite,              // message length 
                &cbWritten,             // bytes written 
                NULL);                  // not overlapped 

            if (!fSuccess)
            {
                _tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());

            }
            do
            {
                // Read from the pipe. 

                fSuccess = ReadFile(
                    hPipe,    // pipe handle 
                    chBuf,    // buffer to receive reply 
                    BUFSIZE*sizeof(TCHAR),  // size of buffer 
                    &cbRead,  // number of bytes read 
                    NULL);    // not overlapped 

                if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
                    break;

                _tprintf(TEXT("Server received depth: \"%s\"\n"), chBuf);
            } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

            if (!fSuccess)
            {
                _tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
            }

                
            
        }
        // Display images
        imshow("Left Image", left);
        imshow("Right Image", right);
    }
    // Close DUO camera
    CloseDUOCamera();
    // Close Dense3D library
    Dense3DClose(dense3d);
    system("pause");
    return 0;
}