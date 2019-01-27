/*
	IMCTool plugin v4.0 - MINI
	By SKGleba
	All Rights Reserved
*/

#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

typedef struct {
	uint32_t off;
	uint32_t sz;
	uint8_t code;
	uint8_t type;
	uint8_t active;
	uint32_t flags;
	uint16_t unk;
} __attribute__((packed)) partition_t;

static partition_t hmb = { 0x001B8000, 0x00048000, 0x8, 0x7, 0, 0x00000FFF, 0x0000 }; // Partition @0x001B8000 * 0x200, size 0x00048000 * 0x200, -userext id, exFat, inactive, flags 0x00000FFF (default), location - actual device

int siofix(void *func) {
	int ret = 0;
	int res = 0;
	int uid = 0;
	ret = uid = ksceKernelCreateThread("siofix", func, 64, 0x10000, 0, 0, 0);
	if (ret < 0){ret = -1; goto cleanup;}
	if ((ret = ksceKernelStartThread(uid, 0, NULL)) < 0) {ret = -1; goto cleanup;}
	if ((ret = ksceKernelWaitThreadEnd(uid, &res, NULL)) < 0) {ret = -1; goto cleanup;}
	ret = res;
cleanup:
	if (uid > 0) ksceKernelDeleteThread(uid);
	return ret;}

int ex(const char* filloc) {
  SceUID fd;
  fd = ksceIoOpen(filloc, SCE_O_RDONLY, 0);
  if (fd < 0) {ksceIoClose(fd); return 0;}
  ksceIoClose(fd);
  return 1;
}

int work(void) {

    if (ex("ur0:temp/tmbr.img") == 1) ksceIoRemove("ur0:temp/tmbr.img");
    
    // Dump 256kb to a temp file
	SceUID fd = ksceIoOpen("sdstor0:int-lp-act-entire", SCE_O_RDONLY, 0777); // eMMC
	SceUID wfd = ksceIoOpen("ur0:temp/tmbr.img", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
	static char buffer[0x8000];
	unsigned int i = 0;
    ksceIoLseek(fd, 0, SCE_SEEK_SET);
	for(i=0;i<0x40000;i=i+0x8000){
		ksceIoRead(fd, buffer, 0x8000);
		ksceIoWrite(wfd, buffer, 0x8000);}
	if (fd > 0) ksceIoClose(fd);
	if (wfd > 0) ksceIoClose(wfd);
	
	// Read needed entries from the temp file
	static partition_t upt;
	static partition_t uptn;
	SceUID dfd = ksceIoOpen("ur0:temp/tmbr.img", SCE_O_RDWR, 0777);
    ksceIoLseek(dfd, 0xE0 + 0x3C, SCE_SEEK_SET); // lseek to default (read-only ptable + default-empty entry)
	ksceIoRead(dfd, &upt, sizeof(upt));
	ksceIoLseek(dfd, 0x2E0 + 0x3C, SCE_SEEK_SET); // lseek to enso's (read-only ptable + default-empty entry)
	ksceIoRead(dfd, &uptn, sizeof(uptn));
	if (dfd > 0) ksceIoClose(dfd);
	
	// Default ptable entry
	if (upt.code == 0 && upt.type == 0 && upt.off == 0) { // Entry not used
	    upt = hmb; // Copy our entry
	} else if (upt.code == 8 && upt.type == 7 && upt.sz == hmb.sz) { // Our mod - active
	    upt.sz = 0; // Disable
	} else if (upt.code == 8 && upt.type == 7 && upt.sz == 0) { // Our mod - inactive
	    upt.sz = hmb.sz; // Enable
	} else {
	    return 0; // n/a
	}
	
	// Enso ptable entry
	if (uptn.code == 0 && uptn.type == 0 && uptn.off == 0 && uptn.sz != 0) { // Entry default-empty
	    uptn = hmb; // Copy our entry
	} else if (uptn.code == 8 && uptn.type == 7 && uptn.sz == hmb.sz) { // Our mod - active
	    uptn.sz = 0; // Disable
	} else if (uptn.code == 8 && uptn.type == 7 && uptn.sz == 0) { // Our mod - inactive
	    uptn.sz = hmb.sz; // Enable
	} 
	
	// Write both entries back
	dfd = ksceIoOpen("ur0:temp/tmbr.img", SCE_O_RDWR, 0777);
    ksceIoLseek(dfd, 0xE0 + 0x3C, SCE_SEEK_SET);
	ksceIoWrite(dfd, &upt, sizeof(upt));
	ksceIoLseek(dfd, 0x2E0 + 0x3C, SCE_SEEK_SET);
	ksceIoWrite(dfd, &uptn, sizeof(uptn));
	if (dfd > 0) ksceIoClose(dfd);
	
	// Write 256kb back to eMMC
	SceUID afd = ksceIoOpen("ur0:temp/tmbr.img", SCE_O_RDONLY, 0777);
	SceUID bfd = ksceIoOpen("sdstor0:int-lp-act-entire", SCE_O_RDWR, 0777);
    ksceIoLseek(afd, 0, SCE_SEEK_SET);
	for(i=0;i<0x40000;i=i+0x8000){
		ksceIoRead(afd, buffer, 0x8000);
		ksceIoWrite(bfd, buffer, 0x8000);}
	if (afd > 0) ksceIoClose(afd);
	if (bfd > 0) ksceIoClose(bfd);
    ksceIoRemove("ur0:temp/tmbr.img");
 return 1;
}

int pwork(void) {
	int ret = 0;
	int state = 0;
	ENTER_SYSCALL(state);
	ret = siofix(work);
	EXIT_SYSCALL(state);
    return ret;
}

void _start() __attribute__ ((weak, alias("module_start")));
 int module_start(SceSize args, void *argp) {
    if (pwork() == 1) {
	    return SCE_KERNEL_START_SUCCESS;
	} else {
	    return SCE_KERNEL_START_FAILED;
	}
 }

 int module_stop(SceSize args, void *argp) {
    return SCE_KERNEL_STOP_SUCCESS;
 }
