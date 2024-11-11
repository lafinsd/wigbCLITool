#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <libgen.h>    // for basename()

#include "igbTool.h"

#define NUMARGS    1
#define INITCP     1

#define OPUPLD     "BlkUpld"
#define OPINFO     "info"
#define ETYPE      "(b2)"
#define DUMMY_NAME "\"ZZSong"

#define USAGE_FMT  "Usage: %s [-d] [-p] [-c<\"Composer Name\">] [-f<offset>] <infilename>\n"
#define OPT_STRING "A::f:dpc:"
#define OPT_HIDDEN "utomator"

static char *skipTitle(char *);
static void  makeofname(char *, char *, char *);
static void  printBanner(char *);

int   gBoguscount = 0;

extern PARSE_RES parseLine(char *, int, int, FILE *);

int main(int argc, char **argv)
{
    FILE *fpin, *fpout, *fpinfo;
    char *fin, *fout, *finfo, *cp, linein[BUFSIZE], cpy[BUFSIZE];
    int   numpages;
    int   wout = 1, inlines = 0, outlines = 0;
    int   curpage = INITCP;
    int   isp = 0, isc = 0, isd = 0, isauto = 0, xtralines = 0;
    char *defAuthor = NULL;
    char *olines[MAXLINES];
    
    printBanner(argv[0]);
    
    {
        int offset, gopt;
        
        opterr = 0;   // suppress error message from getopt()
        while ((gopt=getopt(argc, argv, OPT_STRING)) != -1) {
            errno = 0;
            switch (gopt) {
                case 'A':
                    isauto = optind - 1;
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
                    errno  = 0;
                    offset = (int)strtol(optarg,NULL,10);
                    if (offset > 0) {
                        if (curpage != INITCP) {
                            printf("Multiple non-zero offsets specified\n");
                            exit (-1);
                        }
                        curpage += offset;
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
                    isd = 1;
                    break;
                
                case 'p':
                    isp = 1;
                    break;
                
                case 'c':
                    defAuthor = calloc((strlen(optarg)+1),1);
                    strcpy(defAuthor, optarg);
                    isc = 1;
                    break;
                
                case '?':
                default:
                    printf("Illegal option \"%s\"\n", argv[optind-1]);
                    printf(USAGE_FMT, basename(argv[0]));
                    exit(1);
            }
        }
    }
    
    if (!isc) {
        defAuthor = calloc((strlen(DEFAUTHOR)+1),1);
        strcpy(defAuthor, DEFAUTHOR);
    }
    
    
    
    if ((argc-optind) != NUMARGS)  {
        printf(USAGE_FMT, basename(argv[0]));
        exit(1);
    }
    
    fin = argv[optind];
    
    if ((fpin = fopen(fin, "r")) == NULL) {
        printf("%s: file  not found\n", fin);
        exit(1);
    }
    
    fout  = malloc(strlen(fin) + (strlen(OPUPLD) + 4));      // room for prefix plus numerals up to '9999'
    finfo = malloc(strlen(fin) + (strlen(OPINFO) + 4));      // room for prefix plus numerals up to '9999'
    makeofname(fin, fout, finfo);
    
    fpinfo = fopen(finfo, "w+");
    
    if (isauto) {
        int i;
        
        fprintf (fpinfo, "\n%s", basename(argv[0]));
        for (i=1; i<argc; ++i) {
            if (i != isauto) {
                fprintf (fpinfo, " %s", argv[i]);
            }
        }
        fprintf(fpinfo,"\n\n");
    }
    
    printf("Output file name is \"%s\" (%s)\n\n", basename(fout), fout);
    fprintf(fpinfo, "Output file name is \"%s\" (%s)\n\n", basename(fout), fout);
    
    while (NULL != fgets(linein, sizeof(linein)-1, fpin)) {
        int mlen;
        char *eptr;
        PARSE_RES pres;
        
        if (inlines == MAXLINES) {
            printf("WARNING: Lines read exceeds iGigBook upload limit of %d\nNo more input processed.\n\n", MAXLINES);
            fprintf(fpinfo, "WARNING: Lines read exceeds iGigBook upload limit of %d\nNo more input processed.\n\n", MAXLINES);
            break;
        }
        inlines++;
        strcpy(cpy, linein);
        
        pres = parseLine(cpy, isp, isc, fpinfo);
        if (pres.errors > 0) {
            wout = 0;
            continue;
        }
        else if (pres.spaces > 0) {
            char *pl  = (pres.spaces == 1) ? "space" : "spaces";
            int tlc   = (int)strlen(linein)-1;
            char *FMT = (linein[tlc] == '\n') ? "%s%s\n" : "%s\n%s\n";
            
            gBoguscount += pres.spaces;
            if (gBoguscount < MAXPERROR) {
                printf("%d %s removed from line %d\n", pres.spaces, pl, inlines);
                printf(FMT, linein, cpy);
            }
            fprintf(fpinfo, "%d %s removed from line %d\n", pres.spaces, pl, inlines);
            fprintf(fpinfo, FMT, linein, cpy);
        }
        
        mlen = (int)strlen(cpy)+1;
        cp   = skipTitle(cpy);
        
        /* find first comma after title */
        while ((cp=strchr(cp, ',')) == NULL) {
            cp++;
        }
        *cp = '\0';
        cp++;
        
        /* we're at the start of the page number field. skip it. */
        if (!isp)  {
            while ((cp=strchr(cp, ',')) == NULL) {
                cp++;
            }
            cp++;   //skip comma
        }
        
        /* we're at the start of the number of pages field. get it. */
        if ((numpages=(int)strtol(cp,&eptr,10)) > 0) {
            if ((*eptr != ',') && (isc && (*eptr != '\n'))) {
                if (gBoguscount++ < MAXPERROR) printf("Line %d: bad number-of-pages field\n%s\n", inlines, linein);
                fprintf(fpinfo, "Line %d: bad number-of-pages field\n%s\n", inlines, linein);
                wout = 0;
                continue;
            }
            if (wout) {
                olines[outlines] = calloc(BUFSIZE,1);
                
                if (!isc) {
                    // skip the number of pages field to retrieve composer
                    cp = strchr(cp,',') + 1;
                    sprintf(olines[outlines],"%s,%d,%d,%s", cpy, curpage, numpages, cp);
                } else {
                    sprintf(olines[outlines],"%s,%d,%d,\"%s\"\n", cpy, curpage, numpages, defAuthor);
                }
                
                // return unneeded memory
                olines[outlines] = realloc(olines[outlines], strlen(olines[outlines])+1);
                curpage += numpages;
                outlines++;
            }
        }
        else {
            if (gBoguscount++ < MAXPERROR) printf("\n\nLine %d: bad number-of-pages field\n%s\n", inlines, linein);
            fprintf(fpinfo, "\n\nLine %d: bad number-of-pages field\n%s\n", inlines, linein);
            wout = 0;
            continue;
        }
    }
    
    if (gBoguscount > MAXPERROR) printf("%d more errors/warnings...\nSee %s in output directory\n\n", gBoguscount, basename(finfo));
    
    if ((fpout = fopen(fout, "w")) == NULL) {
        printf("%s: cannot open\n", fout);
        fprintf(fpinfo, "%s: cannot open\n", fout);
        outlines = 0;
    } else {
        if (wout) {
            int i;
            
            for (i=0; i<outlines; ++i) {
                // Bulk Upload will not tolerate an empty line. Kill '\n' at end of last entry (if it's there)
                if (i == (outlines-1)) {
                    char *cp = strchr(olines[i],'\n');
                    if (cp != NULL) {
                        *cp = '\0';
                    }
                }
                fprintf(fpout, "%s", olines[i]);
            }
            xtralines = MINLINES - outlines;
            if ((xtralines > 0) && isd)  {
                int i;
                
                if (outlines) fprintf(fpout,"\n");        // Current last line is missing '\n'
                for (i=0; i<(xtralines-1); ++i) {
                    fprintf(fpout, "%s%d\",999,1,\"[Author]\"\n", DUMMY_NAME,i);
                }
                fprintf(fpout, "%s%d\",999,1,\"[Author]\"", DUMMY_NAME,i);  // no '\n' on the last line
            }
            fclose (fpout);
        }
        else {
            fprintf(fpout, "\nNo lines written.\nSee %s for information\n", basename(finfo));
            outlines = 0;
        }
    }
    
    xtralines = ((xtralines > 0) && isd) ? xtralines : 0;

    printf("\n%d lines read\n%d lines written\n", inlines, outlines + xtralines);
    fprintf(fpinfo, "\n%d lines read\n%d lines written\n", inlines, outlines + xtralines);
    
    
    if (outlines < MINLINES) {
        if (isd) {
            printf("WARNING: Lines in does not meet minimum iGigBook Bulk Upload minimum of %d.\n%d dummy entries written to output to comply\n", MINLINES,xtralines);
            fprintf(fpout ,"WARNING: Lines in does not meet minimum iGigBook Bulk Upload minimum of %d.\n%d dummy entries written to output to comply\n", MINLINES, xtralines);
        }
        else {
            printf("WARNING: Lines written does not meet minimum iGigBook Bulk Upload minimum of %d.\n\n", MINLINES);
            fprintf(fpinfo, "WARNING: Lines written does not meet minimum iGigBook Bulk Upload minimum of %d.\n\n", MINLINES);
        }
    }

    fclose (fpin);
    
    exit(0);
}

static void printBanner(char *myName) {
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
    char *d = malloc(sizeof(__DATE__));
    
    strcpy(d,__DATE__);
    d[6] = '\0';
    idx  = (d[4] != ' ') ? 4 : 5;
    
    sprintf(dts, "%s/%s/%sT%s",&d[7], __MONTH__, &d[idx], __TIME__);
    printf("%s%s (%s)\n", basename(myName), ETYPE, dts);
    free(d);
}

static char *skipTitle(char *line)
{
    char *cp = line;

    while (*cp != '"')  {
        cp++;
    }
    cp++;
    while (*cp != '"')  {
        cp++;
    }

    return (cp+1);
}


static void makeofname (char *fin, char *fout, char *finfo) {
    char *cp;
    long  count;
    
    sprintf(finfo, "%s/%s_%s", dirname(fin), OPINFO, basename(fin));
    
    sprintf(fout, "%s/%s_%s", dirname(fin), OPUPLD, basename(fin));
    cp = fout + strlen(dirname(fout))+sizeof(OPUPLD);
    count = strtol(cp,NULL,10);
    
    while ((access(fout, F_OK)) != -1) {
        ++count;
        sprintf(fout, "%s/%s%ld_%s", dirname(fin), OPUPLD, count, basename(fin));
        sprintf(finfo, "%s/%s%ld_%s", dirname(fin), OPINFO, count, basename(fin));
    }
    return;
}
