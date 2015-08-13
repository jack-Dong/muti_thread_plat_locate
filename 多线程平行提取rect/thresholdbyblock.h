#ifndef LOCATE_THRESHOD
#define LOCATE_THRESHOD
#include"base.h"
/**
   ���Ҷ�ͼ����оֲ���ֵ����ƽ��ֵ�����ݼ����0����
   ���룺�Ҷ�ͼ�� img, ��ֱ�ֿ����nblockht,ˮƽ�ֿ���� nblockwd
   �������ֵ�����ͼ��
*/
Mat locate_threshold(Mat image, unsigned int nblockwd, unsigned int nblockht)
{
	//�������
	Mat img = image.clone();
	Size s = img.size();
	if (nblockht > s.height  ||  nblockwd > s.width)
	{
		cout << "��������ͼ��ߴ�̫С��" << endl;
		exit(-1);
	}
	//ȡ��ÿ�����е����� ������
	vector<Mat> blocks; //��
	unsigned scalewd = s.width / nblockwd; //ˮƽ�̶�
	unsigned scaleht = s.height / nblockht; //��ֱ�̶�
	unsigned begin_x = 0;
	unsigned begin_y = 0;
	Rect rio;
	Mat imgRIO;
	float medim = 0, sum = 0;
	unsigned nozeros = 0;
	uchar pixe;
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
		//������Ȥ�������ֵ �� �������ظ���
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
		//��ֵ��
		medim = sum / nozeros;
		for (size_t i = 0; i < scaleht; i++)
		{
			for (size_t j = 0; j < scalewd; j++)
			{
				pixe = imgRIO.at<uchar>(i, j);
				if (pixe >= medim -1) //��ֹȫ������255
				{
					imgRIO.at<uchar>(i, j) = 255;
				}
				else
				{
					imgRIO.at<uchar>(i, j) = 0;
				}
			}
		}
		////�Զ���ֵ
		//threshold(imgRIO, imgRIO, 0, 255, THRESH_BINARY | THRESH_OTSU);
	}
	return img;
}

/**
	ͨ���ֲ���ֵ���ж�ֵ��
	���룺�Ҷ�ͼ��image_gray
	�������ֵ�����ͼ��
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
	//��ֵ��
	medim = sum / nozeros;
	for (size_t i = 0; i < img_gray.rows; i++)
	{
		for (size_t j = 0; j < img_gray.cols; j++)
		{
			pixe = img_gray.at<uchar>(i, j);
			if (pixe >= medim -1) //��ֹȫ������255
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