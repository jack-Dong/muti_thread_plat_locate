#ifndef GETPLATE
#define GETPLATE
#include"getplatebase.h"

/**
ͨ��ԭͼ�� img �õ����ƿ���λ�õ���״���������� ��ȫ�֣��涨�� + �ֲ���ֵ��ֵ��
���룺ԭͼ�� img ; ��������ǳ��Ƶ�����possible_plate_rects; ͼ������� string img_name; ������չ����k1;�Աȶ���չ����k2; Լ��rectϵ�� rect_ratio; �Ƿ񱣴洦���м�ͼƬbool issave; ͼƬ�ı���·�� string save_path 
�����������״����������
*/
DWORD WINAPI getplateRectbyRegulationAddLocalMiddlethresh(LPVOID paramter)
{
    ThreadParamter tp = *(ThreadParamter *)(paramter);
	Mat image = tp.image;
	vector<Rect> *possible_plate_rects = tp.possible_plate_rects;
	string img_name = tp.img_name;
	float rect_ratio = tp.rect_ratio;
	bool issave = tp.issave;
	string save_path = tp.save_path;
	float k1 = tp.k1;
	float k2 = tp.k2;
	//Mat image, vector<Rect> &possible_plate_rects, string img_name,float k1 ,float k2,float rect_ratio,bool issave = true, string save_path = NULL

	Mat img_gray_copy;
	Mat img_gray_regulated_copy;
	Mat img_contours_copy;
	Mat img_locate_threshold_copy;
	string datainfo; //���洦������е��ı���Ϣ ���һ�����
	datainfo.append("�߳�1  ��������ȫ�֣��涨�� + �ֲ���ֵ��ֵ��\n");

	Mat img = image.clone();
	Mat img_copy = img.clone();
	//imshow(img_name+"ԭͼ", change_size(img_copy, 0.4));

	/**
		�ҶȻ�
	*/
	//ȡHSV�ռ��е�V���� V = max��R��G��B��
	Mat img_gray;
	vector<Mat> planes;
	cvtColor(img, img, CV_BGR2HSV);
	split(img, planes);
	img_gray = planes[2];
	img_gray_copy = img_gray.clone();
	//	imshow(img_name + "�Ҷ�ͼ", change_size(img_gray, 0.4));

	/**
	ͼ����ǿ
	*/	
	img_gray = regulated(img_gray,k1,k2);
	img_gray_regulated_copy = img_gray.clone();
	//imshow(img_name + "��ǿͼ", change_size(img_gray_regulated_copy, 0.4));

	/**
		��˹�˲�
	*/
	GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);

	/**
	��Ե���
	*/
    Mat contours;
	Sobel(img_gray, contours, CV_8U, 1, 0); //��ֱ��Ե
	img_contours_copy = contours.clone();

	/**
	������ֵ ���ж�ֵ��
	*/
		//�ֿ�
	Size img_size = img.size();
	unsigned nblockwd,nblockht;
	if (img_size.width > 400)
	{
		nblockwd =  img_size.width  / 400;
	}else
	{
		nblockwd =  img_size.width  = 1;
	}
	if (img_size.height > 400)
	{
		nblockht =  img_size.height / 400;
	}else
	{
		nblockht =  img_size.height =1;
	}
	contours = locate_threshold(contours, nblockwd, nblockht);
	img_locate_threshold_copy = contours.clone();
	//imshow(img_name + "�ֲ���ֵ�����ͼ", change_size(contours, 0.4));
	

	/**
	   �������Ѱ����ͨ���� ����̬ѧ����ĺ˵�Ӱ�� ����Ӧ�����ö����
	*/
	vector<vector<Point>> contour_lines;
	vector<vector<Point>> contour_lines_temp;
	vector<Size> element_sizes;
	Size size0 = Size(7,5);  element_sizes.push_back(size0);
	Size size1 = Size(11,7);  element_sizes.push_back(size1);
	Size size2 = Size(15,9);  element_sizes.push_back(size2);
	Size size3 = Size(21,13);  element_sizes.push_back(size3);
	Size size4 = Size(25,17);  element_sizes.push_back(size4);

    int cmin = 70 * 2;
	int cmax = 400 * 4 - 1;
	Mat img_contours_less(img.size(), CV_8U, Scalar(255));
	vector<Rect> possible_plates;
	Rect r;
	char num_str[8];
	for (int i = 0; i < element_sizes.size(); i++)
	{
		//�������
		contour_lines_temp = get_contours(contours,element_sizes[i],1,img_name,RegulationAddLocalMiddlethresh,issave,save_path);

		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("����ǰ��������") + num_str + " \n");
		
		//�Ƴ���������̵���������
		vector<vector<Point>>::const_iterator itc = contour_lines_temp.begin();
		while (itc != contour_lines_temp.end())
		{
			if (itc->size() < cmin || itc->size() > cmax)
			{
				itc = contour_lines_temp.erase(itc);
			}
			else
			{
				++itc;
			}
		}
		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("�������������") + num_str + " \n");

	    //�ռ�����
		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
		//������ͨ����ľ�����״������
		vector<vector<Point>>::const_iterator itc1 = contour_lines_temp.begin();
		while (itc1 != contour_lines_temp.end())
		{
			r = boundingRect(Mat(*itc1));
			possible_plates.push_back(r);
			++itc1;
		}
	}
    drawContours(img_contours_less, contour_lines, -1, Scalar(0), 2);
	Mat img_contours_less_copy = img_contours_less.clone();
	//imshow(img_name + "��ͨͨ��������ͼȥ��С����", change_size(img_contours_less, 0.4));
   
	/**
	 RectԼ��
	*/
	possible_plates = rect_decrease(possible_plates,rect_ratio,true,datainfo);
	/**
	��������������ؽ��
	*/
	(*possible_plate_rects).insert((*possible_plate_rects).begin(), possible_plates.begin(),possible_plates.end());

	cout << datainfo <<endl;

	/**
		�м�ͼƬ�ı���
	*/
	char type_char[6] = "__1__";
	if (issave == true)
	{
		//imwrite(save_path + img_name+"ԭͼ.jpg",img_copy);

		imwrite(save_path + img_name+ type_char+"�Ҷ�ͼ" +".jpg",img_gray_copy);
		
	    imwrite(save_path + img_name+ type_char+"ȫ����ǿ���ͼ��" + ".jpg",img_gray_regulated_copy);
	
		imwrite(save_path + img_name+ type_char+"��Եͼ��Sober��ֱ��Ե��" +".jpg",img_contours_copy);

        imwrite(save_path + img_name+ type_char+"�ֲ���ֵ�����ͼ" + ".jpg",img_locate_threshold_copy);
	
		imwrite(save_path + img_name+ type_char+"��ͨͨ��������ͼȥ��С����" + ".jpg",img_contours_less_copy);

		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); 

		}
		imwrite(save_path + img_name+ type_char+"�����ǳ��Ƶľ��ο�ͼԼ����.jpg",rect_img + image);
	}
	SetEvent(event1);
	return 0;
}


