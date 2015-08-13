#ifndef GETPLATEBASE
#define GETPLATEBASE
#include"thresholdbyblock.h"
#include"regulation.h"
#include"mad.h"
#include"color.h"
/**
定义请求图片的rect的不同方式
*/
enum Query_rect_type{ RegulationAddLocalMiddlethresh = 1,NocontoursADDWholeOSTU,ColorThresh,ContoursADDLocalOSTU,LocalMiddlethresh,MadAddWholeOSTU};

/**
 线程所需参数结构体
*/
struct ThreadParamter
{
	Mat image;
	vector<Rect> *possible_plate_rects;
	string img_name;
	float rect_ratio;
	bool issave;
	string save_path;
	float k1; //亮度扩展因子
	float k2; //对比度扩展因子
};
//定义3个事件 主线程等待这三个线程执行完成才继续执行
HANDLE event1; HANDLE event2; HANDLE event3; HANDLE event4; 

/**
图片太大不好输出就变小一点
*/
Mat change_size(Mat img, double arpha)
{
	Size s = img.size();
	s.height = s.height  * arpha;
	s.width = s.width * arpha;
	resize(img, img, s);
	return img;
}

/**
	形态学运算后得到连通区域
	输入： 边缘检测后的灰度图 contour; 闭运算的核的大小Size; 图片的名字 string name; int sign(1,闭，-1腐蚀) ;是否保存处理中间图片bool issave 图片的保存路径 string save_path ; flag (-1暗区 1高亮区)
	输出： 连通区域线 vector<vector<Point>> contour_lines
*/
vector<vector<Point>> get_contours(Mat contour, Size size,int sign,string img_name,Query_rect_type type,bool issave =false, string save_path = NULL, int flag = -1)
{
    /**
		形态学运算
	*/
	Mat cont = contour.clone();
	Mat closed;
	if (sign == 1)
	{
		Mat element = getStructuringElement(MORPH_RECT,size);
		morphologyEx(cont, closed, MORPH_CLOSE, element);
	}else if(sign == -1)
	{
		Mat element = getStructuringElement(MORPH_RECT,size);
		morphologyEx(cont, closed, MORPH_OPEN, element);
	}else if (sign == 0)
	{
		closed = cont;
	}
	
	Mat img_closed_copy = closed.clone();
	//imshow(img_name+"闭运算图", change_size(closed, 0.4));

	/**
		提取连通通区域轮廓
	*/
	vector<vector<Point>> contour_lines;
	findContours(closed, contour_lines, CV_RETR_LIST, CV_CHAIN_APPROX_NONE); //外轮廓 和

	Mat img_contours(closed.size(), CV_8U, Scalar(255));
	drawContours(img_contours, contour_lines, -1, Scalar(0), 2);
	Mat img_contour_lines_copy = img_contours.clone();
    //imshow(img_name + "连通通区域轮廓图", change_size(img_contours, 0.4));
	char type_char[4];
	if (issave == true)
	{
		_itoa(type,type_char,10);
		char wd[8],ht[8];
		_itoa(size.width,wd,10);
		_itoa(size.height,ht,10);
		string sizeinf =string(wd) + " X "+string(ht);
		//cout <<sizeinf.c_str()<<endl;
		if (sign == 1)
		{ 
			if (flag == -1)
			{
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"闭运算图"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"闭运算连通通区域轮廓图"+".jpg",img_contour_lines_copy);
			}else if(flag == 1)
			{
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"高亮区闭运算图"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"高亮区闭运算连通通区域轮廓图"+".jpg",img_contour_lines_copy);
			}
		  
		}else if(sign == 0)
		{
			 if (flag == -1)
			{
				imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"闭运算连通通区域轮廓图"+".jpg",img_contour_lines_copy);
			}else if(flag == 1)
			{
				imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"高亮区闭运算连通通区域轮廓图"+".jpg",img_contour_lines_copy);
			}
		}else if (sign == -1)
		{
			 if (flag == -1)
			{
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"开运算图"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"开运算连通通区域轮廓图"+".jpg",img_contour_lines_copy);
			}else if(flag == 1)
			{
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"高亮区开运算图"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"高亮区开运算连通通区域轮廓图"+".jpg",img_contour_lines_copy);
		    }
		}
	}
	return contour_lines;
}

/**
	判断两个rect的重合率是否大于min_ratio

*/
bool isrepeat(Rect v1, Rect v2,float min_ratio)
{
	int x1,y1,wd1,ht1,x2,y2,wd2,ht2,endx,startx,wd,endy,starty,ht;
	float ratio = 0,area,area1,area2;
	x1 = v1.x;
	y1 = v1.y;
	wd1 =v1.width;
	ht1 =v1.height;
	x2 = v2.x;
	y2 = v2.y;
	wd2 =v2.width;
	ht2 =v2.height;

	endx = max(x1 + wd1, x2 + wd2);
	startx = min(x1,x2);
	wd = wd1 + wd2 - (endx - startx);

	endy = max(y1 + ht1, y2 + ht2);
	starty = min(y1,y2);
	ht = ht1 + ht2 - (endy - starty);

	if(wd <= 0 || ht <= 0)
	{
		ratio = 0;

	}else
	{
		area = wd * ht;
		area1 = wd1 * ht1;
		area2 = wd2 * ht2;
		ratio = area /(area1 + area2 - area);
	}

	if (ratio >= min_ratio)
	{
		return true;
	}
	return false;
}

