#ifndef __ES7210__
#define __ES7210__
// es7243 chip addr
#define ES7210_CHIP_ADDR                               (0x40)

int ES7210_2AmicInit(void);
int ES7210_2AmicDeInit(void);
int ES7210_2DmicInit(void);
int ES7210_2DmicDeInit(void);
int ES7210_4AmicInit(void);
int ES7210_4AmicDeInit(void);
int ES7210_3AmicInit(void);
int ES7210_3AmicDeInit(void);

#endif
