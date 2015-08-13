#ifndef MAD
#define MAD
#include"base.h"

/************************************************************************/
/* 根据公式计算矩形中心点的值                                                 */
/************************************************************************/
float calEnchancPixVal(float meanVal, float varianceVal, float pixVal)
{
	float coefficient = 0.0;
	uchar enhanceVal = 0;
	if (varianceVal<0)
	{
		return 0;
	}
	else if (varianceVal<20.0)
	{
		coefficient = 3/((varianceVal-20)*(varianceVal-20)/200+1);
	}
	else if (varianceVal<60.0&&varianceVal>=20.0)
	{
		coefficient = 3/((varianceVal-20)*(varianceVal-20)/800+1);
	}
	else
	{
		coefficient = 1;
	}
	float enhancePixVal = (pixVal-meanVal)*coefficient+meanVal;
	if (enhancePixVal>=255)
	{
		enhancePixVal = pixVal;
	}
	return enhancePixVal;
}
/************************************************************************/
/* 双线插值                                                                     */
/************************************************************************/
void interpolation(const Mat& src, Mat &dest, vector<Point> position, vector<float> blockMean,vector<float> blockVariance, int xNums, int yNums, int xWidth, int yHeight)
{
	int width = src.cols;
	int height = src.rows;
	int w=dest.cols;
	int h=dest.rows;
	int firPoint = 0;
	int secPoint = 0;
	int thirdPoint = 0;
	int forthPoint = 0;
	for (int i=0;i<yNums-1;i++)
	{
		for (int j=0;j<xNums-1;j++)
		{
			firPoint = i*xNums+j;
			secPoint = (i+1)*xNums+j;
			thirdPoint = firPoint+1;
			forthPoint = secPoint+1;
			for(int n=position[firPoint].y;n<position[forthPoint].y;n++)
			{
				for(int k=position[firPoint].x;k<position[forthPoint].x;k++)
				{
					float cx = 1.0*(k  - xWidth*j-xWidth*0.5)/xWidth;
					float cy = 1.0*(n - yHeight*i-yHeight*0.5)/yHeight;

					float tempMean = (1-cy)*((1-cx)*blockMean[firPoint]+cx*blockMean[thirdPoint])+cy*((1-cx)*blockMean[secPoint]+cx*blockMean[forthPoint]);
					float tempVariance = (1-cy)*((1-cx)*blockVariance[firPoint]+cx*blockVariance[thirdPoint])+cy*((1-cx)*blockVariance[secPoint]+cx*blockVariance[forthPoint]);
					float a = src.at<float>(n, k);
					float b = calEnchancPixVal(tempMean, tempVariance, a);
					dest.at<float>(n, k) = b;
				}
			}
		}
	}
}

/************************************************************************/
/* 灰度图像增强，使用局部灰度值均值和方差结合双线插值法                  */
/************************************************************************/
int enhanceImgByMAD(const Mat& ssrc, Mat& ddest, Size block)
{
	if (ssrc.empty())
	{
		return -1;
	}
	Mat src, dest;
	ssrc.convertTo(src, CV_32FC1);
	dest = src.clone();
	int imgWidth = src.cols;
	int imgHeight = src.rows;
	int xWidth = block.width;
	int yHeight = block.height;
	vector<Point> location;             //矩形顶点的位置
	vector<float> blockMean;			//矩形顶点的均值
	vector<float> blockVariance;		//矩形顶点的方差
	if (imgWidth/xWidth<2&&imgHeight/yHeight<2)
	{
		return -1;
	}
	if (block.width>=imgWidth||block.height>=imgHeight)
	{
		return -2;
	}
	int xNums = imgWidth/xWidth;
	int yNums = imgHeight/yHeight;
	int xyArea = xWidth*yHeight;
	Mat roiImg(xWidth, yHeight, src.type());
	Point startPoint(0,0);
	for(int i=0;i<yNums;i++)
	{
		startPoint.y=i*yHeight;
		for (int j=0;j<xNums;j++)
		{
			startPoint.x=j*xWidth;
			Rect rect(startPoint.x, startPoint.y, xWidth, yHeight);
			src(rect).copyTo(roiImg);
			float tempSum = 0.0;
			float temMean = 0.0;
			float tempVariance = 0.0;
			Point position(0,0);
			for (Mat_<float>::iterator it=roiImg.begin<float>();it!=roiImg.end<float>();++it)
			{  
				tempSum+=(*it);
			}
			temMean = tempSum/xyArea;
			tempSum = 0.0;
			for (Mat_<float>::iterator it=roiImg.begin<float>();it!=roiImg.end<float>();++it)
			{   
				tempSum+=(*it-temMean)*(*it-temMean);
			}
			tempVariance = std::sqrt(tempSum/xyArea);
			position.x = startPoint.x+xWidth/2;
			position.y = startPoint.y+yHeight/2;
			blockMean.push_back(temMean);
			blockVariance.push_back(tempVariance);
			location.push_back(position);
		}
	}
	if (blockMean.size()>=4&&blockVariance.size()>=4)
	{
		interpolation(src, dest, location, blockMean, blockVariance, xNums, yNums, xWidth, yHeight);
		dest.convertTo(ddest, CV_8UC1);
		return 1;
	}
	else
	{
		return -3;
	}

}

#endif // !MAD