///** 不知道问题出哪儿了效果不对
//	把Rect 包裹一次重载 == 运算符
//*/
//typedef struct myRect
//{
//	Rect rect;
//	float min_ratio;
//	bool operator ==(const myRect &other)const
//	{
//		return (isrepeat(this->rect,other.rect,this->min_ratio));
//	}
//}myRect;
//
//namespace std
//{
//	template<>
//	struct hash<myRect>
//	{
//		typedef size_t result_type;
//		typedef myRect argument_type;
//		size_t operator()(const myRect r)const;
//	};
//	size_t hash<myRect>:: operator()(const myRect r)const
//	{
//		return r.rect.x + r.rect.y;
//	}
//}
//
///**
//	rect约减 如果两个rect的重合度大于 min_ratio就判定这两个rect是相同的
//	输入： 有“重复”rect的rect的向量repeat_rect； 最小判定重合的重合率（0->1） min_ratio
//	输出：约减之后的distinct_rect;
//*/
//vector<Rect> rect_decrease( vector<Rect> repeat_rects,float min_ratio)
//{
//	vector<Rect> distinct_rects;
//    unordered_set<myRect> myRects;
//	vector<myRect> myRepeat_rects;
//	myRect myrect_temp;
//	Rect rect_temp;
//	cout <<"包裹以前Rect个数：" <<repeat_rects.size()<<endl;
//	//包裹Rect
//	for (int i = 0; i <repeat_rects.size(); i++)
//	{
//		myrect_temp.rect = repeat_rects[i];
//		myrect_temp.min_ratio = min_ratio;
//		myRepeat_rects.push_back(myrect_temp);
//	}
//	myRects.insert(myRepeat_rects.cbegin(),myRepeat_rects.cend());
//	//解包裹
//	unordered_set<myRect>::iterator set_it = myRects.begin();
//	while (set_it != myRects.end())
//	{
//		rect_temp = (*set_it).rect;
//		distinct_rects.push_back(rect_temp);
//		++set_it;
//	}
//	cout <<"包裹以后Rect个数：" <<distinct_rects.size()<<endl;
//	return distinct_rects;
//}

/**
	rect约减 如果两个rect的重合度大于 min_ratio就判定这两个rect是相同的
	输入： 有“重复”rect的rect的向量repeat_rect; 最小判定重合的重合率（0->1） min_ratio;是否需要保存约减的信息 isneedinfo ; 保存约减信息的字符串引用
	输出：约减之后的distinct_rect;
*/
vector<Rect> rect_decrease( vector<Rect> repeat_rects,float min_ratio,bool isneedinfo = false,string &datainfo = string())
{
	vector<Rect> distinct_rects;
	string info;
	char num_str[8];
	_itoa(repeat_rects.size(),num_str,10);
	info.append(string(" 约减以前Rect个数：" )+ num_str + "\n");

	int repeat_count = 0;
	for (int i = 0; i <repeat_rects.size() ; i++)
	{
		repeat_count = 0;
		for (int j = i + 1; j < repeat_rects.size(); j++)
		{
			if(isrepeat(repeat_rects[i],repeat_rects[j],min_ratio))
			{
			  repeat_count++;
			  break;
			}
		}
		if (repeat_count == 0 && repeat_rects[i].width >= repeat_rects[i].height  && repeat_rects[i].width <= 6 * repeat_rects[i].height && repeat_rects[i].width >= 40 && repeat_rects[i].width <= 400) //说明不重复 并且 rect的高不能大于宽 宽不能大于高的4倍
		//if (repeat_rects[i].width >= repeat_rects[i].height && repeat_rects[i].width <= 5.5 * repeat_rects[i].height && repeat_rects[i].width >= 40 && repeat_rects[i].width <= 400) //说明不重复 并且 rect的高不能大于宽 宽不能大于高的4倍
		{
			distinct_rects.push_back(repeat_rects[i]);
		}
	}

	_itoa(distinct_rects.size(),num_str,10);
	info.append(string(" 约减以后Rect个数：" )+ num_str + "\n");

	if(isneedinfo)
	{
		datainfo.append(info); 
	}
	return distinct_rects;
}


