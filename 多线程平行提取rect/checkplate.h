#ifndef CHECK_PLATE
#define NameMaxLength 200
#define CHECK_PLATE
#include"base.h"
#include"getplate.h"
struct Plate
{
	Rect rect; //����λ��
	int speciaflag; //�Ƿ������⳵�ƣ������ڱ��ϵĳ��� 0-��ɫ����ͨ�� 1-��ɫ���ѣ� 2-��ɫ
	bool isRecongnised; //�Ƿ�ʶ�������
};


/**
��װ����ͼƬ�����ݺͷ���
*/
class TestImage
{
public:
	/**
	��ʼ������
	���룺����ͼƬ������string img_name�� ���Ե�ͼƬ��img , ����������vector<Plate> plates;
	*/
	TestImage(string img_name, Mat img,vector<Plate> plates);
    //�������ԣ�
	string img_name;
	Mat img;  //����ͼƬ
	vector<Plate> plates; //����
	unsigned num; //���Ƹ���
	//bool isdraw; //�Ƿ񱻱�ע������ȷ�ľ��ο�
	//����������
	/**
	 ����Ƿ�λ׼ȷ
	 ���룺�����ǳ��Ƶľ��Σ� vector<Rect> possible_plates �Լ���С�غ��ʣ� min_ratio
	 �����void ����������� ���ƽṹ����
	*/
	void check(vector<Rect> possible_plates,float min_ratio);
private:
	//˽�����ԣ�

	//˽�з�����
};
/**
  ��ʼ������
  ���룺����ͼƬ������string img_name�� ���Ե�ͼƬ��img , ����������vector<Plate> plates;
*/
TestImage::TestImage(string img_name,Mat img,vector<Plate> plates)
{
	this->img = img;
	this->plates = plates;
	this->num = plates.size();
	this->img_name = img_name;
}

/**
	 ����Ƿ�λ׼ȷ
	 ���룺�����ǳ��Ƶľ��Σ� vector<Rect> possible_plates �Լ���С�غ��� min_ratio
	 �����void ����������� ���ƽṹ��isRecongnised��
	*/
void TestImage::check(vector<Rect> possible_plates,float min_ratio)
{
	//ÿ��TestImage�����еľ��ο�������ǳ��Ƶľ���һһ�Ա� �غϱ������� �����ҵ��� 
	for (int i = 0; i < this->plates.size(); i++)
	{      
		for (int j = 0; j < possible_plates.size(); j++)
		{
			if ( isrepeat(this->plates[i].rect,possible_plates[j],min_ratio) == true)
			{
				this->plates[i].isRecongnised = true;
				//����speciaflag ��ʶ ��ԭͼ�ϻ��������˹���ע�ľ��ο�0-��ɫ 1-��ɫ 2-��ɫ
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
				//��ԭͼ�ϰ��ҵ��ľ�������ɫ������
				cout << "---------------"<<endl;
				cout << "|��ȷ��⵽����|" <<endl;
				cout << "---------------"<<endl;
				rectangle(this->img, possible_plates[j], Scalar(255,255,0), 2);
				break;
			}
		}
	}
}

/**
�ָ��ַ���
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
  �������ļ�������Ĳ���ͼƬȫ����װ��testImage���� ��vector�ķ�ʽ����
  ���룺�ļ���·�� string path���ı��ļ����� string name.��"test27.txt"
  �����vector<testImage>
*/
vector<TestImage> gettestImages(string path,string name)
{
	vector<TestImage> testImages;
	ifstream testdata(path + name);
	if (!testdata.is_open())
	{
		cout <<(path + name).c_str()<< "�ļ���ʧ��" <<endl;
	}
	//�涨ͼƬ���Ʋ��ܳ���50���ֽ� ÿ�����ο��¼Ҫռ25���ֽ� ���ο򲻳���5��
	char aline[NameMaxLength];
	string platinfo;
	vector<string> plateinfov;
	string filename; //����ͼƬ���Ʋ�����·��
	Mat img;  //����ͼƬ
	vector<Plate> plates; //����
	unsigned num; //���Ƹ���
	Plate plate;
	Size img_size;
	float zoom_ratio;
	int new_wd;
	int new_ht;
	while (testdata.getline(aline,NameMaxLength))
	{
		platinfo = aline;
		//cout << aline <<"    ���ȣ�" << platinfo.size() <<endl;
		plateinfov.clear();
		SplitString(platinfo,plateinfov," ");
		//cout <<"plateinfov.size="<<plateinfov.size() <<endl;
		plates.clear();
		num = 0;
		filename = plateinfov[0]; //ȡ������
		img = imread(path+filename); //��������ȡ��ͼƬ
		//�ȱ������� ��֤���Ϊ 1306
		img_size = img.size();
		zoom_ratio  = 1306.0 / img_size.width;
		new_wd = zoom_ratio * img_size.width;
		new_ht = zoom_ratio * img_size.height; 

		resize(img,img,Size(new_wd,new_ht));//�Գ���ͼƬ��Сͳһһ��

		if (img.empty())
		{
			cout << path+filename + "ͼƬδ�ܶ�ȡ��" <<endl;
		}
		//imshow("��ȡ����ͼƬ"+ filename, change_size(img,0.4));
		num = atoi(plateinfov[1].c_str()); //ȡ�����Ƹ���
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
		TestImage testImage(filename,img,plates); //����һ��testimage ����
		//imshow("��ȡ����ͼƬ"+ testImage.img_name, change_size(testImage.img,0.4));
		testImages.push_back(testImage);
	}
	return testImages;
}


/**
	��������TestImage ���� ���������ļ��е�ʶ����
	���룺testImage�������� vector<TestImage> testImages, �Ƿ�Ѳ��Ե�ͼƬ����������bool issave ͼƬ�ı���·�� save_path
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
		cout << "���ڴ����" << i << "��ͼƬ,���ƣ�" <<  testImages[i].img_name <<"���Ƹ�����"<<testImages[i].plates.size()<<endl;
		//imshow("��ȡ����ͼƬ"+ testImages[i].img_name, change_size(testImages[i].img,0.4));

		//�����߳��������
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


		string datainfo = "ȫ��������Լ����\n";
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
				   rectangle(rect_img,possible_plates[i], Scalar(0, 255, 0, 0), 2); //��ɫ

			}
			imwrite(save_path + testImages[i].img_name+"��������ǳ��Ƶľ��ο�ͼԼ����.jpg",rect_img + testImages[i].img);
			imwrite(save_path +testImages[i].img_name+"����ʶ����.jpg",testImages[i].img);
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