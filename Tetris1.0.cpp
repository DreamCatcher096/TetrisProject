#define _CRT_SECURE_NO_WARNINGS
#include <graphics.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	WIDTH	10		// 游戏区宽度
#define	HEIGHT	20		// 游戏区高度
#define	UNIT	20		// 每个游戏区单位的实际像素
#define PWIDTH  20
#define NEXT_X -120
#define NEXT_Y  65

int delay = 10;
int score = 0;

typedef struct {
	int x;
	int y;
	int color;
	int symmetry;
	int orientation;
	unsigned short *data;
} tetris_piece_s;
#define MAXSTAR 200	// 星星总数

struct STAR
{
	double x;
	int y;
	double step;
	int color;
};

STAR star[MAXSTAR];

typedef struct Person
{
	TCHAR   name[100] = { 0 };
	long   score;
}Person;

void InitStar(int i);               // 初始化星星
void MoveStar(Person per[]);
int Help();
int Setting();
int Rank(Person per[], char signName[], int v);
int GameOver(Person per[], char signName[], int v);
void WriteToFile(Person per[], int r);
int ReadFromFile(Person per[]);
void RankScore(Person per[], char signName[], int v);
int position_ok(tetris_piece_s piece, int *playfield)
{
	unsigned int mask = 1 << 15;
	for (int i = 0; i < 16; i++) {
		if ((piece.data[piece.orientation] & mask) != 0) {
			if ((piece.x + (PWIDTH * (i % 4))) < -1 || (piece.x + PWIDTH + (PWIDTH * (i % 4))) > 201)
				return 0;
			if ((piece.y + PWIDTH + (i / 4 * PWIDTH)) > 400 || *(playfield + (10 * ((piece.y + (i / 4 * PWIDTH)) / 20) + ((piece.x + (PWIDTH * (i % 4))) / 20))) != 0)
				return 0;
		}
		mask >>= 1;
	}
	return 1;
}

void draw_piece(tetris_piece_s piece, int visible)
{
	unsigned int mask = 1 << 15;
	for (int i = 0; i < 16; i++) {
		if ((piece.data[piece.orientation] & mask) != 0) {
			if (visible)
				setfillcolor(piece.color);
			else
				setfillcolor(BLACK);
			bar(piece.x + (PWIDTH * (i % 4)), piece.y + (i / 4 * PWIDTH), piece.x + PWIDTH + (PWIDTH * (i % 4)), piece.y + PWIDTH + (i / 4 * PWIDTH));
		}
		mask >>= 1;
	}
}

tetris_piece_s get_next_piece(int visible)
{
	static unsigned short square_data[] = { 1, 0xcc00 };
	static unsigned short line_data[] = { 2, 0x4444, 0xf000 };
	static unsigned short s_data[] = { 2, 0x8c40, 0x6c00 };
	static unsigned short z_data[] = { 2, 0x2640, 0xc600 };
	static unsigned short l_data[] = { 4, 0x4460, 0x7400, 0x6220, 0x2e00 };
	static unsigned short r_data[] = { 4, 0x2260, 0xe200, 0x6440, 0x4700 };
	static unsigned short t_data[] = { 4, 0xe400, 0x4640, 0x4e00, 0x4c40 };
	static unsigned short *piece_data[] = {
		square_data,
		line_data,
		s_data,
		z_data,
		l_data,
		r_data,
		t_data
	};
	static int piece_data_len = sizeof(piece_data) / sizeof(piece_data[0]);
	static int colors[] = { RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, 0xc7d0d5, 0xff7148, 0xe2da99, 0xff5a09, 0xf3843e };
	srand((unsigned int)clock());
	int next_piece_index = rand() % piece_data_len;
	unsigned short *next_piece_data = piece_data[next_piece_index];
	tetris_piece_s next_piece;

	next_piece.x = NEXT_X;
	next_piece.y = NEXT_Y;
	next_piece.color = colors[rand() % (sizeof(colors) / sizeof(colors[0]))];
	next_piece.data = next_piece_data + 1;
	next_piece.symmetry = *next_piece_data;
	next_piece.orientation = rand() % next_piece.symmetry;
	draw_piece(next_piece, visible);
	return next_piece;
}

