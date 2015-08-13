#ifndef GETPLATE
#define GETPLATE
#include"getplatebase.h"

/**
通过原图像 img 得到车牌可能位置的形状描述符向量 （全局）规定化 + 局部中值二值化
输入：原图像 img ; 保存可能是车牌的向量possible_plate_rects; 图像的名字 string img_name; 亮度扩展因子k1;对比度扩展因子k2; 约减rect系数 rect_ratio; 是否保存处理中间图片bool issave; 图片的保存路径 string save_path 
输出：矩形形状描述符向量
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
	string datainfo; //保存处理过程中的文本信息 最后一次输出
	datainfo.append("线程1  方法：（全局）规定化 + 局部中值二值化\n");

	Mat img = image.clone();
	Mat img_copy = img.clone();
	//imshow(img_name+"原图", change_size(img_copy, 0.4));

	/**
		灰度化
	*/
	//取HSV空间中的V分量 V = max（R，G，B）
	Mat img_gray;
	vector<Mat> planes;
	cvtColor(img, img, CV_BGR2HSV);
	split(img, planes);
	img_gray = planes[2];
	img_gray_copy = img_gray.clone();
	//	imshow(img_name + "灰度图", change_size(img_gray, 0.4));

	/**
	图像增强
	*/	
	img_gray = regulated(img_gray,k1,k2);
	img_gray_regulated_copy = img_gray.clone();
	//imshow(img_name + "增强图", change_size(img_gray_regulated_copy, 0.4));

	/**
		高斯滤波
	*/
	GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);

	/**
	边缘检测
	*/
    Mat contours;
	Sobel(img_gray, contours, CV_8U, 1, 0); //垂直边缘
	img_contours_copy = contours.clone();

	/**
	根据阈值 进行二值化
	*/
		//分块
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
	//imshow(img_name + "局部二值化后的图", change_size(contours, 0.4));
	

	/**
	   闭运算和寻找连通区域 受形态学运算的核的影响 所以应该设置多个核
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
		//获得轮廓
		contour_lines_temp = get_contours(contours,element_sizes[i],1,img_name,RegulationAddLocalMiddlethresh,issave,save_path);

		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("处理前轮廓个数") + num_str + " \n");
		
		//移除过长或过短的轮廓长度
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
		datainfo.append(string("处理后轮廓个数") + num_str + " \n");

	    //收集轮廓
		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
		//计算连通区域的矩形形状描述符
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
	//imshow(img_name + "连通通区域轮廓图去掉小轮廓", change_size(img_contours_less, 0.4));
   
	/**
	 Rect约减
	*/
	possible_plates = rect_decrease(possible_plates,rect_ratio,true,datainfo);
	/**
	获得轮廓参数带回结果
	*/
	(*possible_plate_rects).insert((*possible_plate_rects).begin(), possible_plates.begin(),possible_plates.end());

	cout << datainfo <<endl;

	/**
		中间图片的保存
	*/
	char type_char[6] = "__1__";
	if (issave == true)
	{
		//imwrite(save_path + img_name+"原图.jpg",img_copy);

		imwrite(save_path + img_name+ type_char+"灰度图" +".jpg",img_gray_copy);
		
	    imwrite(save_path + img_name+ type_char+"全局增强后的图像" + ".jpg",img_gray_regulated_copy);
	
		imwrite(save_path + img_name+ type_char+"边缘图（Sober垂直边缘）" +".jpg",img_contours_copy);

        imwrite(save_path + img_name+ type_char+"局部二值化后的图" + ".jpg",img_locate_threshold_copy);
	
		imwrite(save_path + img_name+ type_char+"连通通区域轮廓图去掉小轮廓" + ".jpg",img_contours_less_copy);

		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); 

		}
		imwrite(save_path + img_name+ type_char+"可能是车牌的矩形框图约减后.jpg",rect_img + image);
	}
	SetEvent(event1);
	return 0;
}


