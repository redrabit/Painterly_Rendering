#include<opencv2/opencv.hpp>

float ComputeD(CvScalar a, CvScalar b)// 원본이미지와 블러가 적용된 이미지간의 색상 오차율을 구하기 위한 함수
{
	float d = (a.val[0] - b.val[0])*(a.val[0] - b.val[0])
		+ (a.val[1] - b.val[1])*(a.val[1] - b.val[1])
		+ (a.val[2] - b.val[2])*(a.val[2] - b.val[2]);

	d = sqrt(d);
	return d;
}

void *Paintlayer(IplImage *src, IplImage *dst, int r, int sizeblur)  //실제적으로 캔버스 위에 무수한 원을찍을 함수
{
	CvSize size = cvGetSize(src);

	IplImage *blur = cvCreateImage(size, 8, 3);
	cvSmooth(src, blur, CV_GAUSSIAN, sizeblur);  //블러가 적용된 이미지, 이때 블러값은 파라미터로 입력받는다.

	int w = size.width;
	int h = size.height;
	int divx = w / r;							//캔버스위엔 무수한 원을 찍게되는데, 이때 원의 중심은 이미지가 n*n개로 쪼개어진 격자안에 들어가게된다. 
	int divy = w / r;							//이때 나눠지는 값은 같은 값을 주게 되는데, 그렇지 않으면, 정사각형이 아닌 이미지가 들어갔을때, 가로와 세로 각각 n개씩 만들지 못한다.
	int dw = w / divx;
	int dh = h / divy;
	CvPoint *Rpoint = new CvPoint[divx*divy];		//격자안에 찍을 원의 중심들

	int remind_v = 0, remind_u = 0;					//격자안에서 원본이미지와 블러가 적용된이미지간의 색상오차가 가장많이 나는 좌표값을 저장할 변수 
	CvScalar g;
	for (int i = 0; i<divx; i++)
		for (int j = 0; j<divy; j++)
		{
			float *d = new float[dw*dh];			//한 격자내부에서의 픽셀들의 오차값을 저장할 배열,배열의 개수는 격자내부의 픽셀들의 개수

			float avr;							//한 격자의 평균 색상 오차값을 저장할 변수
			int k = 0;						    //한 격자내부에서 모든 픽셀값들의 오차값을 저장할때, 배열의 인덱스를 늘려주기 위한 변수
			float sum = 0, max = 0;
			for (int u = 0; u<dw; u++)
				for (int v = 0; v<dh; v++)
				{
					int x = dw*i + u;				//이미지 전체의 x,y좌표를 복원한 값
					int y = dh*j + v;

					CvScalar gridc1, gridc2;		//원본이미지와 블러가 적용된 이미지간의 색상오차를 구할때, 각각의 색상정보값을 읽어올 CvScalar변수
					gridc1 = cvGet2D(blur, y, x);

					gridc2 = cvGet2D(src, y, x);
					if (v == dw - 1)					//한 격자 내부의 픽셀이 격자의 가로길이까지 도달한다면,
						k++;					//그 다음줄의 픽셀의 계속하여 인덱스 번호를 부여한다.
					d[u*k + v] = ComputeD(gridc1, gridc2);

					sum = sum + d[u*k + v];			//격자 내부의 평균 오차율을 구하기위해 모든 픽셀의 오차값의 합을 저장
					if (d[u*k + v]>8)				//블러가 적용된 이미지와 원본이미지의 차이가 많이 나지 않으면, 원을찍지 않는다.
						if (d[u*k + v]>max)		//색상오차가 가장 많이 나는곳에 좌표위에 원을 찍음
						{
							max = d[u*k + v];
							remind_u = x;
							remind_v = y;
							Rpoint[divx*i + j] = cvPoint(x, y); //색상오차가 가장 많이 나는곳에 좌표를 기억
						}
					if (u == dw - 1 && v == dh - 1) //만약 한 격자의 모든픽셀을 조회했다면,

						avr = sum / (u*v);		//평균오차를 구하고
					g = cvGet2D(blur, remind_v, remind_u); //최대오차가 나는곳의 색상정보를 얻어낸다.

				}
			delete[]d;
			cvCircle(dst, Rpoint[divx*i + j], r, g, -1);
		}
	delete[]Rpoint;
	return dst;						//지금 까지 찍은 캔버스의 원들을 반환
}

void main()
{
	printf("=================================================\n");
	printf("Dept. of Digital Contents, Sejong University\n");
	printf("Multimedia Programming Class\n");
	printf("Homework #6: Painterly Rendering\n");
	printf("=================================================\n");

	while (1)
	{
		printf("Input file path : ");
		char input;
		int mode;
		scanf("%s", &input);
		IplImage *src = cvLoadImage(&input);
		if ((cvLoadImage(&input)) == NULL)
		{
			printf("Wrong File Path! Try again!\n");
			continue;
		}


		printf("Select Drawing Mode (0=circle)");
		scanf("%d", &mode);

		if (mode == 0)
		{


			CvSize size = cvGetSize(src);

			IplImage *dst = cvCreateImage(size, 8, 3);

			cvSet(dst, cvScalar(255, 255, 255));		//캔버스는 하얀도화지로 시작하여

			Paintlayer(src, dst, 30, 51);				//차례로 큰원->작은원 순으로 그림을 그린다.이때 붓이 클수록 더 많은 블러효과를 적용한다.
			Paintlayer(src, dst, 15, 31);
			Paintlayer(src, dst, 8, 21);
			Paintlayer(src, dst, 5, 11);
			Paintlayer(src, dst, 2, 3);

			cvShowImage("src", src);
			cvShowImage("dst", dst);
			cvWaitKey();
			cvReleaseImage(&src);
			cvReleaseImage(&dst);
			return;



		}
		else
		{
			printf("Wrong Drawing Choice!\n");
			continue;
		}
	}
}
