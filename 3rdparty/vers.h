#include        "rev.h"

#define STR_2(X)    #X
#define STR_1(X)    STR_2(X)

#define VER_RC      MAJOR,MINOR,REV,BUILD
#define VER_STR     STR_1(MAJOR) "." STR_1(MINOR) "." STR_1(REV) "." STR_1(BUILD)