/**
通过原图像 img 得到车牌可能位置的形状描述符向量  不用边缘检测 直接全局OSTU二值化（针对夜间车牌反光的情况）
输入：原图像 img; 保存可能是车牌的向量possible_plate_rects; 图像的名字 string img_name ;  约减rect系数 rect_ratio;是否保存处理中间图片bool issave 图片的保存路径 string save_path ;
输出：bool
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

	//imshow(img_name+"原图", change_size(img, 0.4));
	Mat img = image.clone();
	Mat img_copy = img.clone();
	string datainfo; //保存处理过程中的文本信息 最后一次输出
	datainfo.append("线程2   方法：不用边缘检测 直接全局OSTU二值化（针对夜间车牌反光的情况）\n");

	vector<Mat> planes;
	Mat img_gray;
	Mat img_gray_copy;
	Mat img_OSTU_threshold;
	Mat img_OSTU_threshold_copy;

	//取HSV空间中的V分量 V = max（R，G，B）
	cvtColor(img, img, CV_BGR2HSV);
	split(img, planes);
	img_gray = planes[2];
	img_gray_copy = img_gray.clone();
	/**
		高斯滤波
	*/
	GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
	/**
	根据阈值 进行二值化
	*/
	threshold(img_gray,img_OSTU_threshold, 0, 255, THRESH_BINARY | THRESH_OTSU);
	img_OSTU_threshold_copy = img_OSTU_threshold.clone();
	
	/**
	   闭运算和寻找连通区域 受形态学运算的核的影响 所以应该设置多个核
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
		//获得轮廓
		contour_lines_temp = get_contours(img_OSTU_threshold,element_sizes[i],-1,img_name,NocontoursADDWholeOSTU,issave,save_path);

		_itoa(contour_lines_temp.size(),num_str,10);
		datainfo.append(string("处理前轮廓个数") + num_str + " \n");

		//移除过长或过短的轮廓长度
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
		datainfo.append(string("处理后轮廓个数") + num_str + " \n");

	    //收集轮廓
		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
		//计算连通区域的矩形形状描述符
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
	//imshow(img_name + "连通通区域轮廓图去掉小轮廓", change_size(img_contours_less, 0.4));
   
	/**
	 Rect约减
	*/
	possible_plates = rect_decrease(possible_plates,rect_ratio,true,datainfo);
	(*possible_plate_rects).insert((*possible_plate_rects).begin(),possible_plates.begin(),possible_plates.end());

	cout << datainfo <<endl;
	/**
		中间图片的保存
	*/
	char type_char[6] = "__2__";
	if (issave == true)
	{
		//imwrite(save_path + img_name+"原图.jpg",img_copy);

		imwrite(save_path + img_name+ type_char+"灰度图" + ".jpg",img_gray_copy);
		
		imwrite(save_path + img_name+ type_char+"自动OSTU二值化后的图" +".jpg",img_OSTU_threshold_copy);
		
		imwrite(save_path + img_name+ type_char+"连通通区域轮廓图去掉小轮廓" +".jpg",img_contours_less_copy);

		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); //绿色

		}
		imwrite(save_path + img_name+ type_char+"可能是车牌的矩形框图约减后.jpg",rect_img + image);

	}
	SetEvent(event2);
	return 0;
}



/**
通过原图像 img 得到车牌可能位置的形状描述符向量 颜色检测
输入：原图像 img; 保存可能是车牌的向量possible_plate_rects; 图像的名字 string img_name; 约减rect系数 rect_ratio;是否保存处理中间图片bool issave 图片的保存路径 string save_path 
输出：矩形形状描述符向量
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

	//imshow(img_name+"原图", change_size(img, 0.4));
	Mat img = image.clone();
	Mat img_copy = img.clone();
	string datainfo; //保存处理过程中的文本信息 最后一次输出
	datainfo.append("线程3  方法：颜色定位\n");
	
	Mat color_threshed;
	Mat color_threshed_copy;

	color_threshed = color_detect(img);
	color_threshed_copy = color_threshed.clone();
	
	/**
	   闭运算和寻找连通区域 受形态学运算的核的影响 所以应该设置多个核
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
		//获得轮廓
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
		datainfo.append(string("处理前轮廓个数") + num_str + " \n");

		//移除过长或过短的轮廓长度
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
		datainfo.append(string("处理后轮廓个数") + num_str + " \n");

	    //收集轮廓
		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
		//计算连通区域的矩形形状描述符
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
	//imshow(img_name + "连通通区域轮廓图去掉小轮廓", change_size(img_contours_less, 0.4));
   
	/**
	 Rect约减
	*/
	possible_plates = rect_decrease(possible_plates,rect_ratio,true,datainfo);
	(*possible_plate_rects).insert((*possible_plate_rects).begin(),possible_plates.begin(),possible_plates.end());
	cout <<datainfo <<endl;
	/**
		中间图片的保存
	*/
	char type_char[6] = "__3__";
	if (issave == true)
	{
		//imwrite(save_path + img_name+"原图.jpg",img_copy);

		imwrite(save_path + img_name+type_char+"颜色检测结果" + ".jpg",color_threshed_copy);

		imwrite(save_path + img_name+type_char+"连通通区域轮廓图去掉小轮廓" + ".jpg",img_contours_less_copy);

		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); 

		}
		imwrite(save_path + img_name+ type_char+"可能是车牌的矩形框图约减后.jpg",rect_img + image);
	}

	SetEvent(event3); //开启事件
	return 0;
}