/**
ͨ��ԭͼ�� img �õ����ƿ���λ�õ���״����������  ���ñ�Ե��� ֱ��ȫ��OSTU��ֵ�������ҹ�䳵�Ʒ���������
���룺ԭͼ�� img; ��������ǳ��Ƶ�����possible_plate_rects; ͼ������� string img_name ;  Լ��rectϵ�� rect_ratio;�Ƿ񱣴洦���м�ͼƬbool issave ͼƬ�ı���·�� string save_path ;
�����bool
*/
DWORD WINAPI getplateRectbyNocontoursADDWholeOSTU(LPVOID paramter)
{
	ThreadParamter tp = *(ThreadParamter *)(paramter);
	Mat image = tp.image;
	vector<Rect> *possible_plate_rects = tp.possible_plate_rects;
	string img_name = tp.img_name;
	float rect_ratio = tp.rect_ratio;
	bool issave = tp.issave;
	string save_path = tp.save_path;

	//imshow(img_name+"ԭͼ", change_size(img, 0.4));
	Mat img = image.clone();
	Mat img_copy = img.clone();
	string datainfo; //���洦������е��ı���Ϣ ���һ�����
	datainfo.append("�߳�2   ���������ñ�Ե��� ֱ��ȫ��OSTU��ֵ�������ҹ�䳵�Ʒ���������\n");

	vector<Mat> planes;
	Mat img_gray;
	Mat img_gray_copy;
	Mat img_OSTU_threshold;
	Mat img_OSTU_threshold_copy;

	//ȡHSV�ռ��е�V���� V = max��R��G��B��
	cvtColor(img, img, CV_BGR2HSV);
	split(img, planes);
	img_gray = planes[2];
	img_gray_copy = img_gray.clone();
	/**
		��˹�˲�
	*/
	GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
	/**
	������ֵ ���ж�ֵ��
	*/
	threshold(img_gray,img_OSTU_threshold, 0, 255, THRESH_BINARY | THRESH_OTSU);
	img_OSTU_threshold_copy = img_OSTU_threshold.clone();
	
	/**
	   �������Ѱ����ͨ���� ����̬ѧ����ĺ˵�Ӱ�� ����Ӧ�����ö����
	*/
	vector<vector<Point>> contour_lines;
	vector<vector<Point>> contour_lines_temp;
	vector<Size> element_sizes;
	Size size0 = Size(7,5);  element_sizes.push_back(size0);
	Size size1 = Size(11,7);  element_sizes.push_back(size1);
	Size size2 = Size(15,9);  element_sizes.push_back(size2);
	Size size3 = Size(21,13);  element_sizes.push_back(size3);
	Size size4 = Size(25,17);  element_sizes.push_back(size4);

    int cmin = 40;
	int cmax = 400 * 4 - 1;
	Mat img_contours_less(img.size(), CV_8U, Scalar(255));
	vector<Rect> possible_plates;
	Rect r;
	char num_str[8];
	for (int i = 0; i < element_sizes.size(); i++)
	{
		//�������
		contour_lines_temp = get_contours(img_OSTU_threshold,element_sizes[i],-1,img_name,NocontoursADDWholeOSTU,issave,save_path);

		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("����ǰ��������") + num_str + " \n");

		//�Ƴ���������̵���������
		vector<vector<Point>>::const_iterator itc = contour_lines_temp.begin();
		while (itc != contour_lines_temp.end())
		{
			if (itc->size() < cmin || itc->size() > cmax)
			{
				itc = contour_lines_temp.erase(itc);
			}
			else
			{
				++itc;
			}
		}

		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("�������������") + num_str + " \n");

	    //�ռ�����
		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
		//������ͨ����ľ�����״������
		vector<vector<Point>>::const_iterator itc1 = contour_lines_temp.begin();
		while (itc1 != contour_lines_temp.end())
		{
			r = boundingRect(Mat(*itc1));
			possible_plates.push_back(r);
			++itc1;
		}

	}


    drawContours(img_contours_less, contour_lines, -1, Scalar(0), 2);
	Mat img_contours_less_copy = img_contours_less.clone();
	//imshow(img_name + "��ͨͨ��������ͼȥ��С����", change_size(img_contours_less, 0.4));
   
	/**
	 RectԼ��
	*/
	possible_plates = rect_decrease(possible_plates,rect_ratio,true,datainfo);
	(*possible_plate_rects).insert((*possible_plate_rects).begin(),possible_plates.begin(),possible_plates.end());

	cout << datainfo <<endl;
	/**
		�м�ͼƬ�ı���
	*/
	char type_char[6] = "__2__";
	if (issave == true)
	{
		//imwrite(save_path + img_name+"ԭͼ.jpg",img_copy);

		imwrite(save_path + img_name+ type_char+"�Ҷ�ͼ" + ".jpg",img_gray_copy);
		
		imwrite(save_path + img_name+ type_char+"�Զ�OSTU��ֵ�����ͼ" +".jpg",img_OSTU_threshold_copy);
		
		imwrite(save_path + img_name+ type_char+"��ͨͨ��������ͼȥ��С����" +".jpg",img_contours_less_copy);

		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); //��ɫ

		}
		imwrite(save_path + img_name+ type_char+"�����ǳ��Ƶľ��ο�ͼԼ����.jpg",rect_img + image);

	}
	SetEvent(event2);
	return 0;
}



