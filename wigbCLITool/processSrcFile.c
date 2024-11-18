//
//  processSrcFile.c
//  wigbCLITool
//
//  Created by Larry on 11/18/24.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "igbTool.h"

int processSrcFile(int initpage, int isp, int isc, int *wout, char *olines[], char *defAuthor, FILE *fpin, FILE *fpinfo) {
    char linein[BUFSIZE], cpy[BUFSIZE];
    int  outlines = 0, inlines = 0;
    int mlen, errorCnt = 0;
    char *eptr, *cp;
    PARSE_RES pres;
    int numpages, curpage = initpage;
    
    while (NULL != fgets(linein, sizeof(linein)-1, fpin)) {
        if (inlines == MAXLINES) {
            printf("WARNING: Lines read exceeds iGigBook upload limit of %d\nNo more input processed.\n\n", MAXLINES);
            fprintf(fpinfo, "WARNING: Lines read exceeds iGigBook upload limit of %d\nNo more input processed.\n\n", MAXLINES);
            break;
        }
        inlines++;
        strcpy(cpy, linein);
        
        // Parse the current line
        pres = parseLine(cpy, isp, isc, fpinfo);
        if (pres.num_errors > 0) {
            errorCnt += pres.num_errors;
            if (pres.error == E_EMPTY) {
                // Empty line. Just remove it.
                printf("Line %d empty. Removed\n", inlines);
                fprintf(fpinfo, "Line %d empty. Removed\n", inlines);
            }
            else {
                // Fatal error. Continue to process but don't create output.
                *wout = 0;
            }
            continue;
        }
        else if (pres.spaces > 0) {
            // Report that spaces were removed but the line otherwise OK.
            char *pl  = (pres.spaces == 1) ? "space" : "spaces";
            int tlc   = (int)strlen(linein)-1;
            char *FMT = (linein[tlc] == '\n') ? "%s%s\n" : "%s\n%s\n";
            
            errorCnt++;
            if (errorCnt < MAXPERROR) {
                printf("%d %s removed from line %d\n", pres.spaces, pl, inlines);
                printf(FMT, linein, cpy);
            }
            fprintf(fpinfo, "%d %s removed from line %d\n", pres.spaces, pl, inlines);
            fprintf(fpinfo, FMT, linein, cpy);
        }
        
        // Line is now in a known state. Continue processing.
        mlen = (int)strlen(cpy)+1;
        
        // Skip title
        cp = strchr((cpy+1), '"') + 1; // ptr at the ',' past the second '"'
        // Stomp on comma at ptr. Title now a printable string
        *cp++ = '\0';
        
        // we're at the start of the page number field. Skip it by finding the comma seperator
        if (!isp) {
            cp = strchr(cp, ',');
            cp++;   //skip comma
        }
        
        // We're at the start of the number of pages field. get it.
        if ((numpages=(int)strtol(cp,&eptr,10)) > 0) {
            if ((*eptr != ',') && (isc && (*eptr != '\n'))) {
                if (errorCnt++ < MAXPERROR) printf("Line %d: bad number-of-pages field\n%s\n", inlines, linein);
                fprintf(fpinfo, "Line %d: bad number-of-pages field\n%s\n", inlines, linein);
                *wout = 0;
                continue;
            }
            if (*wout) {
                // We think we're outputting so prepare storage for the well formed line.
                olines[outlines] = calloc(BUFSIZE,1);
                
                // Populate the output line.
                if (!isc) {
                    // skip the number of pages field to retrieve composer
                    cp = strchr(cp,',') + 1;
                    sprintf(olines[outlines],"%s,%d,%d,%s", cpy, curpage, numpages, cp);
                } else {
                    // We're using the user-provided Composer string.
                    sprintf(olines[outlines],"%s,%d,%d,\"%s\"\n", cpy, curpage, numpages, defAuthor);
                }
                
                // return unneeded memory
                olines[outlines] = realloc(olines[outlines], strlen(olines[outlines])+1);
                curpage += numpages;
                outlines++;
            }
        }
        else {
            // Bad number of pages. Another fatal error. Don't write output.
            if (errorCnt++ < MAXPERROR) printf("\n\nLine %d: bad number-of-pages field\n%s\n", inlines, linein);
            fprintf(fpinfo, "\n\nLine %d: bad number-of-pages field\n%s\n", inlines, linein);
            *wout = 0;
            continue;
        }
    }
    
    return 0;
}
