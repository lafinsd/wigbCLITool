#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <libgen.h>    // for basename()
#include <time.h>

#include "igbTool.h"
#include "gopt.h"

#define NUMARGS    1

#define OPUPLD     "BlkUpld"
#define OPINFO     "info"
#define ETYPE      "(b2.1)"
#define DUMMY_NAME "\"ZZSong"

static int   makeofname(int, char *, char *, char **, char **);
static void  printBanner(char *, char **);
static char *getRunTime (void);
static int   prntOutput(int, PRF_OUTP, FINFO);

int main(int argc, char **argv)
{
    int xtralines   = 0;
    PRF_INP pinp    = {0};
    PRF_OUTP prfout = {0};
    GOPT opts       = {0};
    FINFO fin, fout, finfo;
    char *banner;
    
    opts.argc = argc;
    opts.argv = argv;
    opts.pinp = &pinp;
    
    pinp.initpage = INITCP;
    
    printBanner(argv[0], &banner);
    
    // Process any options
    if (procopt(&opts)) {
        exit (1);
    }
   
    // populate composer string if one wasn't provided
    if (!pinp.isc) {
        pinp.defAuthor = calloc((strlen(DEFAUTHOR)+1),1);
        strcpy(pinp.defAuthor, DEFAUTHOR);
    }
    
    fin.fname = argv[optind];
    
    // Create output file name
    {
        int res = makeofname(argc - optind, argv[optind+1], fin.fname, &fout.fname, &finfo.fname);
        
        if (res) {
            if (res == 1) {
                printf(USAGE_FMT, basename(argv[0]));
            }
            exit (1);
        }
    }
    
    if ((fin.fp = fopen(fin.fname, "r")) == NULL) {
        printf("%s: file  not found\n", fin.fname);
        exit(1);
    }

    finfo.fp = fopen(finfo.fname, "w+");
    fout.fp = fopen(fout.fname, "w+");
    
    fprintf(finfo.fp,"\n%s\n", banner); // banner in info file
    
    // If invoked from Automator print out invocation line in info file.
    if (opts.isauto) {
        int i;
        
        fprintf (finfo.fp, "\n%s", basename(argv[0]));
        for (i=1; i<argc; ++i) {
            // Don't print out the hidden '-Automator' argument. 'isauto' is the index in argv
            if (i != opts.isauto) {
                // Surround entries having blanks with "
                char *cp = strchr(argv[i], ' ');
                if (cp == NULL) {
                    fprintf (finfo.fp, " %s", argv[i]);
                }
                else {
                    fprintf (finfo.fp, " \"%s\"", argv[i]);
                }
            }
        }
    }
    
    printf("\nOutput file name is \"%s\" (\"%s\")\n\n", basename(fout.fname), fout.fname);
    fprintf(finfo.fp, "\n\nOutput file name is \"%s\" (\"%s\")\n\n", basename(fout.fname), fout.fname);
    
    pinp.fin   = fin;
    pinp.finfo = finfo;
    
    // Process each line in the input file
    prfout = processSrcFile(&pinp);
    
    // Done with all the input.
    if (prfout.errorCnt > MAXPERROR) printf("%d more errors/warnings...\nSee %s in output directory\n\n", (prfout.errorCnt-MAXPERROR), basename(finfo.fname));
    
    if (fout.fp == NULL) {
        printf("%s: cannot open\n", fout.fname);
        fprintf(finfo.fp, "%s: cannot open\n", fout.fname);
        prfout.outlines = 0;
    }
    else {
        if (prfout.wout) {
            xtralines = prntOutput(opts.isd, prfout, fout);
        }
        else {
            fprintf(fout.fp, "\nNo lines written.\nSee %s for information\n", basename(finfo.fname));
            prfout.outlines = 0;
        }
    }
    
    xtralines = ((xtralines > 0) && opts.isd) ? xtralines : 0;

    printf("\n%d lines read\n%d lines written\n", prfout.inlines, prfout.outlines + xtralines);
    fprintf(finfo.fp, "\n%d lines read\n%d lines written\n", prfout.inlines, prfout.outlines + xtralines);
    
    // We could be below the minumum if the -d option was not specified.
    if (prfout.outlines < MINLINES) {
        if (opts.isd) {
            printf("WARNING: %d dummy entries written to output to comply with iGigBook minimum of %d\n\n", xtralines, MINLINES);
            fprintf(finfo.fp, "WARNING: %d dummy entries written to output to comply with iGigBook minimum of %d\n\n", xtralines, MINLINES);
        }
        else {
            printf("WARNING: Lines written out does not meet minimum iGigBook Bulk Upload minimum of %d.\nUse the -d option\n\n", MINLINES);
            fprintf(finfo.fp, "WARNING: Lines written out does not meet minimum iGigBook Bulk Upload minimum of %d.\nUse the -d option.\n\n", MINLINES);
        }
    }

    fclose (fin.fp);
    fclose (finfo.fp);
    fclose (fout.fp);
    
    exit(0);
}