tetris_piece_s get_current_piece(tetris_piece_s next_piece, int visible)
{
	tetris_piece_s current_piece = next_piece;

	draw_piece(next_piece, 0);
	current_piece.x = WIDTH * UNIT / 2 - PWIDTH;
	current_piece.y = 0;
	draw_piece(current_piece, visible);
	return current_piece;
}

int move(tetris_piece_s *piece, int *playfield, int dx, int dy, int dz)
{
	int new_position[] = { piece->x + dx, piece->y + dy, (piece->orientation + dz) % piece->symmetry };
	int x = piece->x, y = piece->y, z = piece->orientation;
	draw_piece(*piece, 0);
	piece->x = new_position[0];
	piece->y = new_position[1];
	piece->orientation = new_position[2];
	if (position_ok(*piece, playfield)) {
		draw_piece(*piece, 1);
		return 1;
	}
	else {
		piece->x = x;
		piece->y = y;
		piece->orientation = z;
		draw_piece(*piece, 1);
		return (dy == 0);
	}
}

void flatten_piece(tetris_piece_s piece, int *playfield)
{
	unsigned int mask = 1 << 15;
	for (int i = 0; i < 16; i++) {
		if ((piece.data[piece.orientation] & mask) != 0) {
			*(playfield + (10 * ((piece.y + (i / 4 * PWIDTH)) / 20) + ((piece.x + (PWIDTH * (i % 4))) / 20))) = piece.color;
		}
		mask >>= 1;
	}
}

void update_score(int complete_lines)
{
	static int lines_completed = 0;
	static int level = 1;
	//char name[100];
	lines_completed += complete_lines;
	score += (complete_lines * complete_lines);

	if (score > 20 * level) {
		delay -= 3;
		level++;
	}

}

void draw_playfield(int *playfield)
{
	setfillcolor(BLACK);

	bar(0, 0, WIDTH * UNIT, HEIGHT * UNIT);
	for (int i = 0; i < 200; i++) {
		if (playfield[i]) {
			setfillcolor(playfield[i]);
			bar(i % 10 * PWIDTH, i / 10 * PWIDTH, (i % 10 * PWIDTH + PWIDTH), (i / 10 * PWIDTH + PWIDTH));
		}
	}

}

int line_complete(int *line)
{
	for (int i = 0; i < 10; i++) {
		if (line[i] == 0)
			return 0;
	}
	return 1;
}

int process_complete_lines(int *playfield)
{
	int complete_lines = 0;
	int line[10] = { 0 };
	for (int i = 0; i < 200; i++) {
		line[i % 10] = playfield[i];
		if (i % 10 == 9) {
			if (line_complete(line)) {
				for (int j = i; j >= 10; j--) {
					playfield[j] = playfield[j - 10];
				}
				for (int c = 0; c < 10; c++)
					playfield[c] = 0;
				complete_lines++;
			}
		}
	}
	return complete_lines;
}

void process_fallen_piece(tetris_piece_s *piece, int *playfield)
{
	TCHAR name[12] = { 0 };



	int complete_lines = 0;
	flatten_piece(*piece, playfield);
	complete_lines = process_complete_lines(playfield);
	if (complete_lines > 0) {
		update_score(complete_lines);
		draw_playfield(playfield);


		setfillcolor(BLACK);
		bar(-150, 280, -50, 350);

		name[0] = score;    //可测试i的值;i每次比信息少一个字符; 
		setcolor(RED);
		settextstyle(40, 0, _T("微软雅黑"));
		sprintf((char*)name, "%d", name[0]); //方便查看每一个
		outtextxy(-110, 290, name);
	}

}