/**
ͨ��ԭͼ�� img �õ����ƿ���λ�õ���״���������� ��ɫ���
���룺ԭͼ�� img; ��������ǳ��Ƶ�����possible_plate_rects; ͼ������� string img_name; Լ��rectϵ�� rect_ratio;�Ƿ񱣴洦���м�ͼƬbool issave ͼƬ�ı���·�� string save_path 
�����������״����������
*/
DWORD WINAPI getplateRectbyColorThresh(LPVOID paramter)
{
	ThreadParamter tp = *(ThreadParamter *)(paramter);
	Mat image = tp.image;
	vector<Rect> *possible_plate_rects = tp.possible_plate_rects;
	string img_name = tp.img_name;
	float rect_ratio = tp.rect_ratio;
	bool issave = tp.issave;
	string save_path = tp.save_path;

	//imshow(img_name+"ԭͼ", change_size(img, 0.4));
	Mat img = image.clone();
	Mat img_copy = img.clone();
	string datainfo; //���洦������е��ı���Ϣ ���һ�����
	datainfo.append("�߳�3  ��������ɫ��λ\n");
	
	Mat color_threshed;
	Mat color_threshed_copy;

	color_threshed = color_detect(img);
	color_threshed_copy = color_threshed.clone();
	
	/**
	   �������Ѱ����ͨ���� ����̬ѧ����ĺ˵�Ӱ�� ����Ӧ�����ö����
	*/
	vector<vector<Point>> contour_lines;
	vector<vector<Point>> contour_lines_temp;
	vector<Size> element_sizes;
	Size size0 = Size(7,5);  element_sizes.push_back(size0);
	Size size1 = Size(11,7);  element_sizes.push_back(size1);
	Size size2 = Size(15,9);  element_sizes.push_back(size2);
	Size size3 = Size(21,13);  element_sizes.push_back(size3);

    int cmin = 40;
	int cmax = 400 * 4 - 1;
	Mat img_contours_less(img.size(), CV_8U, Scalar(255));

	vector<Rect> possible_plates;
	Rect r;
	char num_str[8];
	for (int i = 0; i < element_sizes.size() * 2; i++)
	{
		//�������
		if (i == element_sizes.size())
		{
			contour_lines_temp = get_contours(color_threshed,Size(0,0),0,img_name,ColorThresh,issave,save_path);
		}else if( i % 2 == 0)
		{
		    contour_lines_temp = get_contours(color_threshed,element_sizes[i / 2],1,img_name,ColorThresh,issave,save_path);
		}else if( i % 2 == 1)
		{
			contour_lines_temp = get_contours(color_threshed,element_sizes[i / 2],-1,img_name,ColorThresh,issave,save_path);
		}

		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("����ǰ��������") + num_str + " \n");

		//�Ƴ���������̵���������
		vector<vector<Point>>::const_iterator itc = contour_lines_temp.begin();
		while (itc != contour_lines_temp.end())
		{
			if (itc->size() < cmin || itc->size() > cmax)
			{
				itc = contour_lines_temp.erase(itc);
			}
			else
			{
				++itc;
			}
		}

		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("�������������") + num_str + " \n");

	    //�ռ�����
		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
		//������ͨ����ľ�����״������
		vector<vector<Point>>::const_iterator itc1 = contour_lines_temp.begin();
		while (itc1 != contour_lines_temp.end())
		{
			r = boundingRect(Mat(*itc1));
			possible_plates.push_back(r);
			++itc1;
		}
	}

    drawContours(img_contours_less, contour_lines, -1, Scalar(0), 2);
	Mat img_contours_less_copy = img_contours_less.clone();
	//imshow(img_name + "��ͨͨ��������ͼȥ��С����", change_size(img_contours_less, 0.4));
   
	/**
	 RectԼ��
	*/
	possible_plates = rect_decrease(possible_plates,rect_ratio,true,datainfo);
	(*possible_plate_rects).insert((*possible_plate_rects).begin(),possible_plates.begin(),possible_plates.end());
	cout <<datainfo <<endl;
	/**
		�м�ͼƬ�ı���
	*/
	char type_char[6] = "__3__";
	if (issave == true)
	{
		//imwrite(save_path + img_name+"ԭͼ.jpg",img_copy);

		imwrite(save_path + img_name+type_char+"��ɫ�����" + ".jpg",color_threshed_copy);

		imwrite(save_path + img_name+type_char+"��ͨͨ��������ͼȥ��С����" + ".jpg",img_contours_less_copy);

		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); 

		}
		imwrite(save_path + img_name+ type_char+"�����ǳ��Ƶľ��ο�ͼԼ����.jpg",rect_img + image);
	}

	SetEvent(event3); //�����¼�
	return 0;
}

