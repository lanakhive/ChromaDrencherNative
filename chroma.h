#ifndef CHROMA_H
#define CHROMA_H

void blockAlloc(int newCount, int *oldCount, int itemSize, void **buffer);

#include <stdbool.h>
#include <stdlib.h>

typedef struct lineStruct {
	float x;
	float y;
	float sp;
	float alpha;
	bool tip;
} line;

extern int lineDensity;
extern int lineHeight;
extern int lineWidth;
extern int tipHeight;
extern int lineSpeed;
extern float coSpeed;
extern float coTime;
extern float coUp;
extern int lineCount;
extern int tipCount;

extern line *linelist;
extern int linelistCount;

//gradient colors
extern float acolor1[5][3];
extern float acolor2[5][3];
extern int alen1;
extern int alen2;

void shuffle(int *array, size_t length);

int randI(int low, int high);

void coolColors(bool top);

void updateLines(float dt, int screenWidth, int screenHeight);

void prepareLines(int screenWidth, int screenHeight);


#endif // CHROMA_H
