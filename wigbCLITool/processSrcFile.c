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

PRF_OUTP processSrcFile(PRF_INP *pinp) {
    char linein[BUFSIZE], cpy[BUFSIZE];
    int mlen;
    char *eptr, *cp;
    int numpages, curpage = pinp->initpage;
    PARSE_RES pres;
    PRF_OUTP  prf = {0};
    
    prf.wout = 1;
    
    while (NULL != fgets(linein, sizeof(linein)-1, pinp->fin.fp)) {
        if (prf.inlines == MAXLINES) {
            printf("WARNING: Lines read exceeds iGigBook upload limit of %d\nNo more input processed.\n\n", MAXLINES);
            fprintf(pinp->finfo.fp, "WARNING: Lines read exceeds iGigBook upload limit of %d\nNo more input processed.\n\n", MAXLINES);
            break;
        }
        prf.inlines++;
        strcpy(cpy, linein);
        
        // Parse the current line
        pres = parseLine(cpy, pinp->isp, pinp->isc, pinp->finfo.fp);
        if (pres.num_errors > 0) {
            prf.errorCnt += pres.num_errors;
            if (pres.error == E_EMPTY) {
                // Empty line. Just remove it.
                printf("Line %d empty. Removed\n", prf.inlines);
                fprintf(pinp->finfo.fp, "Line %d empty. Removed\n", prf.inlines);
            }
            else {
                // Fatal error. Continue to process but don't create output.
                prf.wout = 0;
            }
            continue;
        }
        else if (pres.spaces > 0) {
            // Report that spaces were removed but the line otherwise OK.
            char *pl  = (pres.spaces == 1) ? "space" : "spaces";
            int tlc   = (int)strlen(linein)-1;
            char *FMT = (linein[tlc] == '\n') ? "%s%s\n" : "%s\n%s\n";
            
            prf.errorCnt++;
            if (prf.errorCnt < MAXPERROR) {
                printf("%d %s removed from line %d\n", pres.spaces, pl, prf.inlines);
                printf(FMT, linein, cpy);
            }
            fprintf(pinp->finfo.fp, "%d %s removed from line %d\n", pres.spaces, pl, prf.inlines);
            fprintf(pinp->finfo.fp, FMT, linein, cpy);
        }
        
        // Line is now in a known state. Continue processing.
        mlen = (int)strlen(cpy)+1;
        
        // Skip title
        cp = strchr((cpy+1), '"') + 1; // ptr at the ',' past the second '"'
        // Stomp on comma at ptr. Title now a printable string
        *cp++ = '\0';
        
        // we're at the start of the page number field. Skip it by finding the comma seperator
        if (!pinp->isp) {
            cp = strchr(cp, ',');
            cp++;   //skip comma
        }
        
        // We're at the start of the number of pages field. get it.
        if ((numpages=(int)strtol(cp,&eptr,10)) > 0) {
            if ((*eptr != ',') && (pinp->isc && (*eptr != '\n'))) {
                if (prf.errorCnt++ < MAXPERROR) printf("Line %d: bad number-of-pages field\n%s\n", prf.inlines, linein);
                fprintf(pinp->finfo.fp, "Line %d: bad number-of-pages field\n%s\n", prf.inlines, linein);
                prf.wout = 0;
                continue;
            }
            if (prf.wout) {
                // We think we're outputting so prepare storage for the well formed line.
                prf.olines[prf.outlines] = calloc(BUFSIZE,1);
                
                // Populate the output line.
                if (!pinp->isc) {
                    // skip the number of pages field to retrieve composer
                    cp = strchr(cp,',') + 1;
                    sprintf(prf.olines[prf.outlines],"%s,%d,%d,%s", cpy, curpage, numpages, cp);
                } else {
                    // We're using the user-provided Composer string.
                    sprintf(prf.olines[prf.outlines],"%s,%d,%d,\"%s\"\n", cpy, curpage, numpages, pinp->defAuthor);
                }
                
                // return unneeded memory
                prf.olines[prf.outlines] = realloc(prf.olines[prf.outlines], strlen(prf.olines[prf.outlines])+1);
                curpage += numpages;
                prf.outlines++;
            }
        }
        else {
            // Bad number of pages. Another fatal error. Don't write output.
            if (prf.errorCnt++ < MAXPERROR) printf("\n\nLine %d: bad number-of-pages field\n%s\n", prf.inlines, linein);
            fprintf(pinp->finfo.fp, "\n\nLine %d: bad number-of-pages field\n%s\n", prf.inlines, linein);
            prf.wout = 0;
            continue;
        }
    }
    
    return prf;
}