/**
ͨ��ԭͼ�� img �õ����ƿ���λ�õ���״����������  ͼ��ָ����������ͼ�ٷֱ���б�Ե����OSTU
���룺ԭͼ�� img; ��������ǳ��Ƶ�����possible_plate_rects; ͼ������� string img_name ;  Լ��rectϵ�� rect_ratio;�Ƿ񱣴洦���м�ͼƬbool issave ͼƬ�ı���·�� string save_path ;
�����bool
*/
DWORD WINAPI getplateRectbyContoursADDLocalOSTU(LPVOID paramter)
{
	ThreadParamter tp = *(ThreadParamter *)(paramter);
	Mat image = tp.image;
	vector<Rect> *possible_plate_rects = tp.possible_plate_rects;
	string img_name = tp.img_name;
	float rect_ratio = tp.rect_ratio;
	bool issave = tp.issave;
	string save_path = tp.save_path;

	//imshow(img_name+"ԭͼ", change_size(img, 0.4));
	Mat img = image.clone();
	Mat img_copy = img.clone();
	string datainfo; //���洦������е��ı���Ϣ ���һ�����
	datainfo.append("�߳�4   ������ͼ��ָ�\n");

	vector<Mat> planes;
	Mat img_gray;
	Mat img_gray_light;
	Mat img_gray_black;
	Mat img_gray_copy;
	Mat img_OSTU_threshold;
	Mat img_OSTU_threshold_copy;

	//ȡHSV�ռ��е�V���� V = max��R��G��B��
	cvtColor(img, img, CV_BGR2HSV);
	split(img, planes);
	img_gray = planes[2];
	img_gray_copy = img_gray.clone();
	imwrite(save_path + img_name+ "�Ҷ�ͼ" + ".jpg",img_gray_copy);
	/**
	��һ��ͼƬ�ָ��������������
	*/
	threshold(img_gray,img_OSTU_threshold, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	imwrite(save_path + img_name+ "ԭͼ�Ҷ�ͼOSTU��ֵ��" + ".jpg",img_OSTU_threshold);
	Mat element;
	char i_str[8];
	for (int i = 1; i <= 4; i++)
	{
		element = getStructuringElement(MORPH_RECT,Size(15 * i,15 *i));
		morphologyEx(img_OSTU_threshold,img_OSTU_threshold,MORPH_OPEN,element);
		_itoa(i,i_str,10);
		//imwrite(save_path + img_name + "��ο�����ͼ" +i_str+".jpg",img_OSTU_threshold);
	}
	
	img_gray.copyTo(img_gray_black,img_OSTU_threshold);
	img_gray_light = img_gray - img_gray_black;
	

	imwrite(save_path + img_name + "������ͼ" +i_str+".jpg",img_gray_light);
	imwrite(save_path + img_name + "����ͼ" +i_str+".jpg",img_gray_black);
	
	get_rects_form_contours(img_gray_light,possible_plate_rects,img_name,rect_ratio,1,true,datainfo,issave,save_path);
	get_rects_form_contours(img_gray_black,possible_plate_rects,img_name,rect_ratio,-1,true,datainfo,issave,save_path);

	cout << datainfo <<endl;

	/**
		�м�ͼƬ�ı���
	*/
	char type_char[6] = "__4__";
	if (issave == true)
	{
		vector<Rect> possible_plates = *possible_plate_rects;
		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); //��ɫ

		}
		imwrite(save_path + img_name+ type_char+"�����ǳ��Ƶľ��ο�ͼԼ����.jpg",rect_img + image);
	}
	SetEvent(event4);
	return 0;
}








