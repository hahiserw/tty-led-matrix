#ifndef _MAGIC_H
#define _MAGIC_H

// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
// https://github.com/18sg/uSHET/blob/master/lib/cpp_magic.h
// and "macro linked lists"


#define CAT(a, b) a ## b
#define ECAT(a, b) CAT(a, b)

#define INC(x) ECAT(INC_, x)
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INC_9 10


#endif
