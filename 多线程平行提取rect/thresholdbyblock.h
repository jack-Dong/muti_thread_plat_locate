#ifndef LOCATE_THRESHOD
#define LOCATE_THRESHOD
#include"base.h"
/**
   将灰度图像进行局部二值化（平均值），据计算非0像素
   输入：灰度图像 img, 垂直分块个数nblockht,水平分块个数 nblockwd
   输出：二值化后的图像
*/
Mat locate_threshold(Mat image, unsigned int nblockwd, unsigned int nblockht)
{
	//参数检查
	Mat img = image.clone();
	Size s = img.size();
	if (nblockht > s.height  ||  nblockwd > s.width)
	{
		cout << "参数错误：图像尺寸太小。" << endl;
		exit(-1);
	}
	//取出每个块中的像素 并计算
	vector<Mat> blocks; //块
	unsigned scalewd = s.width / nblockwd; //水平刻度
	unsigned scaleht = s.height / nblockht; //垂直刻度
	unsigned begin_x = 0;
	unsigned begin_y = 0;
	Rect rio;
	Mat imgRIO;
	float medim = 0, sum = 0;
	unsigned nozeros = 0;
	uchar pixe;
	//cout << "水平分块个数：" << nblockwd <<"  " << "垂直分块个数：" << nblockht <<endl;
	for (size_t i = 0; i < nblockwd * nblockht; i++)
	{
		//获得起点
		begin_x = ( i % nblockwd ) * scalewd;
		begin_y = ( i / nblockwd ) * scaleht;
		rio.x = begin_x;
		rio.y = begin_y;
		rio.width = scalewd;
		rio.height = scaleht;
		imgRIO = img(rio); //取出兴趣区域
		//cout <<"兴趣区域：" <<rio <<endl;
		//计算兴趣区域的中值 和 非零像素个数
		medim = 0;
	    sum  = 0;
		nozeros = 0;
		for (size_t i = 0; i < scaleht; i++)
		{
			for (size_t j = 0; j < scalewd; j++)
			{
				pixe =  imgRIO.at<uchar>(i, j);
				if ( pixe > 0)
				{
					sum = sum + (unsigned int)pixe;
					nozeros++;
				}
			}
		}
		//二值化
		medim = sum / nozeros;
		for (size_t i = 0; i < scaleht; i++)
		{
			for (size_t j = 0; j < scalewd; j++)
			{
				pixe = imgRIO.at<uchar>(i, j);
				if (pixe >= medim -1) //防止全部都是255
				{
					imgRIO.at<uchar>(i, j) = 255;
				}
				else
				{
					imgRIO.at<uchar>(i, j) = 0;
				}
			}
		}
		////自动阈值
		//threshold(imgRIO, imgRIO, 0, 255, THRESH_BINARY | THRESH_OTSU);
	}
	return img;
}

/**
	通过局部中值进行二值化
	输入：灰度图像image_gray
	输出：二值化后的图像
*/
Mat middleThresh(Mat image_gray)
{
	Mat img_gray = image_gray;
	uchar pixe;
	int medim = 0;
	int sum  = 0;
	int nozeros = 0;
	for (size_t i = 0; i < img_gray.rows; i++)
	{
		for (size_t j = 0; j < img_gray.cols; j++)
		{
			pixe =  img_gray.at<uchar>(i, j);
			if ( pixe > 0)
			{
				sum = sum + (unsigned int)pixe;
				nozeros++;
			}
		}
	}
	//二值化
	medim = sum / nozeros;
	for (size_t i = 0; i < img_gray.rows; i++)
	{
		for (size_t j = 0; j < img_gray.cols; j++)
		{
			pixe = img_gray.at<uchar>(i, j);
			if (pixe >= medim -1) //防止全部都是255
			{
				img_gray.at<uchar>(i, j) = 255;
			}
			else
			{
				img_gray.at<uchar>(i, j) = 0;
			}
		}
	}
	return img_gray;
}


#endif