//
///**
//ͨ��ԭͼ�� img �õ����ƿ���λ�õ���״����������
//���룺ԭͼ�� img �� ͼ������� string img_name �Ƿ񱣴洦���м�ͼƬbool issave ͼƬ�ı���·�� string save_path ,����Rect�ķ�ʽ��������չ����k1,�Աȶ���չ����k2 Լ��rectϵ�� rect_ratio
//�����������״����������
//*/
//vector<Rect> getplateRect(Mat image, string img_name, bool issave, string save_path,Query_rect_type type,float k1 ,float k2,float rect_ratio)
//{
//	
//	//imshow(img_name+"ԭͼ", change_size(img, 0.4));
//	Mat img = image.clone();
//	Mat img_copy = img.clone();
//
//	/**
//		�ҶȻ�
//	*/
//	Mat img_gray;
//	//cvtColor(img, img_gray, CV_BGR2GRAY);
//	//imshow(img_name + "�Ҷ�ͼ", change_size(img_gray, 0.4));
//
//	////ֻȡRͨ������
//	//vector<Mat> planes;
//	//split(img,planes);
//	//img_gray = planes[2];
//	////imshow(img_name + "�Ҷ�ͼ", change_size(img_gray, 0.4));
//
//	vector<Mat> planes;
//	Mat img_gray_copy;
//
////	imshow(img_name + "�Ҷ�ͼ", change_size(img_gray, 0.4));
//	//�ֿ�
//	Size img_size = img.size();
//	unsigned nblockwd,nblockht;
//	if (img_size.width > 400)
//	{
//		nblockwd =  img_size.width  / 400;
//	}else
//	{
//		nblockwd =  img_size.width  = 1;
//	}
//	if (img_size.height > 400)
//	{
//		nblockht =  img_size.height / 400;
//	}else
//	{
//		nblockht =  img_size.height =1;
//	}
//
//	/**
//	  ���ݲ�ͬ������ʽ��ͼ����в�ͬ�Ĵ���
//	*/
//	Mat img_gray_regulated_copy;
//	Mat contours;
//	Mat img_contours_copy;
//	Mat img_locate_threshold_copy;
//	Mat img_gray_maded_copy;
//	Mat img_OSTU_threshold_copy;
//	switch (type)
//	{
//	case RegulationAddLocalMiddlethresh:
//		//ȡHSV�ռ��е�V���� V = max��R��G��B��
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//	    ͼ����ǿ
//	       */	
//		img_gray = regulated(img_gray,k1,k2);
//		img_gray_regulated_copy = img_gray.clone();
//		// imshow(img_name + "��ǿͼ", change_size(img_gray, 0.4));
//		/**
//		 ��˹�˲�
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		��Ե���
//	     */
//		Sobel(img_gray, contours, CV_8U, 1, 0); //��ֱ��Ե
//		img_contours_copy = contours.clone();
//		/**
//		������ֵ ���ж�ֵ��
//	    */
//		contours = locate_threshold(contours, nblockwd, nblockht);
//		img_locate_threshold_copy = contours.clone();
//	    //imshow(img_name + "�ֲ���ֵ�����ͼ", change_size(contours, 0.4));
//		break;
//	case LocalMiddlethresh:
//		//ȡHSV�ռ��е�V���� V = max��R��G��B��
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//		 ��˹�˲�
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		��Ե���
//	     */
//		Sobel(img_gray, contours, CV_8U, 1, 0); //��ֱ��Ե
//		img_contours_copy = contours.clone();
//		/**
//		������ֵ ���ж�ֵ��
//	    */
//		contours = locate_threshold(contours, nblockwd, nblockht);
//		img_locate_threshold_copy = contours.clone();
//	    //imshow(img_name + "�ֲ���ֵ�����ͼ", change_size(contours, 0.4));
//		break;
//	case MadAddWholeOSTU:
//		//ȡHSV�ռ��е�V���� V = max��R��G��B��
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//	    ͼ����ǿ
//	    */	
//		enhanceImgByMAD(img_gray,img_gray,Size(11,5));
//		img_gray_maded_copy = img_gray.clone();
//		// imshow(img_name + "��ǿͼ", change_size(img_gray, 0.4));
//		/**
//		 ��˹�˲�
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		��Ե���
//	     */
//		Sobel(img_gray, contours, CV_8U, 1, 0); //��ֱ��Ե
//		img_contours_copy = contours.clone();
//		/**
//		������ֵ ���ж�ֵ��
//	    */
//		threshold(contours, contours, 0, 255, THRESH_BINARY | THRESH_OTSU);
//		img_OSTU_threshold_copy = contours.clone();
//		break;
//	case NocontoursADDWholeOSTU:
//		//ȡHSV�ռ��е�V���� V = max��R��G��B��
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//		 ��˹�˲�
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		������ֵ ���ж�ֵ��
//	    */
//		threshold(img_gray,contours, 0, 255, THRESH_BINARY | THRESH_OTSU);
//		img_OSTU_threshold_copy = contours.clone();
//		break;
//	case ColorThresh:
//		contours = color_detect(img);
//		img_contours_copy = contours.clone();
//	default:
//		break;
//	}
//	
//	/**
//	   �������Ѱ����ͨ���� ����̬ѧ����ĺ˵�Ӱ�� ����Ӧ�����ö����
//	*/
//	vector<vector<Point>> contour_lines;
//	vector<vector<Point>> contour_lines_temp;
//	vector<Size> element_sizes;
//	Size size0 = Size(7,5);  element_sizes.push_back(size0);
//	Size size1 = Size(11,7);  element_sizes.push_back(size1);
//	Size size2 = Size(15,9);  element_sizes.push_back(size2);
//	Size size3 = Size(21,13);  element_sizes.push_back(size3);
//	Size size4 = Size(25,17);  element_sizes.push_back(size4);
//
//    int cmin = 70 * 2 + 30 * 2;
//	if (type == NocontoursADDWholeOSTU || ColorThresh)
//	{
//		cmin = 40;
//	}
//	int cmax = 400 * 4 - 1;
//	Mat img_contours_less(img.size(), CV_8U, Scalar(255));
//	vector<Rect> possible_plates;
//	Rect r;
//	for (int i = 0; i < element_sizes.size(); i++)
//	{
//		//�������
//		contour_lines_temp = get_contours(contours,element_sizes[i],1,img_name,type,issave,save_path);
//		cout << "����ǰ��������" << contour_lines_temp.size() << endl;
//		//�Ƴ���������̵���������
//		vector<vector<Point>>::const_iterator itc = contour_lines_temp.begin();
//		while (itc != contour_lines_temp.end())
//		{
//			if (itc->size() < cmin || itc->size() > cmax)
//			{
//				itc = contour_lines_temp.erase(itc);
//			}
//			else
//			{
//				++itc;
//			}
//		}
//		cout << "�������������" << contour_lines_temp.size() << endl;
//	    //�ռ�����
//		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
//		//������ͨ����ľ�����״������
//		vector<vector<Point>>::const_iterator itc1 = contour_lines_temp.begin();
//		while (itc1 != contour_lines_temp.end())
//		{
//			r = boundingRect(Mat(*itc1));
//			possible_plates.push_back(r);
//			++itc1;
//		}
//	}
//    drawContours(img_contours_less, contour_lines, -1, Scalar(0), 2);
//	Mat img_contours_less_copy = img_contours_less.clone();
//	//imshow(img_name + "��ͨͨ��������ͼȥ��С����", change_size(img_contours_less, 0.4));
//   
//	/**
//	 RectԼ��
//	*/
//	possible_plates = rect_decrease(possible_plates,rect_ratio);
//	/**
//		�м�ͼƬ�ı���
//	*/
//	char type_char[4];
//	if (issave == true)
//	{
//		_itoa(type,type_char,10);
//		//imwrite(save_path + img_name+"ԭͼ.jpg",img_copy);
//		if (type != ColorThresh)
//		{
//			imwrite(save_path + img_name+"�Ҷ�ͼ" + type_char+".jpg",img_gray_copy);
//		}
//		
//		if (type == RegulationAddLocalMiddlethresh)
//		{
//		   imwrite(save_path + img_name+"ȫ����ǿ���ͼ��" + type_char+".jpg",img_gray_regulated_copy);
//		}
//
//		if (type == MadAddWholeOSTU)
//		{ 
//		   imwrite(save_path + img_name+"ȫ����ǿ���ͼ��" + type_char+".jpg",img_gray_maded_copy);
//		}
//
//		if (type != NocontoursADDWholeOSTU)
//		{
//			imwrite(save_path + img_name+"��Եͼ" + type_char+".jpg",img_contours_copy);
//		}
//
//		if (type == MadAddWholeOSTU || type == NocontoursADDWholeOSTU)
//		{
//			imwrite(save_path + img_name+"�Զ�OSTU��ֵ�����ͼ" + type_char+".jpg",img_OSTU_threshold_copy);
//		}else if (type == LocalMiddlethresh ||  type == RegulationAddLocalMiddlethresh)
//		{
//			imwrite(save_path + img_name+"�ֲ���ֵ�����ͼ" + type_char+".jpg",img_locate_threshold_copy);
//		}
//
//		imwrite(save_path + img_name+"��ͨͨ��������ͼȥ��С����" + type_char+".jpg",img_contours_less_copy);
//	}
//	return possible_plates;
//}
//




#endif //!GETPLATE