int down(tetris_piece_s *piece, int *playfield)
{
	if (move(piece, playfield, 0, PWIDTH, 0) == 1)
		return 1;
	process_fallen_piece(piece, playfield);
	return 0;
}

void drop(tetris_piece_s *piece, int *playfield)
{
	while (down(piece, playfield)) {}
}

void right(tetris_piece_s *piece, int *playfield)
{
	move(piece, playfield, PWIDTH, 0, 0);
}

void left(tetris_piece_s *piece, int *playfield)
{
	move(piece, playfield, -PWIDTH, 0, 0);
}

void rotate(tetris_piece_s *piece, int *playfield)
{
	move(piece, playfield, 0, 0, 1);
}

void fdown(tetris_piece_s *piece, int *playfield)
{
	move(piece, playfield, 0, PWIDTH, 0);
}

int gameover(int *playfield)
{
	for (int i = 0; i < 10; i++) {
		if (playfield[i])
			return 1;
	}

	return 0;
}

int game_main()
{
	int playfield[200] = { 0 };
	char c = 0;
	int now;
	TCHAR name[100] = { 0 };
	tetris_piece_s next_piece;
	tetris_piece_s current_piece;

	initgraph(640, 480);

	setbkmode(TRANSPARENT);			// 设置图案填充的背景色为透明
									// 设置坐标原点
	setorigin(220, 20);

	// 绘制游戏区边界
	rectangle(-1, -1, WIDTH * UNIT + 1, HEIGHT * UNIT + 1);
	rectangle(-2, -2, WIDTH * UNIT + 2, HEIGHT * UNIT + 2);
	rectangle(-3, -3, WIDTH * UNIT + 3, HEIGHT * UNIT + 3);
	//rectangle(-145, 50, WIDTH * UNIT -230, HEIGHT * UNIT - 230);
	//rectangle(-146, 49, WIDTH * UNIT -229, HEIGHT * UNIT - 229);
	//rectangle(-147,48, WIDTH * UNIT -228, HEIGHT * UNIT - 228);
	setcolor(YELLOW);
	settextstyle(30, 0, _T("微软雅黑"));
	outtextxy(-145, 20, _T("NEXT"));
	setcolor(YELLOW);
	settextstyle(30, 0, _T("微软雅黑"));
	outtextxy(-130, 250, _T("SCORE"));
	settextstyle(30, 0, _T("微软雅黑"));
	outtextxy(220, 50, _T("a/← : left"));
	outtextxy(220, 80, _T("d/→ : right"));
	outtextxy(220, 110, _T("w/↑ : rotate"));
	outtextxy(220, 140, _T("d/↓ : speed up"));
	outtextxy(220, 170, _T("space : drop"));
	outtextxy(220, 200, _T("q : quit"));
	name[0] = 0;    //可测试i的值;i每次比信息少一个字符; 
	setcolor(RED);
	settextstyle(40, 0, _T("微软雅黑"));
	sprintf((char*)name, "%d", name[0]);
	outtextxy(-110, 290, name);

	next_piece = get_next_piece(1);
	current_piece = get_current_piece(next_piece, 1);
	//for (int i = 0; i < 89900000; i++);
	next_piece = get_next_piece(1);
	while (!(next_piece.data[next_piece.orientation] != current_piece.data[current_piece.orientation] && next_piece.color != current_piece.color)) {
		draw_piece(next_piece, 0);
		next_piece = get_next_piece(1);
	}
	now = clock() / 100;
	while (c != 'q')
	{
		// 获取按键

		// 根据输入，计算新的坐标
		if (_kbhit()) {
			c = _getch();
			if (c == -32) {
				c = _getch();
			}
			switch (c)
			{
			case 75:
			case 'a':
				left(&current_piece, playfield);
				break;
			case 77:
			case 'd':
				right(&current_piece, playfield);
				break;
			case 72:
			case 'w':
				rotate(&current_piece, playfield);
				break;
			case 80:
			case 's':
				fdown(&current_piece, playfield);
				break;
			case ' ':
				drop(&current_piece, playfield);
				current_piece = get_current_piece(next_piece, 1);
				next_piece = get_next_piece(1);
				if (next_piece.color == current_piece.color) {
					draw_piece(next_piece, 0);
					next_piece = get_next_piece(1);
				}
				continue;
				break;
			case 27: break;
			}
		}
		if ((clock() / 100) - now >= delay) {
			now = clock() / 100;
			if (!down(&current_piece, playfield)) {
				current_piece = get_current_piece(next_piece, 1);
				next_piece = get_next_piece(1);
				if (next_piece.color == current_piece.color) {
					draw_piece(next_piece, 0);
					next_piece = get_next_piece(1);
				}
			}
		}
		if (gameover(playfield)) {
			break;
		}
		Sleep(20);
	}
	/*char name[100];
	name[0]=score;    //可测试i的值;i每次比信息少一个字符;
	cleardevice();
	setcolor(RED);
	settextstyle(40, 0, _T("微软雅黑"));
	sprintf(name,"%d", name[0]); //方便查看每一个
	outtextxy(150, 200, name);
	_getch();*/
	return 0;
}