/**
通过原图像 img 得到车牌可能位置的形状描述符向量  图像分割成明暗两幅图再分别进行边缘检测和OSTU
输入：原图像 img; 保存可能是车牌的向量possible_plate_rects; 图像的名字 string img_name ;  约减rect系数 rect_ratio;是否保存处理中间图片bool issave 图片的保存路径 string save_path ;
输出：bool
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

	//imshow(img_name+"原图", change_size(img, 0.4));
	Mat img = image.clone();
	Mat img_copy = img.clone();
	string datainfo; //保存处理过程中的文本信息 最后一次输出
	datainfo.append("线程4   方法：图像分割\n");

	vector<Mat> planes;
	Mat img_gray;
	Mat img_gray_light;
	Mat img_gray_black;
	Mat img_gray_copy;
	Mat img_OSTU_threshold;
	Mat img_OSTU_threshold_copy;

	//取HSV空间中的V分量 V = max（R，G，B）
	cvtColor(img, img, CV_BGR2HSV);
	split(img, planes);
	img_gray = planes[2];
	img_gray_copy = img_gray.clone();
	imwrite(save_path + img_name+ "灰度图" + ".jpg",img_gray_copy);
	/**
	把一张图片分割成明暗两个部分
	*/
	threshold(img_gray,img_OSTU_threshold, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	imwrite(save_path + img_name+ "原图灰度图OSTU二值化" + ".jpg",img_OSTU_threshold);
	Mat element;
	char i_str[8];
	for (int i = 1; i <= 4; i++)
	{
		element = getStructuringElement(MORPH_RECT,Size(15 * i,15 *i));
		morphologyEx(img_OSTU_threshold,img_OSTU_threshold,MORPH_OPEN,element);
		_itoa(i,i_str,10);
		//imwrite(save_path + img_name + "多次开运算图" +i_str+".jpg",img_OSTU_threshold);
	}
	
	img_gray.copyTo(img_gray_black,img_OSTU_threshold);
	img_gray_light = img_gray - img_gray_black;
	

	imwrite(save_path + img_name + "高亮区图" +i_str+".jpg",img_gray_light);
	imwrite(save_path + img_name + "暗区图" +i_str+".jpg",img_gray_black);
	
	get_rects_form_contours(img_gray_light,possible_plate_rects,img_name,rect_ratio,1,true,datainfo,issave,save_path);
	get_rects_form_contours(img_gray_black,possible_plate_rects,img_name,rect_ratio,-1,true,datainfo,issave,save_path);

	cout << datainfo <<endl;

	/**
		中间图片的保存
	*/
	char type_char[6] = "__4__";
	if (issave == true)
	{
		vector<Rect> possible_plates = *possible_plate_rects;
		Mat rect_img(img.size(),CV_8UC3,Scalar::all(0));
		for (int i = 0; i < possible_plates.size(); i++)
		{
			rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); //绿色

		}
		imwrite(save_path + img_name+ type_char+"可能是车牌的矩形框图约减后.jpg",rect_img + image);
	}
	SetEvent(event4);
	return 0;
}








