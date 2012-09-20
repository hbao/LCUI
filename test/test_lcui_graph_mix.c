//����LCUI��ͼ�����ٶ�

#include <LCUI_Build.h>
#include LC_LCUI_H
#include LC_MISC_H
#include LC_GRAPHICS_H
#include LC_MEM_H

int main()
{
	clock_t time, t;
	int i, w, h;
	LCUI_Graph bg, fg;
	
	Graph_Init(&bg);
	Graph_Init(&fg);
	bg.have_alpha = IS_FALSE;
	fg.have_alpha = IS_TRUE; 
	w = 1640;
	h = 1480;
	//Ϊǰ��ͼ�ͱ���ͼ�����ڴ�
	Malloc_Graph(&bg, w, h);
	Malloc_Graph(&fg, w, h);
	printf("graph size: %dx%d\n", w, h);
	time = clock(); //��ʼ��ʱ
	for(i=0; i<=255; ++i){
		t = clock(); 
		nobuff_print("[%2d] mix graph, alpha = %3d, use time: ", i, i);
		Fill_Graph_Alpha(&fg, i);
		Mix_Graph(&bg, &fg, Pos(0,0));
		printf("%ldms\n", clock()-t);//������ʱ��������
	} 
	printf("total time: %ldms\n", clock()-time);
	return 0;
}