// 主函数
void main()
{
	Person per[300];
	srand((unsigned)time(NULL));           // 随机种子
	initgraph(640, 480);	               // 打开图形窗口
	cleardevice();
	MoveStar(per);
}

// 初始化星星
void InitStar(int i)
{
	star[i].x = 0;
	star[i].y = rand() % 480;
	star[i].step = (rand() % 5000) / 1000.0 + 1;
	star[i].color = (int)(star[i].step * 255 / 6.0 + 0.5);	// 速度越快，颜色越亮
	star[i].color = RGB(star[i].color, star[i].color, star[i].color);
}
// 移动星星
void MoveStar(Person per[])
{
	int a = -1, num = 220;
	int paint = 0;
	int v = -1;
	// 初始化所有星星
	char signName[100];
	for (int i = 0; i<MAXSTAR; i++)
	{
		InitStar(i);
		star[i].x = rand() % 640;
	}
	while (a != 0)
	{
		for (int i = 0; i<MAXSTAR; i++)
		{
			if (paint == 1 || paint == 0)
			{
				setcolor(RED);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(260, 220, _T("S t a r t"));
				setcolor(YELLOW);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(265, 265, _T("H e l p"));
				outtextxy(265, 310, _T("R a n k"));
				outtextxy(240, 355, _T("S e t t i n g"));
			}
			if (paint == 2)
			{
				setcolor(YELLOW);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(260, 220, _T("S t a r t"));
				setcolor(RED);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(265, 265, _T("H e l p"));
				setcolor(YELLOW);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(265, 310, _T("R a n k"));
				outtextxy(240, 355, _T("S e t t i n g"));
			}
			if (paint == 3)
			{
				setcolor(YELLOW);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(260, 220, _T("S t a r t"));
				outtextxy(265, 265, _T("H e l p"));
				setcolor(RED);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(265, 310, _T("R a n k"));
				setcolor(YELLOW);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(240, 355, _T("S e t t i n g"));
			}
			if (paint == 4)
			{
				setcolor(YELLOW);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(260, 220, _T("S t a r t"));
				outtextxy(265, 265, _T("H e l p"));
				outtextxy(265, 310, _T("R a n k"));
				setcolor(RED);
				settextstyle(40, 0, _T("微软雅黑"));
				outtextxy(240, 355, _T("S e t t i n g"));
			}

			// 擦掉原来的星星
			putpixel((int)star[i].x, star[i].y, 0);
			putpixel((int)star[i].x + 1, star[i].y, 0);
			putpixel((int)star[i].x, star[i].y + 1, 0);
			putpixel((int)star[i].x + 1, star[i].y + 1, 0);

			// 计算新位置
			star[i].x += star[i].step + 6;  //可调整速度
			if (star[i].x > 640)	InitStar(i);

			// 画新星星
			putpixel((int)star[i].x, star[i].y, star[i].color);
			putpixel((int)star[i].x + 1, star[i].y, star[i].color);
			putpixel((int)star[i].x, star[i].y + 1, star[i].color);
			putpixel((int)star[i].x + 1, star[i].y + 1, star[i].color);

			//标题
			setcolor(YELLOW);
			settextstyle(64, 0, _T("微软雅黑"));
			outtextxy(220, 110, _T("T e t r i s"));

			if (_kbhit())
			{
				char b;
				b = _getch();
				if (b == -32) {
					b = _getch();
				}
				switch (b)
				{
					case 72 :
					case 'w': 
						num = num - 45;
						break;
					case 80 :
					case 's': 
						num = num + 45;
						break;
					case 10 :
					case 13 :
					case 'j': 
						if (num <= 255)
						{
							game_main();
							a = GameOver(per, signName, v);
							cleardevice();
						}
						if (num == 265)
						{
							a = Help();
							cleardevice();
						}
						if (num == 310)
						{
							a = Rank(per, signName, v);
							cleardevice();
						}
						if (num >= 355)
						{
							a = Setting();
							cleardevice();
						}
				}
				if (num <= 225)
				{
					if (num == 175)
					{
						num = num + 45;
					}
					setcolor(RED);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(260, 220, _T("S t a r t"));
					setcolor(YELLOW);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(265, 265, _T("H e l p"));
					outtextxy(265, 310, _T("R a n k"));
					outtextxy(240, 355, _T("S e t t i n g"));
					paint = 1;
				}
				if (num == 265)
				{
					setcolor(YELLOW);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(260, 220, _T("S t a r t"));
					setcolor(RED);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(265, 265, _T("H e l p"));
					setcolor(YELLOW);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(265, 310, _T("R a n k"));
					outtextxy(240, 355, _T("S e t t i n g"));
					paint = 2;
				}
				if (num == 310)
				{
					setcolor(YELLOW);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(260, 220, _T("S t a r t"));
					outtextxy(265, 265, _T("H e l p"));
					setcolor(RED);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(265, 310, _T("R a n k"));
					setcolor(YELLOW);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(240, 355, _T("S e t t i n g"));
					paint = 3;
				}
				if (num >= 355)
				{
					if (num>355)
					{
						num = num - 45;
					}
					setcolor(YELLOW);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(260, 220, _T("S t a r t"));
					outtextxy(265, 265, _T("H e l p"));
					outtextxy(265, 310, _T("R a n k"));
					setcolor(RED);
					settextstyle(40, 0, _T("微软雅黑"));
					outtextxy(240, 355, _T("S e t t i n g"));
					paint = 4;
				}
			}
		}
		Sleep(20);
	}
}

