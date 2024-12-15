#ifndef __KVM_H__
#define __KVM_H__

typedef struct
{
  int fd;
  char *namelist;
} kvm_t;


extern kvm_t *kvm_open();
extern int kvm_close();
extern int kvm_nlist();
extern int kvm_read();

#endif
