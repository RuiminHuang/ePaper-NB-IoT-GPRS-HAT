#include <stdio.h>
#include <string.h>

#include "iniParse.h"

static char *skipwhite(char *string){
	for (;;){
		if (*string == 0){
			return NULL;
		}
		if (*string == ';'){
			return NULL;
		}
		if (*string == '#'){
			return NULL;
		}
		if (*string != ' ' && *string != '\t' && *string != '\r' && *string != '\n'){
			return string;
		}
		string++;
	}
}


static const char *afterequal(const char *string){
	const char *spot = string;
	for (;;){
		if (*spot == '=' || *spot == ':'){
			for (;;){
				spot++;
				if (0 == *spot){
					return NULL;
				}
				else if (*spot != ' ' && *spot != '\t' && *spot != '\r' && *spot != '\n'){
					return spot;
				}
				else{
					continue;
				}
			}
		}
		else if (*spot == 0){
			return NULL;
		}
		else{
			spot++;
			continue;
		}
	}
}


char *inifindTag(FILE *fp, const char *tag){
	static char line[256];
	int tagLen = strlen(tag);
	char * findContent;
	char *endFindContent;
	char *skip;

	rewind(fp);

	while( !feof(fp) ){
		memset(line, 0, 256); // add by min 2015/1/16
		if (fgets(line, 256, fp) == NULL){
			printf("gets line error \n");
			return NULL;
		}
 		//printf("fgets: %s\n",line);
		skip = skipwhite(line);
		if (skip != NULL){
			if (strncmp(skip, tag, tagLen) == 0){
				findContent = (char *)afterequal(line);
				if (findContent == NULL){
					return NULL;
				}
				endFindContent = findContent + strlen(findContent)-1;
				while(*endFindContent == ' ' || *endFindContent == '\t'
				|| *endFindContent == '\r' || *endFindContent == '\n'){
					//endFindContent equal 0 mean end of the file
					*endFindContent=0;
					endFindContent--;
				}
				return findContent;
			}
		}
	}
	return NULL;
}