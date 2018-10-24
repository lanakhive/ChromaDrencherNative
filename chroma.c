#include "chroma.h"


int lineDensity = 100;
int lineHeight = 1024;
int lineWidth = 2;
int tipHeight = 64;
int lineSpeed = 10;
float coSpeed = 30;
float coTime = 0;
float coUp = 1;
int lineCount = 0;
int tipCount = 0;

line *linelist;
int linelistCount = 0;

//gradient colors
float acolor1[5][3] = {0};
float acolor2[5][3] = {0};
int alen1 = 0;
int alen2 = 0;

void shuffle(int *array, size_t length) {
	int i = length;
	int j;
	int temp;
	if (length == 0) return;
	while (--i) {
		j = rand() % (i+1);
		temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

int randI(int low, int high) {
	return (rand() % (high+1-low)) + low;
}

void coolColors(bool top) {
	const int ct[][3] = {
		{255,  40, 100},
		{255, 255,   0},
		{255, 150,   0},
		{150,   0, 255},
		{150, 255, 100},
		{255,   0, 200},
		{ 40, 100, 255}
	};
	int order[] = {0, 1, 2, 3, 4, 5, 6};
	shuffle(order, 7);
	int r = randI(3, 5);
	if (top) {
		alen1 = r;
		//SDL_Log("coolColors top: %d", r);
		for (int i = 0; i < r; i++) {
			acolor1[i][0] = ct[ order[i] ][0];
			acolor1[i][1] = ct[ order[i] ][1];
			acolor1[i][2] = ct[ order[i] ][2];
			//SDL_Log("%f %f %f", acolor1[i][0], acolor1[i][1], acolor1[i][2]);
		}
	} else {
		alen2 = r;
		//SDL_Log("coolColors bot: %d", r);
		for (int i = 0; i < r; i++) {
			acolor2[i][0] = ct[ order[i] ][0];
			acolor2[i][1] = ct[ order[i] ][1];
			acolor2[i][2] = ct[ order[i] ][2];
			//SDL_Log("%f %f %f", acolor2[i][0], acolor2[i][1], acolor2[i][2]);
		}
	}
}

void updateLines(float dt, int screenWidth, int screenHeight) {
	int cw = screenWidth;
	int ch = screenHeight;
	int length = lineCount;
	line *ele;
	for (int i=0; i<length; i++) {
		ele = &linelist[i];
		ele->y = ele->y + (ele->sp*lineSpeed) * dt;
		if (ele->y > ch) {
			ele->y = 0 - lineHeight;
		}
	}
	coTime += (coUp * coSpeed * dt);

	if (coTime > 255) {
		coTime = 255;
		coUp = -1;
		coolColors(true);
		//colorSpan(true);
	} else if (coTime < 0) {
		coTime = 0;
		coUp = 1;
		coolColors(false);
		//colorSpan(false);
	}
}

void prepareLines(int screenWidth, int screenHeight) {
	lineCount = lineDensity * screenWidth / 100;
	tipCount = 0;
	if (lineCount > linelistCount) {
		blockAlloc(lineCount, &linelistCount, sizeof(line), (void **)&linelist);
	}
	for (int i = 0; i<lineCount; i++) {
		line *ele = linelist+i;
		ele->x = randI(0, screenWidth);
		ele->y = randI(-lineHeight, screenHeight);
		ele->sp = randI(3, 8); 
		ele->tip = false;
		ele->alpha = randI(25, 100) / 255.0;
		if (randI(1, 10) == 9) {
			ele->tip = true;
			ele->alpha = 220 / 225.0;
		}
	}
	for (int i = 0; i<lineCount; i++) {
		line *ele = &linelist[i];
		if (ele->tip) tipCount++;
	}
}