static void printBanner(char *myName, char **banner) {
#define __MONTH__ (\
    __DATE__[2] == 'n' ? (__DATE__[1] == 'a' ? "1" : "6") \
    : __DATE__[2] == 'b' ? "2" \
    : __DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? "3" : "4") \
    : __DATE__[2] == 'y' ? "5" \
    : __DATE__[2] == 'l' ? "7" \
    : __DATE__[2] == 'g' ? "8" \
    : __DATE__[2] == 'p' ? "9" \
    : __DATE__[2] == 't' ? "10" \
    : __DATE__[2] == 'v' ? "11" \
    : "12")
    int   idx;
    char  dts[50];
    char *rt, *d = malloc(sizeof(__DATE__));
    
    *banner = malloc(BUFSIZE);
    rt = getRunTime();
    
    strcpy(d,__DATE__);
    d[6] = '\0';
    idx  = (d[4] != ' ') ? 4 : 5;
    
    sprintf(dts, "%s-%s-%sT%s",&d[7], __MONTH__, &d[idx], __TIME__);
    sprintf(*banner, "%s%s (%s  %s)", basename(myName), ETYPE, dts, rt);
    printf("%s\n", *banner);
    free(d);
    free(rt);
    return;
}

static int makeofname (int diff, char *np, char *fin, char **fout, char **finfo) {

    if (diff != NUMARGS)  {
        if (diff == (NUMARGS+1)) {
            // Optional output file name is there. Get it
            int  len = (int)strlen(dirname(fin)) + (int)strlen(np) + 1;  // room for '/'
            
            *fout = calloc(len,1);
            sprintf(*fout, "%s/%s", dirname(fin),np);
            
            *finfo = calloc(((int)strlen(fin) + (int)(strlen(OPINFO) + (int)strlen(np) + 2)),1);
            sprintf(*finfo, "%s/%s_%s", dirname(fin), OPINFO, np);
        }
        else {
            return (1);
        }
    }
    else {
        // Optional output file name not there. Use default output file names.
        *fout  = malloc(strlen(fin) + (strlen(OPUPLD) + 4));  // room for '/', '_', '99' if needed
        *finfo = malloc(strlen(fin) + (strlen(OPINFO) + 4));
        sprintf(*finfo, "%s/%s_%s", dirname(fin), OPINFO, basename(fin));
        sprintf(*fout, "%s/%s_%s", dirname(fin), OPUPLD, basename(fin));
        
#if DO_NOT_OVERWRITE==1
        char *cp    = *fout + strlen(dirname(*fout))+sizeof(OPUPLD);
        int   count = (int)strtol(cp,NULL,10);
        
        while ((access(*fout, F_OK)) != -1) {
            if (++count == 100) {
                printf("Count for duplicate (%d) output files excessive. Run terminated\n", count);
                return 2;
            }
            sprintf(*fout, "%s/%s%d_%s", dirname(fin), OPUPLD, count, basename(fin));
            sprintf(*finfo, "%s/%s%d_%s", dirname(fin), OPINFO, count, basename(fin));
        }
#endif
    }
    
    return 0;
}

static char *getRunTime (void) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char *rt = malloc(BUFSIZE);
    
    sprintf(rt, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    return rt;
}

static int prntOutput(int isd, PRF_OUTP prfout, FINFO fout) {
    
    int i, xtralines;
    
    // OK to write the output.
    for (i=0; i<prfout.outlines; ++i) {
        // Bulk Upload will not tolerate an empty line. Kill '\n' at end of last entry (if it's there)
        if (i == (prfout.outlines-1)) {
            char *cp = strchr(prfout.olines[i],'\n');
            if (cp != NULL) {
                *cp = '\0';
            }
        }
        fprintf(fout.fp, "%s", prfout.olines[i]);
    }
    
    // Add the extra lines if we're below the minimum and the option was there.
    xtralines = MINLINES - prfout.outlines;
    if ((xtralines > 0) && isd)  {
        int i;
        
        if (prfout.outlines) fprintf(fout.fp,"\n");        // Current last line is missing '\n'
        for (i=0; i<(xtralines-1); ++i) {
            fprintf(fout.fp, "%s%d\",999,1,\"[Author]\"\n", DUMMY_NAME,i);
        }
        fprintf(fout.fp, "%s%d\",999,1,\"[Author]\"", DUMMY_NAME,i);  // no '\n' on the last line
    }
    
    return xtralines;
}