//
///**
//通过原图像 img 得到车牌可能位置的形状描述符向量
//输入：原图像 img 和 图像的名字 string img_name 是否保存处理中间图片bool issave 图片的保存路径 string save_path ,请求Rect的方式，亮度扩展因子k1,对比度扩展因子k2 约减rect系数 rect_ratio
//输出：矩形形状描述符向量
//*/
//vector<Rect> getplateRect(Mat image, string img_name, bool issave, string save_path,Query_rect_type type,float k1 ,float k2,float rect_ratio)
//{
//	
//	//imshow(img_name+"原图", change_size(img, 0.4));
//	Mat img = image.clone();
//	Mat img_copy = img.clone();
//
//	/**
//		灰度化
//	*/
//	Mat img_gray;
//	//cvtColor(img, img_gray, CV_BGR2GRAY);
//	//imshow(img_name + "灰度图", change_size(img_gray, 0.4));
//
//	////只取R通道来做
//	//vector<Mat> planes;
//	//split(img,planes);
//	//img_gray = planes[2];
//	////imshow(img_name + "灰度图", change_size(img_gray, 0.4));
//
//	vector<Mat> planes;
//	Mat img_gray_copy;
//
////	imshow(img_name + "灰度图", change_size(img_gray, 0.4));
//	//分块
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
//	  根据不同的请求方式对图像进行不同的处理
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
//		//取HSV空间中的V分量 V = max（R，G，B）
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//	    图像增强
//	       */	
//		img_gray = regulated(img_gray,k1,k2);
//		img_gray_regulated_copy = img_gray.clone();
//		// imshow(img_name + "增强图", change_size(img_gray, 0.4));
//		/**
//		 高斯滤波
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		边缘检测
//	     */
//		Sobel(img_gray, contours, CV_8U, 1, 0); //垂直边缘
//		img_contours_copy = contours.clone();
//		/**
//		根据阈值 进行二值化
//	    */
//		contours = locate_threshold(contours, nblockwd, nblockht);
//		img_locate_threshold_copy = contours.clone();
//	    //imshow(img_name + "局部二值化后的图", change_size(contours, 0.4));
//		break;
//	case LocalMiddlethresh:
//		//取HSV空间中的V分量 V = max（R，G，B）
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//		 高斯滤波
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		边缘检测
//	     */
//		Sobel(img_gray, contours, CV_8U, 1, 0); //垂直边缘
//		img_contours_copy = contours.clone();
//		/**
//		根据阈值 进行二值化
//	    */
//		contours = locate_threshold(contours, nblockwd, nblockht);
//		img_locate_threshold_copy = contours.clone();
//	    //imshow(img_name + "局部二值化后的图", change_size(contours, 0.4));
//		break;
//	case MadAddWholeOSTU:
//		//取HSV空间中的V分量 V = max（R，G，B）
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//	    图像增强
//	    */	
//		enhanceImgByMAD(img_gray,img_gray,Size(11,5));
//		img_gray_maded_copy = img_gray.clone();
//		// imshow(img_name + "增强图", change_size(img_gray, 0.4));
//		/**
//		 高斯滤波
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		边缘检测
//	     */
//		Sobel(img_gray, contours, CV_8U, 1, 0); //垂直边缘
//		img_contours_copy = contours.clone();
//		/**
//		根据阈值 进行二值化
//	    */
//		threshold(contours, contours, 0, 255, THRESH_BINARY | THRESH_OTSU);
//		img_OSTU_threshold_copy = contours.clone();
//		break;
//	case NocontoursADDWholeOSTU:
//		//取HSV空间中的V分量 V = max（R，G，B）
//		cvtColor(img, img, CV_BGR2HSV);
//		split(img, planes);
//		img_gray = planes[2];
//		img_gray_copy = img_gray.clone();
//		/**
//		 高斯滤波
//		*/
//		GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);
//		/**
//		根据阈值 进行二值化
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
//	   闭运算和寻找连通区域 受形态学运算的核的影响 所以应该设置多个核
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
//		//获得轮廓
//		contour_lines_temp = get_contours(contours,element_sizes[i],1,img_name,type,issave,save_path);
//		cout << "处理前轮廓个数" << contour_lines_temp.size() << endl;
//		//移除过长或过短的轮廓长度
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
//		cout << "处理后轮廓个数" << contour_lines_temp.size() << endl;
//	    //收集轮廓
//		contour_lines.insert(contour_lines.end(),contour_lines_temp.begin(),contour_lines_temp.end());
//		//计算连通区域的矩形形状描述符
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
//	//imshow(img_name + "连通通区域轮廓图去掉小轮廓", change_size(img_contours_less, 0.4));
//   
//	/**
//	 Rect约减
//	*/
//	possible_plates = rect_decrease(possible_plates,rect_ratio);
//	/**
//		中间图片的保存
//	*/
//	char type_char[4];
//	if (issave == true)
//	{
//		_itoa(type,type_char,10);
//		//imwrite(save_path + img_name+"原图.jpg",img_copy);
//		if (type != ColorThresh)
//		{
//			imwrite(save_path + img_name+"灰度图" + type_char+".jpg",img_gray_copy);
//		}
//		
//		if (type == RegulationAddLocalMiddlethresh)
//		{
//		   imwrite(save_path + img_name+"全局增强后的图像" + type_char+".jpg",img_gray_regulated_copy);
//		}
//
//		if (type == MadAddWholeOSTU)
//		{ 
//		   imwrite(save_path + img_name+"全局增强后的图像" + type_char+".jpg",img_gray_maded_copy);
//		}
//
//		if (type != NocontoursADDWholeOSTU)
//		{
//			imwrite(save_path + img_name+"边缘图" + type_char+".jpg",img_contours_copy);
//		}
//
//		if (type == MadAddWholeOSTU || type == NocontoursADDWholeOSTU)
//		{
//			imwrite(save_path + img_name+"自动OSTU二值化后的图" + type_char+".jpg",img_OSTU_threshold_copy);
//		}else if (type == LocalMiddlethresh ||  type == RegulationAddLocalMiddlethresh)
//		{
//			imwrite(save_path + img_name+"局部二值化后的图" + type_char+".jpg",img_locate_threshold_copy);
//		}
//
//		imwrite(save_path + img_name+"连通通区域轮廓图去掉小轮廓" + type_char+".jpg",img_contours_less_copy);
//	}
//	return possible_plates;
//}
//




#endif //!GETPLATE