#ifndef VERIFIERS_H
#define VERIFIERS_H

//Permissions
//Tariff Permissions
#define PERMTARIFEDIT      (1<<0)
#define PERMTARIFSELECT    (1<<1)
//Member Permissions
#define PERMMBREDIT        (1<<5)
#define PERMMBRALLOC       (1<<6)
#define PERMLOGSUMVIEW     (1<<8)
#define PERMLOGSUMSAVE     (1<<9)

//Cashing Permissions
#define PERMCASHDISCOUNT   (1<<10)
#define PERMCASHCANCEL     (1<<11)
#define PERMCASHRECEIVE    (1<<12)
#define PERMCASHEDIT       (1<<13)
//Product Permissions
#define PERMPRODEDIT       (1<<20)
#define PERMPRODSELL       (1<<21)
#define PERMPRODSTOCK      (1<<22)
//Employee Permissions
#define PERMEMPEDIT        (1<<25)
//Tickets Permissions
#define PERMTKTPRN        (1<<26)
#define PERMTKTGEN         (1<<27)
#define PERMTKTQRY           (1<<28)

//Settings Permissions
#define PERMSETTINGS      (1<<29)

//General permissions
#define PERMGENCONF        (1<<30)

int isDate(char *string);
int isTime(char *string);
int isPrice(char *string);
int isPermitted(unsigned long perm);
#endif
