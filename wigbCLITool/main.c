#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <libgen.h>    // for basename()
#include <time.h>

#include "igbTool.h"

#define NUMARGS    1
#define INITCP     1

#define OPUPLD     "BlkUpld"
#define OPINFO     "info"
#define ETYPE      "(b1.1)"
#define DUMMY_NAME "\"ZZSong"

#define USAGE_FMT  "Usage: %s [-d] [-p] [-c<\"Composer\">] [-f<offset>] <infile> [outfile]\n"
#define OPT_STRING "A::f:dpc:"
#define OPT_HIDDEN "utomator"

static void  makeofname(char *, char *, char *);
static void  printBanner(char *, char **);
static char *getRunTime (void);

int main(int argc, char **argv)
{
    char *cp, *banner;
    int isd = 0, isauto = 0, xtralines = 0;
    FINFO fin, fout, finfo;
    PRF_INP pinp    = {0};
    PRF_OUTP prfout = {0};
    
    pinp.initpage = INITCP;
    
    printBanner(argv[0], &banner);
    
    {
        int offset, gopt;
        
        opterr = 0;   // suppress error message from getopt()
        while ((gopt=getopt(argc, argv, OPT_STRING)) != -1) {
            errno = 0;
            switch (gopt) {
                case 'A':
                    /* This option occurs only when invoked from Automator and causes the
                     * invocation line to appear in the info file. Guard against accidental
                     * confusion if a bogus option interferes.
                     */
                    isauto = optind - 1;  // Set 'isauto' to index in argv of the option
                    if (optarg == 0) {
                        printf("Illegal option \"%s\"\n", argv[isauto]);
                        printf(USAGE_FMT, basename(argv[0]));
                        exit(1);
                    }
                    else {
                        if (strcmp(OPT_HIDDEN, optarg) != 0) {
                            printf("Illegal option \"%s\"\n", argv[isauto]);
                            printf(USAGE_FMT, basename(argv[0]));
                            exit(1);
                        }
                    }
                    break;
                
                case 'f':
                    // Offset provided
                    errno  = 0;
                    offset = (int)strtol(optarg,NULL,10);
                    if (offset > 0) {
                        if (pinp.initpage != INITCP) {
                            printf("Multiple non-zero offsets specified\n");
                            exit (-1);
                        }
                        pinp.initpage += offset;
                    }
                    else if (offset == 0) {
                        if (errno != 0) {
                            printf("Illegal or missing offset. The -f option requires an offset value.\n");
                            printf(USAGE_FMT, basename(argv[0]));
                            exit(-1);
                        }
                    }
                    else {
                        printf("Illegal or missing offset. The -f option requires an offset value.\n");
                        printf(USAGE_FMT, basename(argv[0]));
                        exit(-1);
                    }
                    break;
                
                case 'd':
                    // Add dummy entries if needed
                    isd = 1;
                    break;
                
                case 'p':
                    // No dummy page numbers in source file
                    pinp.isp = 1;
                    break;
                
                case 'c':
                    // Composer name provided
                    pinp.defAuthor = calloc((strlen(optarg)+1),1);
                    strcpy(pinp.defAuthor, optarg);
                    pinp.isc = 1;
                    break;
                
                case '?':
                default:
                    printf("Illegal option \"%s\"\n", argv[optind-1]);
                    printf(USAGE_FMT, basename(argv[0]));
                    exit(1);
            }
        }
    }
    
    // populate composer string if one wasn't provided
    if (!pinp.isc) {
        pinp.defAuthor = calloc((strlen(DEFAUTHOR)+1),1);
        strcpy(pinp.defAuthor, DEFAUTHOR);
    }
    
    // See if optional output file name is there and if so save it.
    {
        int diff = argc - optind;
        
        cp = NULL;
        if (diff != NUMARGS)  {
            if (diff == (NUMARGS+1)) {
                cp = argv[optind+1];
            }
            else {
                printf(USAGE_FMT, basename(argv[0]));
                exit(1);
            }
        }
    }
    
    fin.fname = argv[optind];
    
    if ((fin.fp = fopen(fin.fname, "r")) == NULL) {
        printf("%s: file  not found\n", fin.fname);
        exit(1);
    }
    
    if (cp == NULL) {
#if DO_NOT_OVERWRITE==1
        // room for '9999', '_', and '/'
        int xtra = 6;
#else
        // room for '_', and '/'
        int xtra = 2;
#endif
        // Optional output file name not there. Use default output file names.
        fout.fname  = malloc(strlen(fin.fname) + (strlen(OPUPLD) + xtra));
        finfo.fname = malloc(strlen(fin.fname) + (strlen(OPINFO) + xtra));
        makeofname(fin.fname, fout.fname, finfo.fname);
    }
    else {
        // Optional output file name is there
        int len = (int)strlen(dirname(fin.fname)) + (int)strlen(cp) + 1;  // room for '/'
        
        fout.fname = calloc(len,1);
        sprintf(fout.fname, "%s/%s", dirname(fin.fname), cp);
        
        finfo.fname = calloc(((int)strlen(fin.fname) + (int)(strlen(OPINFO) + (int)strlen(cp) + 2)),1);
        sprintf(finfo.fname, "%s/%s_%s", dirname(fin.fname), OPINFO, cp);
    }
    
    finfo.fp = fopen(finfo.fname, "w+");
    fout.fp = fopen(fout.fname, "w+");
    
    fprintf(finfo.fp,"\n%s\n", banner); // banner in info file
    
    // If invoked from Automator print out invocation line in info file.
    if (isauto) {
        
        int i;
        
        fprintf (finfo.fp, "\n%s", basename(argv[0]));
        for (i=1; i<argc; ++i) {
            // Don't print out the hidden '-Automator' argument. 'isauto' is the index in argv
            if (i != isauto) {
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
    fprintf(finfo.fp, "\nOutput file name is \"%s\" (\"%s\")\n\n", basename(fout.fname), fout.fname);
    
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
            int i;
            
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
        }
        else {
            fprintf(fout.fp, "\nNo lines written.\nSee %s for information\n", basename(finfo.fname));
            prfout.outlines = 0;
        }
    }
    
    xtralines = ((xtralines > 0) && isd) ? xtralines : 0;

    printf("\n%d lines read\n%d lines written\n", prfout.inlines, prfout.outlines + xtralines);
    fprintf(finfo.fp, "\n%d lines read\n%d lines written\n", prfout.inlines, prfout.outlines + xtralines);
    
    // We could be below the minumum if the -d option was not specified.
    if (prfout.outlines < MINLINES) {
        if (isd) {
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

static void makeofname (char *fin, char *fout, char *finfo) {
    
    sprintf(finfo, "%s/%s_%s", dirname(fin), OPINFO, basename(fin));
    sprintf(fout, "%s/%s_%s", dirname(fin), OPUPLD, basename(fin));
    
// We may eventually eliminate the overwrite protection.
#if DO_NOT_OVERWRITE==1
    
    char *cp    = fout + strlen(dirname(fout))+sizeof(OPUPLD);
    long  count = strtol(cp,NULL,10);
    
    while ((access(fout, F_OK)) != -1) {
        ++count;
        sprintf(fout, "%s/%s%ld_%s", dirname(fin), OPUPLD, count, basename(fin));
        sprintf(finfo, "%s/%s%ld_%s", dirname(fin), OPINFO, count, basename(fin));
    }
#endif
    return;
}

static char *getRunTime (void) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char *rt = malloc(BUFSIZE);
    
    sprintf(rt, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    return rt;
}
