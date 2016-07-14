#include "dataFilter.h"
int limit = 1400;
int limit_cap = 45;

double objDetection(Mat3f data)
{
    double average_depth;
    double z = 0;
    int count = 0;
    for (int w = 130; w<190; w++)
    {
        for (int h = 90; h<150; h++)
        {
            if (!std::isinf(data[h][w][2]))
            {
                z += data[h][w][2];
                count++;
            }
            
        }
    }

    average_depth = z / count;
    printf("Average Center Depth: %f\n", average_depth);

    return average_depth;
}


double objDetection1(Mat3f data)
{
    

    double average_depth;
    double average_overlimt_depth;
    double z = 0;
    double z_overlimit = 0;
    int count = 0;
    int overlimit_count = 0;
    for (int w = 0; w<320; w++)
    {
        for (int h = 0; h<240; h++)
        {
            if (!std::isinf(data[h][w][2]))
            {
                z += data[h][w][2];
                count++;
                if (data[h][w][2] <= limit)
                {
                    z_overlimit += data[h][w][2];
                    overlimit_count++;
                }
            }

        }
    }

    average_depth = z / count;
    average_overlimt_depth = z_overlimit/overlimit_count;
  

    if (overlimit_count > 1000)
    {
        printf("Average Over limit Center Depth: %f\n", average_overlimt_depth);
        return average_overlimt_depth;
    }
    else
    {
        printf("Average Center Depth: %f\n", average_depth);
        return average_depth;
    }

}

double objDetection2(Mat3f data)
{
    //320x240
    double average_depth;
    double average_overlimt_depth;
    int x_inc = 40;
    int y_inc = 30;
    int x_count = 0;
    int y_count = 0;
    double z = 0;
    double z_overlimit = 0;
    double z_overlimit_temp = 0;
    int count = 0;
    int overlimit_count = 0;
    int overlimit_count_temp = 0;
    
    
    while (y_count < 240)
    {
        for (int w = x_count; w < x_count+x_inc; w++)
        {
            for (int h = y_count; h < y_count+y_inc; h++)
            {
                if (!std::isinf(data[h][w][2]))
                {
                    z += data[h][w][2];
                    count++;

                    if (data[h][w][2] <= limit)
                    {
                        z_overlimit_temp += data[h][w][2];
                        overlimit_count++;
                    }

                }

            }
            x_count += x_inc;
        }

        if (z_overlimit_temp > z_overlimit && overlimit_count_temp > limit_cap)
        {
            z_overlimit = z_overlimit_temp;
            overlimit_count = overlimit_count_temp;
        }

        x_count = 0;
        y_count += y_inc;
        z_overlimit_temp = 0;
        overlimit_count = 0;
    }
    average_depth = z / count;
    average_overlimt_depth = z_overlimit/
    printf("Average Center Depth: %f\n", average_depth);
    printf("Average overlimit depth: %f\n", average_overlimt_depth);

    if (overlimit_count > limit_cap)
        return average_overlimt_depth;
    else
        return average_depth;
}   

double objDetection3(Mat3f data) //wip
{
    double average_depth;
    double z = 0;
    int count = 0;
    for (int w = 130; w<190; w++)
    {
        for (int h = 90; h<150; h++)
        {
            if (!std::isinf(data[h][w][2]))
            {
                z += data[h][w][2];
                count++;
            }

        }
    }

    average_depth = z / count;
    printf("Average Center Depth: %f\n", average_depth);

    return average_depth;
}