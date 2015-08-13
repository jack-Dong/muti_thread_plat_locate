#ifndef GETPLATEBASE
#define GETPLATEBASE
#include"thresholdbyblock.h"
#include"regulation.h"
#include"mad.h"
#include"color.h"
/**
��������ͼƬ��rect�Ĳ�ͬ��ʽ
*/
enum Query_rect_type{ RegulationAddLocalMiddlethresh = 1,NocontoursADDWholeOSTU,ColorThresh,ContoursADDLocalOSTU,LocalMiddlethresh,MadAddWholeOSTU};

/**
 �߳���������ṹ��
*/
struct ThreadParamter
{
	Mat image;
	vector<Rect> *possible_plate_rects;
	string img_name;
	float rect_ratio;
	bool issave;
	string save_path;
	float k1; //������չ����
	float k2; //�Աȶ���չ����
};
//����3���¼� ���̵߳ȴ��������߳�ִ����ɲż���ִ��
HANDLE event1; HANDLE event2; HANDLE event3; HANDLE event4; 

/**
ͼƬ̫�󲻺�����ͱ�Сһ��
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
	��̬ѧ�����õ���ͨ����
	���룺 ��Ե����ĻҶ�ͼ contour; ������ĺ˵Ĵ�СSize; ͼƬ������ string name; int sign(1,�գ�-1��ʴ) ;�Ƿ񱣴洦���м�ͼƬbool issave ͼƬ�ı���·�� string save_path ; flag (-1���� 1������)
	����� ��ͨ������ vector<vector<Point>> contour_lines
*/
vector<vector<Point>> get_contours(Mat contour, Size size,int sign,string img_name,Query_rect_type type,bool issave =false, string save_path = NULL, int flag = -1)
{
    /**
		��̬ѧ����
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
	//imshow(img_name+"������ͼ", change_size(closed, 0.4));

	/**
		��ȡ��ͨͨ��������
	*/
	vector<vector<Point>> contour_lines;
	findContours(closed, contour_lines, CV_RETR_LIST, CV_CHAIN_APPROX_NONE); //������ ��

	Mat img_contours(closed.size(), CV_8U, Scalar(255));
	drawContours(img_contours, contour_lines, -1, Scalar(0), 2);
	Mat img_contour_lines_copy = img_contours.clone();
    //imshow(img_name + "��ͨͨ��������ͼ", change_size(img_contours, 0.4));
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
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"������ͼ"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"��������ͨͨ��������ͼ"+".jpg",img_contour_lines_copy);
			}else if(flag == 1)
			{
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"������������ͼ"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"��������������ͨͨ��������ͼ"+".jpg",img_contour_lines_copy);
			}
		  
		}else if(sign == 0)
		{
			 if (flag == -1)
			{
				imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"��������ͨͨ��������ͼ"+".jpg",img_contour_lines_copy);
			}else if(flag == 1)
			{
				imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"��������������ͨͨ��������ͼ"+".jpg",img_contour_lines_copy);
			}
		}else if (sign == -1)
		{
			 if (flag == -1)
			{
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"������ͼ"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"��������ͨͨ��������ͼ"+".jpg",img_contour_lines_copy);
			}else if(flag == 1)
			{
				 imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"������������ͼ"+".jpg",img_closed_copy);
		         imwrite(save_path + img_name+"__"+type_char+"__"+sizeinf+"��������������ͨͨ��������ͼ"+".jpg",img_contour_lines_copy);
		    }
		}
	}
	return contour_lines;
}

/**
	�ж�����rect���غ����Ƿ����min_ratio

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

///** ��֪��������Ķ���Ч������
//	��Rect ����һ������ == �����
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
//	rectԼ�� �������rect���غ϶ȴ��� min_ratio���ж�������rect����ͬ��
//	���룺 �С��ظ���rect��rect������repeat_rect�� ��С�ж��غϵ��غ��ʣ�0->1�� min_ratio
//	�����Լ��֮���distinct_rect;
//*/
//vector<Rect> rect_decrease( vector<Rect> repeat_rects,float min_ratio)
//{
//	vector<Rect> distinct_rects;
//    unordered_set<myRect> myRects;
//	vector<myRect> myRepeat_rects;
//	myRect myrect_temp;
//	Rect rect_temp;
//	cout <<"������ǰRect������" <<repeat_rects.size()<<endl;
//	//����Rect
//	for (int i = 0; i <repeat_rects.size(); i++)
//	{
//		myrect_temp.rect = repeat_rects[i];
//		myrect_temp.min_ratio = min_ratio;
//		myRepeat_rects.push_back(myrect_temp);
//	}
//	myRects.insert(myRepeat_rects.cbegin(),myRepeat_rects.cend());
//	//�����
//	unordered_set<myRect>::iterator set_it = myRects.begin();
//	while (set_it != myRects.end())
//	{
//		rect_temp = (*set_it).rect;
//		distinct_rects.push_back(rect_temp);
//		++set_it;
//	}
//	cout <<"�����Ժ�Rect������" <<distinct_rects.size()<<endl;
//	return distinct_rects;
//}