/**
	自己写Sobel算子 检测垂直边缘
	输入：灰度图image
	输出：边缘检测后的图
*/
Mat SobelS(Mat image)
{
	Mat img = image.clone();
	img.convertTo(img,CV_32F);
	Mat rt(img.size(),CV_32F,Scalar::all(0));
	int nr = img.rows; //行数
    int nc = img.cols;
	float pixe;
	float min_thresh = 10;
	int delta = 10;
    for (size_t i = delta; i < nr - delta; i++)
    {
        for (size_t j = delta; j < nc - delta; j++)
        {
			//如果方块的左边或者右边全都是0 那么它本身也就是0 避免检测到分块时候出现的边缘
			if ( (img.at<float>(i-1,j-delta) < min_thresh) || (img.at<float>(i,j-delta)  < min_thresh) || (img.at<float>(i+1,j-delta) < min_thresh)  || (img.at<float>(i-1,j+delta) < min_thresh) || (img.at<float>(i,j+delta)   < min_thresh)  || (img.at<float>(i+1,j+delta) < min_thresh) )
            {
				rt.at<float>(i,j) = 0;
			}else
			{
				pixe = (img.at<float>(i-1,j-1) +  2 *img.at<float>(i,j-1) + img.at<float>(i+1,j-1)) - ( img.at<float>(i-1,j+1) + 2*img.at<float>(i,j+1) + img.at<float>(i+1,j+1));
				if (pixe >= 0)
				{
					rt.at<float>(i,j) =  pixe;
				}else
				{
					rt.at<float>(i,j) = -pixe;
				}
			}
        }
    }
	rt.convertTo(rt,CV_8U);
	return rt;
}


/**
  通过灰度图 的 边缘检测 和 全局OSTU 最后得到可能是车牌的RECT 
  输入：灰度图片 image_gray;带回Rect结果的possible_plate_rects;图片名称img_name;约减率 rect_ratio; 标定是高亮区或者暗区（1，-1）sign; 是否需要保存处理过程中的信息 &datainfo;是否需要保存处理过程中的图片 issave;中间图片保存的路径
  输出：bool
*/
bool get_rects_form_contours(Mat image_gray,vector<Rect> *possible_plate_rects,string img_name,float rect_ratio,int sign, bool isneedinfo = false,string &datainfo = string(),bool issave = false,string save_path = NULL)
{
	Mat img_gray = image_gray;
	Mat img_threshold;
	Mat img_threshold_copy;
	Mat img_contours_copy;
	string info;
    /**
		高斯滤波
	*/
	GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);

	/**
	    边缘检测 自己写
	*/

	//Sobel(img_gray, img_gray, CV_8U, 1, 0); //垂直边缘
	img_gray = SobelS(img_gray);
    img_contours_copy = img_gray.clone();

	/**
	根据阈值 进行二值化
	*/
	threshold(img_gray,img_threshold, 1,255, THRESH_BINARY | THRESH_OTSU);
	//img_threshold = middleThresh(img_gray);
	img_threshold_copy = img_threshold.clone();
	
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

    int cmin = 40 * 2;
	int cmax = 400 * 4 - 1;
	Mat img_contours_less(img_gray.size(), CV_8U, Scalar(255));
	vector<Rect> possible_plates;
	Rect r;
	char num_str[8];
	for (int i = 0; i < element_sizes.size(); i++)
	{
		//获得轮廓
		contour_lines_temp = get_contours(img_threshold,element_sizes[i],1,img_name,ContoursADDLocalOSTU,issave,save_path,sign);

		_itoa(contour_lines_temp.size(),num_str,10);
		info.append(string("处理前轮廓个数") + num_str + " \n");

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
		info.append(string("处理后轮廓个数") + num_str + " \n");

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
	possible_plates = rect_decrease(possible_plates,rect_ratio,isneedinfo,info);
	(*possible_plate_rects).insert((*possible_plate_rects).begin(),possible_plates.begin(),possible_plates.end());

	/**
		中间处理信息的保存
	*/
	if (isneedinfo == true)
	{
		datainfo.append(info);
	}

    /**
		中间图片的保存
	*/
	
	if (issave == true)
	{
		char type_char[8] = "__4__";
		if (sign == 1)
		{
			imwrite(save_path + img_name+ type_char+"边缘图_高亮区" +".jpg",img_contours_copy);

			imwrite(save_path + img_name+ type_char+"二值化后的图_高亮区" +".jpg",img_threshold_copy);

			imwrite(save_path + img_name+ type_char+"连通通区域轮廓图去掉小轮廓_高亮区" +".jpg",img_contours_less_copy);
		}else if (sign == -1)
		{
			imwrite(save_path + img_name+ type_char+"边缘图_暗区" +".jpg",img_contours_copy);

			imwrite(save_path + img_name+ type_char+"二值化后的图_暗区" +".jpg",img_threshold_copy);

			imwrite(save_path + img_name+ type_char+"连通通区域轮廓图去掉小轮廓_暗区" +".jpg",img_contours_less_copy);
		}
	}
	return true;
}


#endif //!GETPLATEBASE