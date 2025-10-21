#ifndef __CFG_H__
#define __CFG_H__

#define RAM1_SIZE   0xC000
#define RAM1_BASE   0x20000000
#define RAM1_LIMIT  (RAM1_SIZE + RAM1_BASE)

#define STACK_SIZE  (0x1000)

#define HEAP_SIZE   (RAM1_LIMIT - AlignExpr(ImageLimit(RW_IRAM1),8))

#endif
