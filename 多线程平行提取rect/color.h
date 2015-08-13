#ifndef COLOR
#define COLOR
#include"base.h"
Mat color_detect(Mat image)
{
	Mat img = image.clone();
	Mat rt(img.size(),CV_8U,Scalar(0));
	  if (image.channels() <  3)
    {
        cout << "输入的不是彩色图像" << endl;
		return rt;
    }
	cvtColor(img,img,CV_BGR2HSV); //转到HSV颜色空间
	Size size = img.size();
	uchar * src_data; //数据地址指针
	uchar * dst_data; //数据地址指针
	for (int i = 0; i < size.height; i++)
	{
		src_data = img.ptr<uchar>(i);
		dst_data = rt.ptr<uchar>(i);
		for (int j = 0; j < size.width * 3; j = j +3)
		{
			/*cout <<"<"<<(int)data[j]<<","<<(int)data[j+1]<<","<<(int)data[j+2]<<">" ;*/
			//检测蓝色 h 80->130 检测黄色 20->30
			if( (src_data[j]>= 90 && src_data[j] <= 130 && src_data[j+1] >= 0.2 * 255 && src_data[j+2] >= 0.17 * 255) ||  (src_data[j]>= 15 && src_data[j] <= 30 && src_data[j+1] >= 0.2 * 255  && src_data[j+2] >= 0.17 * 255))
			{
				dst_data[j/3] = 255;
			}
		}
		//cout <<"\n";
	}
	return rt;
}

#endif //!COLOR