//帮助界面
int Help()
{

	cleardevice();
	setcolor(LIGHTGRAY);
	settextstyle(25, 0, _T("黑体"));
	outtextxy(185, 80, _T("字母"));
	outtextxy(320, 80, _T("功能"));
	outtextxy(200, 140, _T("w"));
	outtextxy(290, 140, _T("变形/上移"));
	outtextxy(200, 180, _T("s"));
	outtextxy(320, 180, _T("下移"));
	outtextxy(200, 220, _T("a"));
	outtextxy(320, 220, _T("左移"));
	outtextxy(200, 260, _T("d"));
	outtextxy(320, 260, _T("右移"));
	outtextxy(200, 300, _T("j"));
	outtextxy(320, 300, _T("确定"));
	outtextxy(400, 360, _T("按任意键返回.."));
	if (_kbhit())
	{
		return 1;
	}
	_getch();
}

//设置界面
int Setting()
{
	int speed;
	char c = 0;
	int x = 270, y = 165, z = 280, q = 175;
	cleardevice();
	setcolor(YELLOW);
	settextstyle(40, 0, _T("微软雅黑"));
	outtextxy(130, 100, _T("speed:"));
	outtextxy(300, 150, _T("初级"));
	outtextxy(300, 200, _T("中级"));
	outtextxy(300, 250, _T("高级"));
	setfillcolor(RGB(0xFF, 0x0, 0x80));

	//画一实心矩形，
	bar(x, y, z, q);
	while (c != 27)
	{
		// 获取按键
		c = _getch();

		// 先擦掉上次显示的旧图形

		setfillcolor(BLACK);
		bar(x, y, z, q);

		// 根据输入，计算新的坐标

		switch (c)
		{
		case 'w': x -= 0;  y -= 50; z -= 0;  q -= 50; break;
		case 's': x += 0;  y += 50; z += 0;  q += 50; break;
		case 'j': if (y>150 && y<200)
		{
			delay = 10;
			return 1;         //返回初级速度，可改2即可；
		}
				  if (y>200 && y<250)
				  {
					  delay = 4;
					  return 1;        //返回中级速度，可改3即可；
				  }
				  if (y>250)
				  {
					  delay = 1;
					  return 1;       //返回高级速度，可改4即可；
				  }
		}
		if (y<150)
		{
			y = y + 50;
			q = q + 50;
		}
		if (q>280)
		{
			y = y - 50;
			q = q - 50;
		}
		// 绘制新的图形
		setfillcolor(RGB(0xFF, 0x0, 0x80));
		//画一实心矩形，
		bar(x, y, z, q);
	}
	_getch();
}

