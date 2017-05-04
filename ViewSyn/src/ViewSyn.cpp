//#include <string.h>
//#include <stdio.h>

/*This is my VSRS!!*/

#include <iostream>
#include <iomanip>
#include "opencv_cfg.h"

#include <time.h>
#include <stdlib.h>
//#include <opencv2/highgui/highgui.hpp>
//#include <iostream>


#include "version.h"
#include "yuv.h"
#include "ParameterViewInterpolation.h"
#include "ViewInterpolation.h"


using namespace std;
using namespace cv;

#ifndef WIN32
#define BYTE unsigned char
#endif

int main(int argc , char *argv[]) 
{
  unsigned int n;

  CParameterViewInterpolation  cParameter;

  CViewInterpolation cViewInterpolation;
  CIYuv yuvBuffer;

#ifdef OUTPUT_COMPUTATIONAL_TIME
  clock_t start, finish, first;
  first = start = clock();
#endif

  printf("View Synthesis Reference Software (VSRS), Version %.1f\n", VERSION);
  printf("     - MPEG 3DV, August 2009\n\n");
  
  if ( cParameter.Init( argc, argv ) != 1 ) return 0;//读取配置文件参数,保存到缓存;

  if(!cViewInterpolation.Init(cParameter)) return 10;//将缓存中的配置文件参数具体设置;

  if(!yuvBuffer.Resize(cParameter.getSourceHeight(), cParameter.getSourceWidth(), 420)) return 2;//设置每一帧YUV图像空间大小;

  FILE *fin_view_r, *fin_view_l, *fin_depth_r, *fin_depth_l, *fout;

  if( (fin_view_l  = fopen(cParameter.getLeftViewImageName() .c_str(), "rb"))==NULL ||
      (fin_view_r  = fopen(cParameter.getRightViewImageName().c_str(), "rb"))==NULL ||
      (fin_depth_l = fopen(cParameter.getLeftDepthMapName()  .c_str(), "rb"))==NULL ||
      (fin_depth_r = fopen(cParameter.getRightDepthMapName() .c_str(), "rb"))==NULL ||
      (fout = fopen(cParameter.getOutputVirViewImageName()   .c_str(), "wb"))==NULL )
  {
    fprintf(stderr, "Can't open input file(s)\n");
    return 3;
  }//将左右视点的纹理+深度图像 以及 输出文件指针定义;
 
#ifdef OUTPUT_COMPUTATIONAL_TIME
  finish = clock();
  printf( "Initialization: %.4f sec\n", (double)(finish - start) / CLOCKS_PER_SEC);
  start = finish;
#endif

  for(n = cParameter.getStartFrame(); n < cParameter.getStartFrame() + cParameter.getNumberOfFrames(); n++) //具体算法实现;
  {
    printf("frame number = %d ", n);

    if( !cViewInterpolation.getDepthBufferLeft() ->ReadOneFrame(fin_depth_l, n) || 
        !cViewInterpolation.getDepthBufferRight()->ReadOneFrame(fin_depth_r, n)  ) break;//分别获得左右视点参考深度图;
    printf(".");

    cViewInterpolation.setFrameNumber( n - cParameter.getStartFrame()); // Zhejiang //标记当前是实际处理的第几帧;

    if(!yuvBuffer.ReadOneFrame(fin_view_l, n)) break;//把左视点纹理图像读入yuvBuffer中;
    if(!cViewInterpolation.SetReferenceImage(1, &yuvBuffer)) break;//根据输入时RGB还是YUV格式来把yuvBuffer中的数据拷到m_pcTempYuvLeft和m_pcTempYuvRight中;
    printf(".");

    if(!yuvBuffer.ReadOneFrame(fin_view_r, n)) break;//同上;
    if(!cViewInterpolation.SetReferenceImage(0, &yuvBuffer)) break;//同上;
    printf(".");

    if(!cViewInterpolation.DoViewInterpolation( &yuvBuffer )) break;//此处是真正的具体算法了,我理解此处的yuvBuffer是传数据出来的作用;
    printf("."); 
    
    if(!yuvBuffer.WriteOneFrame(fout)) break;

#ifdef OUTPUT_COMPUTATIONAL_TIME
    finish = clock();
    printf("->End (%.4f sec)\n", (double)(finish - start) / CLOCKS_PER_SEC);
    start = finish;
#else
    printf("->End\n");
#endif

  } // for n

  fclose(fout);
  fclose(fin_view_l);
  fclose(fin_view_r);
  fclose(fin_depth_l);
  fclose(fin_depth_r);
 
#ifdef OUTPUT_COMPUTATIONAL_TIME
  finish = clock();
  printf("Total: %.4f sec\n", ((double)(finish-first))/((double)CLOCKS_PER_SEC));
#endif
  waitKey();
  return 0;
}

