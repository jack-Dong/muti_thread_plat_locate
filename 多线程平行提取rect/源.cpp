#include"getplate.h"
#include"checkplate.h"
int main()
{   
	float right_ratio = 0,right_ratio_sum = 0;
	string filePath_front = "C:\\Users\\dsz\\Desktop\\���Ƽ��\\";  //'\' ��ʾת���
	string filePath_middle;
	string save_path = "C:\\Users\\dsz\\Desktop\\";
	string filePath;
	char i_char[4];
	vector<TestImage> testImages;
	vector<float> right_score;
	for (int i = 26; i < 27; i++)
	//for (int i = 0; i < 27; i++)
	{
		right_ratio = 0;
		filePath_middle = "test";
		_itoa(i+1,i_char,10);
		filePath_middle = filePath_middle + i_char; 
		filePath = filePath_front + filePath_middle + "\\";
		cout<< filePath.c_str() <<endl;
		//vector<string> files;
		////��ȡ���и�ʽΪjpg���ļ�  
		//string format = ".jpg";
		//GetAllFormatFiles(filePath, files, format);
	
		//���ļ����¶�ȡ test5.txt ���ɶ���testImage
		testImages =  gettestImages(filePath,filePath_middle+".txt");

		/*imshow("��ȡ����ͼƬ"+ testImages[1].img_name, change_size(testImages[1].img,0.4));
		Mat rt = regulate(testImages[1].img,1,5);
		imshow("��ǿ���ͼƬ"+ testImages[1].img_name,change_size(rt,0.4));*/

		//���ʶ��׼ȷ��
		right_ratio = accuracy(testImages,true,save_path + filePath_middle + "\\");
		cout << "============================================================= \n"<<endl;
		cout <<filePath_middle+"��׼ȷ��Ϊ��"<<right_ratio<<"\n"<<endl;
		cout << "============================================================= \n"<<endl;
		right_score.push_back(right_ratio);
		right_ratio_sum += right_ratio;
		testImages.clear();
	}

	for (int i = 0; i < right_score.size(); i++)
	{
		cout<<"test"<< i + 1 <<" = " <<right_score[i] <<"     ||";
	}
	cout << "\n���в����ļ����ܵ���ȷ�ʣ�" << right_ratio_sum / 27 <<endl;
	return 0;
}
