#ifndef CHECK_PLATE
#define NameMaxLength 200
#define CHECK_PLATE
#include"base.h"
#include"getplate.h"
struct Plate
{
	Rect rect; //矩形位置
	int speciaflag; //是否是特殊车牌，不便于辨认的车牌 0-红色（普通） 1-黄色（难） 2-蓝色
	bool isRecongnised; //是否识别出来了
};


/**
封装测试图片的数据和方法
*/
class TestImage
{
public:
	/**
	初始化方法
	输入：测试图片的名字string img_name， 测试的图片：img , 车牌向量：vector<Plate> plates;
	*/
	TestImage(string img_name, Mat img,vector<Plate> plates);
    //公开属性：
	string img_name;
	Mat img;  //测试图片
	vector<Plate> plates; //车牌
	unsigned num; //车牌个数
	//bool isdraw; //是否被标注上了正确的矩形框
	//公开方法：
	/**
	 检查是否定位准确
	 输入：可能是车牌的矩形： vector<Rect> possible_plates 以及最小重合率： min_ratio
	 输出：void ，结果保存在 车牌结构体中
	*/
	void check(vector<Rect> possible_plates,float min_ratio);
private:
	//私有属性：

	//私有方法：
};
/**
  初始化方法
  输入：测试图片的名字string img_name， 测试的图片：img , 车牌向量：vector<Plate> plates;
*/
TestImage::TestImage(string img_name,Mat img,vector<Plate> plates)
{
	this->img = img;
	this->plates = plates;
	this->num = plates.size();
	this->img_name = img_name;
}

/**
	 检查是否定位准确
	 输入：可能是车牌的矩形： vector<Rect> possible_plates 以及最小重合率 min_ratio
	 输出：void ，结果保存在 车牌结构体isRecongnised中
	*/
void TestImage::check(vector<Rect> possible_plates,float min_ratio)
{
	//每个TestImage对象中的矩形框与可能是车牌的矩形一一对比 重合比例超过 就算找到了 
	for (int i = 0; i < this->plates.size(); i++)
	{      
		for (int j = 0; j < possible_plates.size(); j++)
		{
			if ( isrepeat(this->plates[i].rect,possible_plates[j],min_ratio) == true)
			{
				this->plates[i].isRecongnised = true;
				//根据speciaflag 标识 在原图上画出本来人工标注的矩形框，0-红色 1-黄色 2-蓝色
				if (this->plates[i].speciaflag == 0)
				{
					rectangle(this->img, this->plates[i].rect, Scalar(0, 0, 255, 0), 2);
				}else if (this->plates[i].speciaflag == 1)
				{
					rectangle(this->img, this->plates[i].rect, Scalar(0, 255, 255, 0), 2);
				}else if (this->plates[i].speciaflag == 2)
				{
					rectangle(this->img, this->plates[i].rect, Scalar(255, 0, 0, 0), 2);
				}
				//在原图上把找到的矩形用青色画出来
				cout << "---------------"<<endl;
				cout << "|正确检测到车牌|" <<endl;
				cout << "---------------"<<endl;
				rectangle(this->img, possible_plates[j], Scalar(255,255,0), 2);
				break;
			}
		}
	}
}

/**
分割字符串
*/
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));
 
    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}

/**
  把整个文件夹下面的测试图片全部封装成testImage对象 以vector的方式返回
  输入：文件夹路径 string path，文本文件名称 string name.如"test27.txt"
  输出：vector<testImage>
*/
vector<TestImage> gettestImages(string path,string name)
{
	vector<TestImage> testImages;
	ifstream testdata(path + name);
	if (!testdata.is_open())
	{
		cout <<(path + name).c_str()<< "文件打开失败" <<endl;
	}
	//规定图片名称不能超过50个字节 每个矩形框记录要占25个字节 矩形框不超过5个
	char aline[NameMaxLength];
	string platinfo;
	vector<string> plateinfov;
	string filename; //测试图片名称不包含路径
	Mat img;  //测试图片
	vector<Plate> plates; //车牌
	unsigned num; //车牌个数
	Plate plate;
	Size img_size;
	float zoom_ratio;
	int new_wd;
	int new_ht;
	while (testdata.getline(aline,NameMaxLength))
	{
		platinfo = aline;
		//cout << aline <<"    长度：" << platinfo.size() <<endl;
		plateinfov.clear();
		SplitString(platinfo,plateinfov," ");
		//cout <<"plateinfov.size="<<plateinfov.size() <<endl;
		plates.clear();
		num = 0;
		filename = plateinfov[0]; //取出名字
		img = imread(path+filename); //根据名字取出图片
		//等比例缩放 保证宽度为 1306
		img_size = img.size();
		zoom_ratio  = 1306.0 / img_size.width;
		new_wd = zoom_ratio * img_size.width;
		new_ht = zoom_ratio * img_size.height; 

		resize(img,img,Size(new_wd,new_ht));//对车牌图片大小统一一下

		if (img.empty())
		{
			cout << path+filename + "图片未能读取到" <<endl;
		}
		//imshow("读取到的图片"+ filename, change_size(img,0.4));
		num = atoi(plateinfov[1].c_str()); //取出车牌个数
		for (int i = 0; i < num ; i++)
		{
			plate.isRecongnised = false;
			plate.speciaflag = atoi(plateinfov[i * 5 + 2].c_str()) * zoom_ratio;
			plate.rect.x = atoi(plateinfov[i * 5 + 3].c_str()) * zoom_ratio;
			plate.rect.y = atoi(plateinfov[i * 5 + 4].c_str()) * zoom_ratio;
			plate.rect.width = atoi(plateinfov[i * 5 + 5].c_str()) * zoom_ratio;
			plate.rect.height = atoi(plateinfov[i * 5 + 6].c_str()) * zoom_ratio;
			//cout << plate.rect <<endl; 
			plates.push_back(plate);
		}
		TestImage testImage(filename,img,plates); //生成一个testimage 对象
		//imshow("读取到的图片"+ testImage.img_name, change_size(testImage.img,0.4));
		testImages.push_back(testImage);
	}
	return testImages;
}


