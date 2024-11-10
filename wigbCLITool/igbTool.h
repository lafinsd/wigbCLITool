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

typedef struct {
    int spaces;
    int errors;
} PARSE_RES;
