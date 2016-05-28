#ifndef IMG_CTRL_H
#define IMG_CTRL_H

struct img_ctrl_struc{
  int *(*playImages)(void * type);
  int *(*haltPlay)(void * arg);
  int *(*initImages)(void * arg);
};

extern struct img_ctrl_struc *img_ctrl;

#endif
