#ifndef REGULATION
#define REGULATION
#include"base.h"

/*
����Ҷ�ͼ���ֱ��ͼ
���룺�Ҷ�ͼ��
�����1 * 256 �� ֱ��ͼMat
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
	�õ�һ�ŻҶ�ͼƬ�������ͷ���
	���룺Mat
	�����vector<double> nv; nv[0] ������ nv[1] �Ƿ���
*/
vector<double> get_nv(Mat m)
{
	vector<double> rt;
	//�ȵõ�ֱ��ͼ
	Mat hist = hist_get(m);
	//�õ������ܶȺ���
	hist = hist / (m.cols * m.rows) * 1.0;
	Mat hist_prob = hist.clone(); //��������ܶȺ������㷽��ļ���
	//��������
	//cout <<"������"<<hist.cols << "   " <<"������"<< hist.rows << endl;
	for (size_t i = 0; i < hist.rows; i++)
	{
		//cout << hist.at<float>(i) <<endl;
		hist.at<float>(i) = hist.at<float>(i) * i;
	}
	//cout << hist << endl;
	Scalar means = sum(hist);
	rt.push_back(means[0]); //������������ɹ�
	//cout << rt[0];
	//�����׼��
	Mat variance1(1, 256, CV_32F,Scalar::all(0));
	for (size_t i = 0; i < variance1.cols; i++)
	{
		variance1.at<float>(i) =  pow((i - means[0]), 2) * hist_prob.at<float>(i);
		//cout << pow((pow((i - means[0]), 2) * hist_prob.at<float>(i)), 0.5) <<endl;
	}
	//cout << variance1 << endl;
	Scalar variance = sum(variance1);
	//cout << variance;
	rt.push_back( sqrt( variance[0] )); //���˱�׼��������
	return rt;
}

/*
	��˹���ұ���
	���룺vector<double> nv��nv[0] ������ nv[1] �Ƿ��, ������չ����k1,�Աȶ���չ���� k2,ԭ�Ҷ�ͼ�� img
	�������˹����ӳ�� ��ӳ���Ĳ��ұ� Mat lup(1, 256, CV_32F, Scalar::all(0));
*/
Mat lookup( vector<double> nv,double k1,double k2,Mat img)
{
	Mat lup(1, 256, CV_8U, Scalar::all(0)); //���Ĳ��ұ�
	Mat src(1, 256, CV_32F, Scalar::all(0)); //ԭͼ������ܶ�ֱ��ͼ
	Mat dest(1, 256, CV_32F, Scalar::all(0)); //Ŀ��ͼ������ܶ�ֱ��ͼ
	//�õ���˹ӳ��ĸ���ֱ��ͼ�����涨ͼ�� Ŀ��ͼ�� �����⻯
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
	//���쵽0��1
	dest = dest / dest.at<float>(255);
	//cout << dest << endl;
	//cout << "Ŀ��ͼ���ۼƸ����ܶ�ֱ��ͼ:";
	//cout << dest << endl;
	//�õ�ԭͼ��ĸ���ֱ��ͼ �����⻯
	src = hist_get(img);
	src = src / (img.cols * img.rows * 1.0 );
	for (size_t i = 0; i < 255; i++)
	{
		src.at<float>(i + 1) = src.at<float>(i) +src.at<float>(i + 1);
	}
	//cout << "ԭͼ���ۼƸ����ܶ�ֱ��ͼ:";
	//cout << src <<endl;
	//����ԭʼͼ�� �� Ŀ��ͼ ���ۻ�����ֱ��ͼ ���Ҷȼ��Ĳ�ľ���ֵ, �����һ����ά������
	Mat diff(256, 256, CV_32F, Scalar::all(0));
	for(size_t j = 0; j < 256; j++)
	{
		for (size_t i = 0; i < 256; i++)
		{
			diff.at<float>(i,j) = fabs( dest.at<float>(i) - src.at<float>(j));
		}
	}
	
	//ͨ����ӳ��õ�Ŀ��ͼ��Ĳ��ұ�
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
	//cout <<"ӳ�䵽������λ�ã�"<< endY <<endl;
	//�Ѻ��������
	for (size_t i = endY + 1; i < 256; i++)
	{
		lup.at<uchar>(i) = lup.at<uchar>(endY);
	}
	//cout << "���ұ�";
	//cout << lup<< endl;
	return lup;
}

