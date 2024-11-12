//
//  parseLine.c
//  wigbCLITool
//
//  Created by Larry on 11/9/24.
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "igbTool.h"

#define tCOMMA ','
#define tQUOTE '\"'

static int isblank(char *);

PARSE_RES parseLine(char *line, int isp, int isc, FILE *fpinfo) {
    char tgtf[]  = {tQUOTE, tQUOTE, tCOMMA, tCOMMA, tCOMMA, tQUOTE, tQUOTE};    // full line
    char tgtp[]  = {tQUOTE, tQUOTE, tCOMMA, tCOMMA, tQUOTE, tQUOTE};            // page field missing
    char tgta[]  = {tQUOTE, tQUOTE, tCOMMA, tCOMMA};                            // author field missing
    char tgtpa[] = {tQUOTE, tQUOTE, tCOMMA};                                    // page and author fields missing
    
    char *tgt;
    char *cp = line;
    int  tsz, i=0, j=0, len=(int)strlen(line);
    char *rstr, *tstr = malloc(len);
    PARSE_RES res = {0,0,E_NONE};
    
    if (isblank(line)) {
        res.error = E_EMPTY;
        res.num_errors++;
        return res;
    }
    // depending on options set parse target
    if (isp && isc) {
        tgt = tgtpa;
        tsz = sizeof(tgtpa);
    } else if (isp) {
        tgt = tgtp;
        tsz = sizeof(tgtp);
    } else if (isc) {
        tgt = tgta;
        tsz = sizeof(tgta);
    } else {
        tgt = tgtf;
        tsz = sizeof(tgtf);
    }
    
    while ((i < len) && (j < tsz)){
        if ((j == (tsz-2)) && (*cp == ',') && (!isc)) {
            res.error = E_TOKEN;
            if (res.num_errors++ < MAXPERROR) printf("Unexpected ',' in this line\n%s\n", line);
            fprintf(fpinfo, "Unexpected ',' in this line\n%s\n", line);
            free(tstr);
            return res;
        }
        
        if (*cp++ == tgt[j]) {
            ++j;
        }
        ++i;
    }
    
    if (i < (len-1)) {
        // make sure there aren't any more tokens
        while (i < len) {
            if ((*cp == tCOMMA) || (*cp == tQUOTE)) {
                res.error = E_TOKEN;
                if (res.num_errors++ < MAXPERROR) printf("There is an extra token in this line\n%s\n", line);
                fprintf(fpinfo, "Extra token(s) in this line\n%s\n", line);
                free(tstr);
                return res;
            }
            ++cp;
            ++i;
        }
    }
    
    if (j != tsz) {
        if (!isc && (tgt[j] == tCOMMA)) {
            // Line is missing the author without -c option. Use the default by inference.
            strcpy ((cp-1), ",");       // overwrite the \n
            strcat (line, "\"");
            strcat (line, DEFAUTHOR);
            strcat (line, "\"\n");
            len  = (int)strlen(line);    // reset len based on added text
            tstr = realloc(tstr,len);
        }
        else {
            res.error = E_TOKEN;
            if (res.num_errors++ < MAXPERROR) printf("Token(s) missing in this line\n%s\n", line);
            fprintf(fpinfo, "Token(s) missing in this line\n%s\n", line);
            free(tstr);
            return res;
        }
        
    }
    
    cp = line;
    rstr = tstr;
    while (*cp == ' ') {        //skip initial spaces
        res.spaces++;
        cp++;
    }
    *rstr++ = *cp++;            //record the "
    while (*cp != '"') {        //find the next " copying the contents until then
        *rstr++ = *cp++;
    }
    *rstr++ = *cp++;            //record the "
    
    if (!isc) {
        while (*cp != '"') {        //look for next "
            if (*cp != ' ') {       //copy character if it isn't a space
                *rstr++ = *cp++;
            }
            else {
                res.spaces++;
                cp++;
            }
        }
    }
    
    {
        int sofar = (int)(cp - line);
        strncpy(rstr, cp, (len-sofar));
        strncpy(line,tstr,len);
        
        len   = (int)strlen(line);
        int i = line[len-1] == '\n' ? 2 : 1;
        
        // remove trailing blanks
        while(line[len-i] == ' ') {
            res.spaces++;
            line[len-i] = '\n';
            ++i;
        }
        line[len-i+2] = '\0';
        
    }
    
    free(tstr);
    return res;
}

static int isblank(char *line) {
    int i;
    
    for (i=0; i<strlen(line); ++i) {
        if (line[i] == ' '  || line[i] == '\n') {
            continue;
        }
        return 0;
    }
    return 1;
}