/**
	遍历所有TestImage 对象 计算整个文件夹的识别率
	输入：testImage对象向量 vector<TestImage> testImages, 是否把测试的图片都保存下来bool issave 图片的保存路径 save_path
*/
float accuracy(vector<TestImage> testImages,bool issave,string save_path)
{
	float plates_sum = 0,recongnised_sum = 0;
    vector<Rect> possible_plates;
	vector<Rect> possible_plates1;
	vector<Rect> possible_plates2;
	vector<Rect> possible_plates3;
	vector<Rect> possible_plates4;
	vector<Rect> possible_plates5;
    // for (int i = 4; i < 10; i++)
	for (int i = 0; i < testImages.size(); i++)
	{
		cout <<"=========================================================" <<endl;
		cout << "正在处理第" << i << "张图片,名称：" <<  testImages[i].img_name <<"车牌个数："<<testImages[i].plates.size()<<endl;
		//imshow("读取到的图片"+ testImages[i].img_name, change_size(testImages[i].img,0.4));

		//生成线程所需参数
		ThreadParamter tp1,tp2,tp3,tp4;
		tp1.image = testImages[i].img; tp1.possible_plate_rects = &possible_plates1;tp1.img_name = testImages[i].img_name;
		tp1.k1 = 1,tp1.k2 = 5; tp1.rect_ratio = 0.7; tp1.issave = issave; tp1.save_path = save_path;

		tp2.image = testImages[i].img; tp2.possible_plate_rects = &possible_plates2;tp2.img_name = testImages[i].img_name;
		tp2.k1 = 1,tp2.k2 = 5; tp2.rect_ratio = 0.7; tp2.issave = issave; tp2.save_path = save_path;

		tp3.image = testImages[i].img; tp3.possible_plate_rects = &possible_plates3;tp3.img_name = testImages[i].img_name;
		tp3.k1 = 1,tp3.k2 = 5; tp3.rect_ratio = 0.7; tp3.issave = issave; tp3.save_path = save_path;

		tp4.image = testImages[i].img; tp4.possible_plate_rects = &possible_plates4;tp4.img_name = testImages[i].img_name;
		tp4.k1 = 1,tp4.k2 = 5; tp4.rect_ratio = 0.7; tp4.issave = issave; tp4.save_path = save_path;

		//HANDLE hThread1 = CreateThread(NULL,0,getplateRectbyRegulationAddLocalMiddlethresh,&tp1,0,NULL);
	    //HANDLE hThread2 = CreateThread(NULL,0,getplateRectbyNocontoursADDWholeOSTU,&tp2,0,NULL);
		HANDLE hThread3 = CreateThread(NULL,0,getplateRectbyColorThresh,&tp3,0,NULL);
		//HANDLE hThread4 = CreateThread(NULL,0,getplateRectbyContoursADDLocalOSTU,&tp4,0,NULL);
		event1 = CreateEvent(NULL,TRUE,FALSE,NULL);
		event2 = CreateEvent(NULL,TRUE,FALSE,NULL);
		event3 = CreateEvent(NULL,TRUE,FALSE,NULL);
		event4 = CreateEvent(NULL,TRUE,FALSE,NULL);
		//CloseHandle(hThread1);
		//CloseHandle(hThread2);
		CloseHandle(hThread3);
		//CloseHandle(hThread4);

		//WaitForSingleObject(event1,INFINITE);
		//WaitForSingleObject(event2,INFINITE);
		WaitForSingleObject(event3,INFINITE);
		//WaitForSingleObject(event4,INFINITE);

	    possible_plates.insert(possible_plates.end(),possible_plates1.begin(),possible_plates1.end());
		possible_plates.insert(possible_plates.end(),possible_plates2.begin(),possible_plates2.end());
		possible_plates.insert(possible_plates.end(),possible_plates3.begin(),possible_plates3.end());
		possible_plates.insert(possible_plates.end(),possible_plates4.begin(),possible_plates4.end());


		string datainfo = "全部轮廓的约减：\n";
		possible_plates = rect_decrease(possible_plates,0.5,true,datainfo);
		cout << datainfo<<endl;
	
		testImages[i].check(possible_plates,0.3);
		cout <<"=========================================================" <<endl;
		plates_sum += testImages[i].num;
		for (int j = 0; j  < testImages[i].num; j ++)
		{
			if (testImages[i].plates[j].isRecongnised == true)
			{
				++recongnised_sum;
			}
		}

		if (issave == true)
		{
			Mat rect_img(testImages[i].img.size(),CV_8UC3,Scalar::all(0));
			for (int i = 0; i < possible_plates.size(); i++)
			{
				   rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); //绿色

			}
			imwrite(save_path + testImages[i].img_name+"总体可能是车牌的矩形框图约减后.jpg",rect_img + testImages[i].img);
			imwrite(save_path +testImages[i].img_name+"最终识别结果.jpg",testImages[i].img);
		}

		possible_plates1.clear();
		possible_plates2.clear();
		possible_plates3.clear();
		possible_plates4.clear();
		possible_plates.clear();
		ResetEvent(event1);
		ResetEvent(event2);
		ResetEvent(event3);
		ResetEvent(event4);
	}
	return recongnised_sum / plates_sum;
}


#endif //!CHECK_PLATE