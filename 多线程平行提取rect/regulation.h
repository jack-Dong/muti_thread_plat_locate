#ifndef REGULATION
#define REGULATION
#include"base.h"

/*
计算灰度图像的直方图
输入：灰度图像
输出：1 * 256 的 直方图Mat
*/
Mat hist_get(Mat &gray)
{
	const int channels = { 0 };
	Mat hist;
	const int hist_size[1] = { 256 };
	const float range[2] = { 0, 255 };
	const float *ranges[1] = { range };
	calcHist(&gray, 1, channels, Mat(), hist, 1, hist_size, ranges);
	return hist;
}

/*
	得到一张灰度图片的期望和方差
	输入：Mat
	输出：vector<double> nv; nv[0] 是期望 nv[1] 是方差
*/
vector<double> get_nv(Mat m)
{
	vector<double> rt;
	//先得到直方图
	Mat hist = hist_get(m);
	//得到概率密度函数
	hist = hist / (m.cols * m.rows) * 1.0;
	Mat hist_prob = hist.clone(); //保存概率密度函数方便方差的计算
	//计算期望
	//cout <<"列数："<<hist.cols << "   " <<"行数："<< hist.rows << endl;
	for (size_t i = 0; i < hist.rows; i++)
	{
		//cout << hist.at<float>(i) <<endl;
		hist.at<float>(i) = hist.at<float>(i) * i;
	}
	//cout << hist << endl;
	Scalar means = sum(hist);
	rt.push_back(means[0]); //到此期望计算成功
	//cout << rt[0];
	//计算标准差
	Mat variance1(1, 256, CV_32F,Scalar::all(0));
	for (size_t i = 0; i < variance1.cols; i++)
	{
		variance1.at<float>(i) =  pow((i - means[0]), 2) * hist_prob.at<float>(i);
		//cout << pow((pow((i - means[0]), 2) * hist_prob.at<float>(i)), 0.5) <<endl;
	}
	//cout << variance1 << endl;
	Scalar variance = sum(variance1);
	//cout << variance;
	rt.push_back( sqrt( variance[0] )); //到此标准差计算结束
	return rt;
}

/*
	高斯查找表函数
	输入：vector<double> nv（nv[0] 是期望 nv[1] 是方差）, 亮度扩展因子k1,对比度扩展因子 k2,原灰度图像 img
	输出：高斯函数映射 组映射后的查找表 Mat lup(1, 256, CV_32F, Scalar::all(0));
*/
Mat lookup( vector<double> nv,double k1,double k2,Mat img)
{
	Mat lup(1, 256, CV_8U, Scalar::all(0)); //最后的查找表
	Mat src(1, 256, CV_32F, Scalar::all(0)); //原图像概率密度直方图
	Mat dest(1, 256, CV_32F, Scalar::all(0)); //目标图像概率密度直方图
	//得到高斯映射的概率直方图（即规定图像 目标图像） 并均衡化
	for (size_t i = 0; i < 256; i++)
	{
		dest.at<float>(i) = (1 / (sqrt(2 * PI) * k2 * nv[1])) * exp((-pow(i - k1 * nv[0], 2)) / (2.0 * pow(k2 * nv[1], 2)));
	}
	//cout << dest << endl;
	for (size_t i = 0; i < 255; i++)
	{
		dest.at<float>(i + 1) = dest.at<float>(i) +dest.at<float>(i + 1);
	}
	//cout << dest << endl;
	//拉伸到0到1
	dest = dest / dest.at<float>(255);
	//cout << dest << endl;
	//cout << "目标图像累计概率密度直方图:";
	//cout << dest << endl;
	//得到原图像的概率直方图 并均衡化
	src = hist_get(img);
	src = src / (img.cols * img.rows * 1.0 );
	for (size_t i = 0; i < 255; i++)
	{
		src.at<float>(i + 1) = src.at<float>(i) +src.at<float>(i + 1);
	}
	//cout << "原图像累计概率密度直方图:";
	//cout << src <<endl;
	//计算原始图像 到 目标图 像累积概率直方图 各灰度级的差的绝对值, 存放在一个二维矩阵里
	Mat diff(256, 256, CV_32F, Scalar::all(0));
	for(size_t j = 0; j < 256; j++)
	{
		for (size_t i = 0; i < 256; i++)
		{
			diff.at<float>(i,j) = fabs( dest.at<float>(i) - src.at<float>(j));
		}
	}
	
	//通过组映射得到目标图像的查找表
	int lastStartY = 0; int lastEndY = 0; int startY = 0; int endY = 0;
	double minvalue;
	for (size_t i = 0; i < 256; i++)
	{
		minvalue = diff.at<float>(i,0);
		for (size_t j = 1; j < 256; j++)
		{
			if (minvalue > diff.at<float>(i,j))
			{
				minvalue = diff.at<float>(i, j);
				endY = j;
			}
		}
		if (startY != lastStartY || endY != lastEndY)
		{   
			for (size_t k = startY; k <= endY; k++)
			{
				lup.at<uchar>(k) = i;
			}
			lastStartY = startY;
			lastEndY = endY;
			startY = lastEndY + 1;
		}
	}
	//cout <<"映射到的最后的位置："<< endY <<endl;
	//把后面的添上
	for (size_t i = endY + 1; i < 256; i++)
	{
		lup.at<uchar>(i) = lup.at<uchar>(endY);
	}
	//cout << "查找表：";
	//cout << lup<< endl;
	return lup;
}

