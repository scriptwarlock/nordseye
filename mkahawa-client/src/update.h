#ifndef UPDATE_H
#define UPDATE_H

typedef struct
{
  char sgnstr[5];
  char update_name[32];
  char dest_fname[24];
  char dest_path[128];
  long fsize;
  long fmode;
  long vernr;
  char verstr[16];
  long key;
  long lbuf;
  char cbuf[24];
} UpdateInfo;

#define MAXCHUNKSIZE 2048
struct CHUNK{
  uint pos;
  uint blen;
  char buf[MAXCHUNKSIZE];
};
#define ONUPDATESTATE 1
#define OFFUPDATESTATE 0

extern int updateState;
#endif
