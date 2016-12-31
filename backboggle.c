#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#define BENCHITTER 100000

#define BOARDX 4
#define BOARDY 4

#define MAXWORD 16 //should be boardx * boardy or some shit


#define TRUE 1
#define FALSE 0


#define ISLOWER(char) ( ((char) >='a' && (char) <= 'z') )
#define ISUPPER(char) ( ((char) >='A' && (char) <= 'Z') )


int numwords = 0;
typedef struct wordtrie_s {
	struct wordtrie_s * nexts[26];
	uint8_t isend;
//	uint8_t isfound;
	int lastfound;
} wordtrie_t;



char * dice[16] = {
#include "dice.h"
};

char board[BOARDX][BOARDY];
char boardsign[BOARDX][BOARDY] = {0};

void printboard(void){
	int x,y;
	for(x = 0; x < BOARDX; x++){
		for(y = 0; y < BOARDY; y++)
			putc(board[x][y]+'a', stdout);
		putc('\n', stdout);
	}
}



void genrandomboard(void){
	int x, y;
//	for(x = 0; x < BOARDX; x++)
//	for(y = 0; y < BOARDY; y++)
//		boardsign[x][y] = 0;
	//basically fisher yates shuffle of dice
	int dicearray[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int i;
	for(i = 15; i > 0; i--){
		int arrpos = rand()%i;
		int tmp = dicearray[arrpos];
		dicearray[arrpos] = dicearray[i];
		dicearray[i] = tmp;
	}
//	printf("dicearray is {");
//	for(i = 0; i < 16; i++) printf("%i %s", dicearray[i], i < 15 ? "," : "}\n");
//now go through and randomly chooser
	for(x = 0; x < BOARDX; x++){
//		int rndval = rand();
		for(y = 0; y < BOARDY; y++){
			char * die = dice[dicearray[x*BOARDY + y]];
			board[x][y] = die[rand()%6]-'a';
//			board[x][y] = die[rndval%6]-'a';
//			rndval/=6;
		}
	}

}




wordtrie_t root = {0};


int curfind = 0;
int numfound = 0;
/*
int resettrie(wordtrie_t * cur){
	int cnt = 0;
	if(cur->isfound) cnt++;
	cur->isfound = 0;
	int i;
	for(i = 0; i < 26; i++) if(cur->nexts[i])cnt+=resettrie(cur->nexts[i]);
	return cnt;
}
*/

int addtotrie(const char * word, const size_t wordlen, wordtrie_t * const r){
	int i, num_created = 0;
	wordtrie_t * cur = r;
	for(i = 0; i < wordlen; i++){
		char curc = word[i];
		if(ISLOWER(curc)) curc -= 'a';
		else if(ISUPPER(curc)) curc -= 'A';
		else {/*printf("invalid word %s %c %i\n", word, curc, curc); */return -1;}
		if(curc == 'q'-'a' && (word[i+1] == 'u' || word[i+1] == 'U')) i++;

		if(cur->nexts[curc]){
			cur = cur->nexts[curc];
		} else {
			cur->nexts[curc] = malloc(sizeof(wordtrie_t));
			cur = cur->nexts[curc];
			memset(cur, 0, sizeof(wordtrie_t));
			num_created++;
		}
	}
	cur->isend = TRUE;
	return num_created;
}


int jdepth = 0;
char jeremy[MAXWORD] = {0};

int search(int x, int y, wordtrie_t *cur){
	if(boardsign[x][y]) return 0;
	char dil = board[x][y];
	wordtrie_t *next =cur->nexts[dil];
	if(!next) return 0;
	boardsign[x][y] = 1;

	jeremy[jdepth] = dil+'a';
	jdepth++;
	if(dil == 'q'){
		jeremy[jdepth] = dil+'u';
		jdepth++;
	}
	if(next->isend){
		#ifndef BENCHMARK
//		if(!next->isfound)printf("%s\n", jeremy);
		#endif
		//next->isfound = 1;
		if(next->lastfound != curfind){
			next->lastfound = curfind;
			numfound++;
		}
	}
	if(y > 0){
		if(x > 0) search(x-1, y-1, next);
		search(x, y-1, next);
		if(x < BOARDX-1) search(x+1, y-1, next);
	}
	if(x > 0) search(x-1, y, next);
	if(x < BOARDX-1) search(x+1, y, next);
	if(y < BOARDY-1){
		if(x > 0) search(x-1, y+1, next);
		search(x, y+1, next);
		if(x < BOARDX-1) search(x+1, y+1, next);
	}
	boardsign[x][y] = 0;
	jeremy[jdepth] = 0;
	jdepth--;
	return 1;
}

int loadwords(const char * filename){
	FILE *f = fopen(filename, "r");
	if(!f){
		printf("unable to load %s\n", filename);
		return -1;
	}
	char *line = 0;
	ssize_t len = 0;
	ssize_t read = 0;
	while ((read = getline(&line, &len, f)) != -1) {
//		printf("%s", line);
		numwords++;
		addtotrie(line, read-1, &root);
	}
	if(line) free(line);
	fclose(f);
	return numwords;
}



int main(const int argc, const char ** argv){
	if(argc < 2){
		printf("need more args\n");
		return 1;
	}
	srand(time(0));
//	srand(1337);
	loadwords(argv[1]);


#ifdef BENCHMARK
	int k;
	struct timespec tstart = {0}, tend = {0};
	double time;
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	for(k = 0; k < BENCHITTER; k++){
		genrandomboard();
	}
	clock_gettime(CLOCK_MONOTONIC, &tend);
	time = ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec);
	printf("time to do randomize %.8f\n", time);


	clock_gettime(CLOCK_MONOTONIC, &tstart);
	for(k = 0; k < BENCHITTER; k++){
		int x, y;
		for(x = 0; x < BOARDX; x++)
		for(y = 0; y < BOARDY; y++)
			search(x, y, &root);
	}
	clock_gettime(CLOCK_MONOTONIC, &tend);
	time = ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec);
	printf("time to do search %.8f\n", time);
/*
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	for(k = 0; k < BENCHITTER; k++){
		resettrie(&root);
	}
	clock_gettime(CLOCK_MONOTONIC, &tend);
	time = ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec);
	printf("time to do reset %.8f\n", time);
*/
#else
struct timespec tstart = {0}, tend = {0};
clock_gettime(CLOCK_MONOTONIC, &tstart);



int mxwords = 0;
int cnt;
for(cnt = 0;TRUE; cnt++){
	curfind = cnt+1;
	genrandomboard();
	int x, y;
	for(x = 0; x < BOARDX; x++)
	for(y = 0; y < BOARDY; y++)
		search(x, y, &root);
//	int result = resettrie(&root);
	int result = numfound;
	numfound = 0;
	if(result > mxwords){
		mxwords = result;
		printf("after %i tries found %i words out of %i (~%.4f\%)\n", cnt, result, numwords, 100.0* ((double)result)/((double)numwords));
		printboard();
	}
	if(!(cnt %100000)){
		clock_gettime(CLOCK_MONOTONIC, &tend);
		double time = ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec);
		printf("%i tries in %.8f seconds, or  %.8f tries per second\n", cnt, time, (double)cnt/time);
	}
}
#endif

	return 0;
}
