//
//  igbTool.h
//  wigbCLITool
//
//  Created by Larry on 11/9/24.
//

#define BUFSIZE  200
#define MAXLINES 500  // Limited by iGigBook Bulk Upload constraints
#define MINLINES  50  // Limited by iGigBook Bulk Upload constraints
#define MAXPERROR 10

#define DEFAUTHOR  "[Author]"

typedef enum {
    E_NONE,
    E_EMPTY,
    E_TOKEN
} ETYPE;

typedef struct {
    int   spaces;
    int   num_errors;
    ETYPE error;
} PARSE_RES;

typedef struct {
    int   initpage;
    int   isc;
    int   isp;
    char *defAuthor;
    char *olines[MAXLINES];
    FILE *fpin;
    FILE *fpinfo;
} PRF_INP;

typedef struct {
    int wout;
    int inlines;
    int outlines;
} PRF_OUTP;

PARSE_RES parseLine(char *, int, int, FILE *);
int processSrcFile(int, int, int, int *, char *[], char *, FILE *, FILE *);
//int processSrcFile(PRF_INP *, int *, char *[]);