/*
���ø�˹������ø����ܶȺ��� ������ӳ��õ���ǿ���ͼ��
���룺ԭʼͼ��(�����ǲ�ɫ) img  ������չ����k1,�Աȶ���չ���� k2
�����ǿ���ͼ�� rt
*/
Mat regulate(Mat &img, double k1, double k2)
{
	//cout <<"img.channels()="  <<img.channels()<<endl;
	//�ж��Ƿ���Ҫ����ͨ��
	if (img.channels() == 3)
	{
		cvtColor(img,img,CV_BGR2HSV);
		//����ͨ���ֱ���
		vector<Mat> planes;
		split(img, planes);
		//imshow("V", planes[2]);
		//����������ͷ���
		vector<double> nv2 = get_nv(planes[2]);
		//�������ұ�
		Mat lup2(1, 256, CV_32F, Scalar::all(0));
		//�����ұ�
		//cout << lup.cols << "   " << lup.rows << endl;
		lup2 = lookup(nv2, k1, k2, planes[2]); //���ò��Һ���
		//Ӧ�ò��ұ�
		LUT(planes[2], lup2, planes[2]);
		//������ͼ��
		merge(planes, img);
		cvtColor(img,img, CV_HSV2BGR);
	}
	else if(img.channels() == 1)
	{
		vector<double> nv = get_nv(img);
		//�������ұ�
		Mat lup(1, 256, CV_32F, Scalar::all(0));
		//�����ұ�
		//cout << lup.cols << "   " << lup.rows << endl;
		lup = lookup(nv, k1, k2, img); //���ò��Һ���
		//Ӧ�ò��ұ�
		LUT(img, lup, img);
	}
	return img;
}

/*
��һ�ŻҶ�ͼ���ֱ��ͼ��ͼ�񻯵���ʽչ�ֳ��� �˺�����������ͼ
*/
Mat hist_image(Mat hist)
{
	double max_value = 0; 
	//�ҵ�ֱ��ͼ���е����ֵ����Сֵ
	minMaxLoc(hist, NULL, &max_value, 0, 0);
	Mat hist_image(hist.rows, hist.rows * 5, CV_8U, Scalar(255)); //���׻���
	//��������Ϊͼ��߶ȵ�90%
	int hpt = static_cast<int>(0.9 * hist.rows); //static_castǿ������ת��
	for (int h = 0; h < hist.rows * 5; h = h + 5)
	{
		float bin_val = hist.at<float>(h / 5);
		//int intensity = static_cast<int>(bin_val*hpt / max_value); 
		int intensity = (int)(bin_val*hpt / max_value);
		for (int i = 0; i < 5; i++)
		{
			line(hist_image, Point(h + i, hist.rows), Point(h + i, hist.rows - intensity), Scalar::all(0)); //ͼ���������Ͻ�Ϊ����ԭ��
		}

	}
	return hist_image;
}

/**
  ����ֲ���ֵһ�� ͼ����оֲ�����ȫ����ǿ
  ���룺ԭͼ��img; ������չ����k1; �Աȶ���չ����k2; �Ƿ���Ҫ�ֿ� isneeddivideblocks; ��ֱ�ֿ����nblockht;ˮƽ�ֿ���� nblockwd 
  �������ǿ���ͼƬ rt
*/
Mat regulated(Mat image,float k1, float k2,bool isneeddivideblocks = false,unsigned int nblockwd = 0, unsigned int nblockht = 0)
{
	//�������
	Mat img = image.clone();
	Size s = img.size();
	if (nblockht > s.height  ||  nblockwd > s.width)
	{
		cout << "��������ͼ��ߴ�̫С��" << endl;
		exit(-1);
	}
	if (k1 < 0  || k2 < 0)
	{
		cout << "��������k1,k2��չ���Ӳ���Ϊ��" << endl;
		exit(-1);
	}
	if (isneeddivideblocks)
	{
		//ȡ��ÿ�����е����� ������
		vector<Mat> blocks; //��
		unsigned scalewd = s.width / nblockwd; //ˮƽ�̶�
		unsigned scaleht = s.height / nblockht; //��ֱ�̶�
		unsigned begin_x = 0;
		unsigned begin_y = 0;
		Rect rio;
		Mat imgRIO;
		//cout << "ˮƽ�ֿ������" << nblockwd <<"  " << "��ֱ�ֿ������" << nblockht <<endl;
		for (size_t i = 0; i < nblockwd * nblockht; i++)
		{
			//������
			begin_x = ( i % nblockwd ) * scalewd;
			begin_y = ( i / nblockwd ) * scaleht;
			rio.x = begin_x;
			rio.y = begin_y;
			rio.width = scalewd;
			rio.height = scaleht;
			imgRIO = img(rio); //ȡ����Ȥ����
			//cout <<"��Ȥ����" <<rio <<endl;
			imgRIO = regulate(imgRIO,k1,k2);
		}
	}else
	{
		//ȫ����ǿ
	   img = regulate(img,k1,k2);
	}
	return img;
}
#endif //!REGULATION