/*
运用高斯函数求得概率密度函数 再用组映射得到增强后的图像
输入：原始图像(可以是彩色) img  亮度扩展因子k1,对比度扩展因子 k2
输出增强后的图像 rt
*/
Mat regulate(Mat &img, double k1, double k2)
{
	//cout <<"img.channels()="  <<img.channels()<<endl;
	//判定是否需要分离通道
	if (img.channels() == 3)
	{
		cvtColor(img,img,CV_BGR2HSV);
		//分离通道分别处理
		vector<Mat> planes;
		split(img, planes);
		//imshow("V", planes[2]);
		//获得其期望和方差
		vector<double> nv2 = get_nv(planes[2]);
		//创建查找表
		Mat lup2(1, 256, CV_32F, Scalar::all(0));
		//填充查找表
		//cout << lup.cols << "   " << lup.rows << endl;
		lup2 = lookup(nv2, k1, k2, planes[2]); //调用查找函数
		//应用查找表
		LUT(planes[2], lup2, planes[2]);
		//输出结果图像
		merge(planes, img);
		cvtColor(img,img, CV_HSV2BGR);
	}
	else if(img.channels() == 1)
	{
		vector<double> nv = get_nv(img);
		//创建查找表
		Mat lup(1, 256, CV_32F, Scalar::all(0));
		//填充查找表
		//cout << lup.cols << "   " << lup.rows << endl;
		lup = lookup(nv, k1, k2, img); //调用查找函数
		//应用查找表
		LUT(img, lup, img);
	}
	return img;
}

/*
把一张灰度图像的直方图以图像化的形式展现出来 此函数计算这张图
*/
Mat hist_image(Mat hist)
{
	double max_value = 0; 
	//找到直方图当中的最大值和最小值
	minMaxLoc(hist, NULL, &max_value, 0, 0);
	Mat hist_image(hist.rows, hist.rows * 5, CV_8U, Scalar(255)); //纯白画布
	//设置最大峰为图像高度的90%
	int hpt = static_cast<int>(0.9 * hist.rows); //static_cast强制类型转换
	for (int h = 0; h < hist.rows * 5; h = h + 5)
	{
		float bin_val = hist.at<float>(h / 5);
		//int intensity = static_cast<int>(bin_val*hpt / max_value); 
		int intensity = (int)(bin_val*hpt / max_value);
		for (int i = 0; i < 5; i++)
		{
			line(hist_image, Point(h + i, hist.rows), Point(h + i, hist.rows - intensity), Scalar::all(0)); //图像是以左上角为坐标原点
		}

	}
	return hist_image;
}

/**
  就像局部阈值一样 图像进行局部或者全局增强
  输入：原图像img; 亮度扩展因子k1; 对比度扩展因子k2; 是否需要分块 isneeddivideblocks; 垂直分块个数nblockht;水平分块个数 nblockwd 
  输出：增强后的图片 rt
*/
Mat regulated(Mat image,float k1, float k2,bool isneeddivideblocks = false,unsigned int nblockwd = 0, unsigned int nblockht = 0)
{
	//参数检查
	Mat img = image.clone();
	Size s = img.size();
	if (nblockht > s.height  ||  nblockwd > s.width)
	{
		cout << "参数错误：图像尺寸太小。" << endl;
		exit(-1);
	}
	if (k1 < 0  || k2 < 0)
	{
		cout << "参数错误：k1,k2扩展因子不能为负" << endl;
		exit(-1);
	}
	if (isneeddivideblocks)
	{
		//取出每个块中的像素 并计算
		vector<Mat> blocks; //块
		unsigned scalewd = s.width / nblockwd; //水平刻度
		unsigned scaleht = s.height / nblockht; //垂直刻度
		unsigned begin_x = 0;
		unsigned begin_y = 0;
		Rect rio;
		Mat imgRIO;
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
			imgRIO = regulate(imgRIO,k1,k2);
		}
	}else
	{
		//全局增强
	   img = regulate(img,k1,k2);
	}
	return img;
}
#endif //!REGULATION