//排行榜函数
int Rank(Person per[], char signName[], int v)
{
	char s[100];

	//stu[0].name[0]='a';

	cleardevice();
	setcolor(YELLOW);
	settextstyle(35, 0, _T("微软雅黑"));
	outtextxy(90, 45, _T("ID"));
	outtextxy(210, 45, _T("SCORE"));
	outtextxy(420, 45, _T("Name"));

	//sprintf(s,"%c", per[0].name[0]);
	//outtextxy(150, 90, s);
	RankScore(per, signName, v);
	if (_kbhit())
	{
		return 1;
	}

	_getch();
}

//结束界面
int GameOver(Person per[], char signName[], int v)
{
	TCHAR s[100] = { 0 };
	int num = 1;
	int x;
	TCHAR name[100] = { 0 };
	//char numer[100];
	int r;
	per[0].score = score;          /*只需传入分数*/

								   //cleardevice();
	closegraph();
	initgraph(640, 480);
	setcolor(YELLOW);
	settextstyle(35, 0, _T("微软雅黑"));
	outtextxy(250, 80, _T("游戏结束"));
	outtextxy(180, 150, _T("分 数 :"));
	outtextxy(50, 220, _T("请输入你的名字:"));
	sprintf((char*)s, "%d", per[0].score); //方便查看每一个
	outtextxy(280, 150, s); //可输出全部内容	
	setlinecolor(WHITE);
	line(244, 260, 254, 260);
	line(244, 261, 254, 261);
	line(244, 262, 254, 262);
	while (_kbhit() == 0)
	{
		setlinecolor(BLACK);
		line(244, 260, 254, 260);
		line(244, 261, 254, 261);
		line(244, 262, 254, 262);
		Sleep(5);
		setlinecolor(WHITE);
		line(244, 260, 254, 260);
		line(244, 261, 254, 261);
		line(244, 262, 254, 262);
	}
	char c;
	int i = 0;
	while (num != 0)
	{
		setcolor(YELLOW);
		settextstyle(35, 0, _T("微软雅黑"));
		outtextxy(250, 80, _T("游戏结束"));
		outtextxy(180, 150, _T("分 数 :"));
		sprintf((char*)s, "%d", per[0].score); //方便查看每一个
		outtextxy(280, 150, s); //可输出全部内容	
		outtextxy(50, 220, _T("请输入你的名字:"));
		if (_kbhit())
		{
			c = _getch();
			name[i] = c;
			name[i + 1] = 0;
			//sprintf(name,"%c", name[i]);
			cleardevice();
			outtextxy(245, 220, name);

			if (c == 8)
			{
				while (i != x - 2)
				{
					cleardevice();
					name[i] = 0;
					outtextxy(245, 220, name);
					i--;
				}
			}
			if (c == 13)
			{
				num = 0;
			}
			i = i + 1;
			x = i;
		}
	}
	cleardevice();

	//numer[0]=i;    //可测试i的值;i每次比信息少一个字符;
	//sprintf(name,"%d", name[0]); //方便查看每一个
	//outtextxy(150, 200, name); //可输出全部内容	
	for (r = 0; r<i - 1; r++)
	{
		per[0].name[r] = name[r];
	}
	per[0].name[r + 1] = 0;
	//sprintf(s,"%c", per[0].name[2]); //方便查看每一个
	//outtextxy(150, 200, s); //可输出全部内容
	WriteToFile(per, r);
	for (v = 0; v<i - 1; v++)
	{
		signName[v] = name[v];
	}
	signName[v] = 0;
	RankScore(per, signName, v);

	_getch();

	return 1;
}

