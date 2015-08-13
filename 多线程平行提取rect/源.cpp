#include"getplate.h"
#include"checkplate.h"
int main()
{   
	float right_ratio = 0,right_ratio_sum = 0;
	string filePath_front = "C:\\Users\\dsz\\Desktop\\车牌检测\\";  //'\' 表示转义符
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
		////读取所有格式为jpg的文件  
		//string format = ".jpg";
		//GetAllFormatFiles(filePath, files, format);
	
		//从文件夹下读取 test5.txt 生成对象testImage
		testImages =  gettestImages(filePath,filePath_middle+".txt");

		/*imshow("读取到的图片"+ testImages[1].img_name, change_size(testImages[1].img,0.4));
		Mat rt = regulate(testImages[1].img,1,5);
		imshow("增强后的图片"+ testImages[1].img_name,change_size(rt,0.4));*/

		//获得识别准确度
		right_ratio = accuracy(testImages,true,save_path + filePath_middle + "\\");
		cout << "============================================================= \n"<<endl;
		cout <<filePath_middle+"的准确度为："<<right_ratio<<"\n"<<endl;
		cout << "============================================================= \n"<<endl;
		right_score.push_back(right_ratio);
		right_ratio_sum += right_ratio;
		testImages.clear();
	}

	for (int i = 0; i < right_score.size(); i++)
	{
		cout<<"test"<< i + 1 <<" = " <<right_score[i] <<"     ||";
	}
	cout << "\n所有测试文件的总的正确率：" << right_ratio_sum / 27 <<endl;
	return 0;
}