/**
	rectԼ�� �������rect���غ϶ȴ��� min_ratio���ж�������rect����ͬ��
	���룺 �С��ظ���rect��rect������repeat_rect; ��С�ж��غϵ��غ��ʣ�0->1�� min_ratio;�Ƿ���Ҫ����Լ������Ϣ isneedinfo ; ����Լ����Ϣ���ַ�������
	�����Լ��֮���distinct_rect;
*/
vector<Rect> rect_decrease( vector<Rect> repeat_rects,float min_ratio,bool isneedinfo = false,string &datainfo = string())
{
	vector<Rect> distinct_rects;
	string info;
	char num_str[8];
	_itoa(repeat_rects.size(),num_str,10);
	info.append(string(" Լ����ǰRect������" )+ num_str + "\n");

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
		if (repeat_count == 0 && repeat_rects[i].width >= repeat_rects[i].height  && repeat_rects[i].width <= 6 * repeat_rects[i].height && repeat_rects[i].width >= 40 && repeat_rects[i].width <= 400) //˵�����ظ� ���� rect�ĸ߲��ܴ��ڿ� ���ܴ��ڸߵ�4��
		//if (repeat_rects[i].width >= repeat_rects[i].height && repeat_rects[i].width <= 5.5 * repeat_rects[i].height && repeat_rects[i].width >= 40 && repeat_rects[i].width <= 400) //˵�����ظ� ���� rect�ĸ߲��ܴ��ڿ� ���ܴ��ڸߵ�4��
		{
			distinct_rects.push_back(repeat_rects[i]);
		}
	}

	_itoa(distinct_rects.size(),num_str,10);
	info.append(string(" Լ���Ժ�Rect������" )+ num_str + "\n");

	if(isneedinfo)
	{
		datainfo.append(info); 
	}
	return distinct_rects;
}


/**
	�Լ�дSobel���� ��ⴹֱ��Ե
	���룺�Ҷ�ͼimage
	�������Ե�����ͼ
*/
Mat SobelS(Mat image)
{
	Mat img = image.clone();
	img.convertTo(img,CV_32F);
	Mat rt(img.size(),CV_32F,Scalar::all(0));
	int nr = img.rows; //����
    int nc = img.cols;
	float pixe;
	float min_thresh = 10;
	int delta = 10;
    for (size_t i = delta; i < nr - delta; i++)
    {
        for (size_t j = delta; j < nc - delta; j++)
        {
			//����������߻����ұ�ȫ����0 ��ô������Ҳ����0 �����⵽�ֿ�ʱ����ֵı�Ե
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
  ͨ���Ҷ�ͼ �� ��Ե��� �� ȫ��OSTU ���õ������ǳ��Ƶ�RECT 
  ���룺�Ҷ�ͼƬ image_gray;����Rect�����possible_plate_rects;ͼƬ����img_name;Լ���� rect_ratio; �궨�Ǹ��������߰�����1��-1��sign; �Ƿ���Ҫ���洦������е���Ϣ &datainfo;�Ƿ���Ҫ���洦������е�ͼƬ issave;�м�ͼƬ�����·��
  �����bool
*/
bool get_rects_form_contours(Mat image_gray,vector<Rect> *possible_plate_rects,string img_name,float rect_ratio,int sign, bool isneedinfo = false,string &datainfo = string(),bool issave = false,string save_path = NULL)
{
	Mat img_gray = image_gray;
	Mat img_threshold;
	Mat img_threshold_copy;
	Mat img_contours_copy;
	string info;
    /**
		��˹�˲�
	*/
	GaussianBlur(img_gray, img_gray, Size(5, 5), 1.5);

	/**
	    ��Ե��� �Լ�д
	*/

	//Sobel(img_gray, img_gray, CV_8U, 1, 0); //��ֱ��Ե
	img_gray = SobelS(img_gray);
    img_contours_copy = img_gray.clone();

	/**
	������ֵ ���ж�ֵ��
	*/
	threshold(img_gray,img_threshold, 1,255, THRESH_BINARY | THRESH_OTSU);
	//img_threshold = middleThresh(img_gray);
	img_threshold_copy = img_threshold.clone();
	
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

    int cmin = 40 * 2;
	int cmax = 400 * 4 - 1;
	Mat img_contours_less(img_gray.size(), CV_8U, Scalar(255));
	vector<Rect> possible_plates;
	Rect r;
	char num_str[8];
	for (int i = 0; i < element_sizes.size(); i++)
	{
		//�������
		contour_lines_temp = get_contours(img_threshold,element_sizes[i],1,img_name,ContoursADDLocalOSTU,issave,save_path,sign);

		_itoa(contour_lines_temp.size(),num_str,10);
		info.append(string("����ǰ��������") + num_str + " \n");

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
		info.append(string("�������������") + num_str + " \n");

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
	possible_plates = rect_decrease(possible_plates,rect_ratio,isneedinfo,info);
	(*possible_plate_rects).insert((*possible_plate_rects).begin(),possible_plates.begin(),possible_plates.end());

	/**
		�м䴦����Ϣ�ı���
	*/
	if (isneedinfo == true)
	{
		datainfo.append(info);
	}

    /**
		�м�ͼƬ�ı���
	*/
	
	if (issave == true)
	{
		char type_char[8] = "__4__";
		if (sign == 1)
		{
			imwrite(save_path + img_name+ type_char+"��Եͼ_������" +".jpg",img_contours_copy);

			imwrite(save_path + img_name+ type_char+"��ֵ�����ͼ_������" +".jpg",img_threshold_copy);

			imwrite(save_path + img_name+ type_char+"��ͨͨ��������ͼȥ��С����_������" +".jpg",img_contours_less_copy);
		}else if (sign == -1)
		{
			imwrite(save_path + img_name+ type_char+"��Եͼ_����" +".jpg",img_contours_copy);

			imwrite(save_path + img_name+ type_char+"��ֵ�����ͼ_����" +".jpg",img_threshold_copy);

			imwrite(save_path + img_name+ type_char+"��ͨͨ��������ͼȥ��С����_����" +".jpg",img_contours_less_copy);
		}
	}
	return true;
}


#endif //!GETPLATEBASE