//把玩家信息写入文件;
void WriteToFile(Person per[], int r)
{
	FILE *fp;
	int i, j;
	if ((fp = fopen("PlayInformation.txt", "a+")) == NULL)
	{
		printf("failure to open studentinformation.txt!\n");
		exit(0);
	}
	for (i = 0; i<1; i++)
	{
		fprintf(fp, "\n%6d", per[i].score);
		for (j = 0; j<r; j++)
		{
			fprintf(fp, "%c", per[i].name[j]);
		}
	}
	fclose(fp);
}

//从文件中读出信息
int ReadFromFile(Person per[])
{
	FILE *fp;
	int n, j;
	if ((fp = fopen("PlayInformation.txt", "r")) == NULL)
	{
		printf("failure to open PlayInformation.txt!\n");
		exit(0);
	}
	for (n = 0; !feof(fp); n++)
	{
		fscanf(fp, "\n%d", &per[n].score);
		fscanf(fp, "%s", &per[n].name);
	}

	fclose(fp);
	return n;
}

//对读出的数据进行排序
void RankScore(Person per[], char signName[], int v)
{
	struct Person temp;
	TCHAR s[100] = { 0 };
	TCHAR ls[100] = { 0 };
	int n;
	int i, j, k, l, z = 0;
	int a;
	n = ReadFromFile(per);
	/*s[1]=n;
	sprintf(s,"%d", s[1]);   //测试是否把数据从文件中读入
	outtextxy(100, 100,s);
	*/
	for (i = 0; i<n - 1; i++)
	{
		k = i;
		for (j = i + 1; j<n; j++)
		{
			if (per[j].score>per[k].score)
			{
				k = j;
			}
		}
		if (k != i)
		{
			temp = per[k];
			per[k] = per[i];
			per[i] = temp;
		}
	}
	//outtextxy(390, 83+z,per[0].name);
	for (l = 0; l<10; l++)
	{
		a = 0;
		for (int b = 0; b<v; b++)
		{
			if (per[l].name[b] == signName[b])
			{
				a++;
			}
		}
		if (a == v)
		{
			setcolor(RED);
		}
		else
		{
			setcolor(YELLOW);
		}
		ls[1] = l;
		sprintf((char*)s, "%d", per[l].score);
		sprintf((char*)ls, "%d", ls[1] + 1);
		settextstyle(30, 0, _T("微软雅黑"));
		outtextxy(95, 83 + z, ls);
		outtextxy(200, 83 + z, s);
		outtextxy(390, 83 + z, per[l].name);
		z = z + 35;
	}
}
