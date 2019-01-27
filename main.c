/*
	IMCUnlockLite by SKGleba
	All Rights Reserved
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/io/dirent.h>
#include <psp2/shellutil.h>
#include <psp2/power.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>
#include <taihen.h>

#define KPPATH "ux0:app/SKGIMCULK/kp"

int scr = 0;

int callkp() {
SceUID mod_id;
tai_module_args_t argg;
	argg.size = sizeof(argg);
	argg.pid = KERNEL_PID;
	argg.args = 0;
	argg.argp = NULL;
	argg.flags = 0;
	mod_id = taiLoadStartKernelModuleForUser(KPPATH, &argg);
 if (mod_id < 0) return 0; // KP?
		argg.size = sizeof(argg);
		argg.pid = KERNEL_PID;
		argg.args = 0;
		argg.argp = NULL;
		argg.flags = 0;
		taiStopUnloadKernelModuleForUser(mod_id, &argg, NULL, NULL);
 return 1;
}

int main() {
	SceCtrlData pad, old_pad;
	unsigned int kd;
	old_pad.buttons = 0;
	int col1 = 255;
	int col2 = 255;
	int col3 = 255;
	vita2d_pgf *font;
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	font = vita2d_load_default_pgf();
	memset(&pad, 0, sizeof(pad));
	
	const char din[512] = "\nThis installer modifies boot regions of the PS Vita.\nIt is a potentially dangerous process, so DO NOT interrupt it.\n\nI provide this tool \"as is\" without warranty of any kind.\n\n\nBy SKGleba (twitter.com/skgleba)";
	
	while(1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		kd = pad.buttons & ~old_pad.buttons;
		
		vita2d_start_drawing();
		vita2d_clear_screen();
		
		switch(scr) {
			case 0:
				vita2d_pgf_draw_text(font, 275, 45, RGBA8(255,255,255,255), 1.5f, "IMCUnlock v4 LITE by SKGleba");
				vita2d_pgf_draw_text(font, 35, 100, RGBA8(255,255,255,255), 1.5f, din);
				vita2d_draw_rectangle(402, 425, 145, 70, RGBA8(col1,col2,col3,255));
				vita2d_pgf_draw_text(font, 410, 470, RGBA8(0,0,0,255), 1.5f, "Unlock !");
				break;
			case 4: 
                if (callkp() == 1) {
                    scr = 5;
                } else scr = 69;
                
				break;
			case 5:
				vita2d_pgf_draw_text(font, 35, 115, RGBA8(255,255,255,255), 1.5f, "Flash Success.\n\n\n\nPress CROSS to restart the system.\n\nPress CIRCLE to launch the enso installer.\n\nPress TRIANGLE to exit this app.");
				vita2d_draw_rectangle(402, 425, 145, 70, RGBA8(col1,col2,col3,255));
				vita2d_pgf_draw_text(font, 410, 470, RGBA8(0,0,0,255), 1.5f, "Continue");
				break;
			case 69:
				vita2d_pgf_draw_text(font, 35, 115, RGBA8(255,255,255,255), 1.5f, "EUNKDEVF");
				break;
		}
		
		if (kd & SCE_CTRL_CROSS) {
			switch(scr) {
				case 0:
					scr = 4;
					break;
				case 5:
					scePowerRequestColdReset();
					break;
			 }
		} if (kd & SCE_CTRL_TRIANGLE) {
			switch(scr) {
				case 5:
					sceKernelExitProcess(0);
					break;
			}
		} if (kd & SCE_CTRL_CIRCLE) {
			switch(scr) {
				case 5:
					sceAppMgrLaunchAppByUri(0xFFFFF, "psgm:play?titleid=MLCL00003");
					sceKernelExitProcess(0);
					break;
			}
		}
		
		old_pad = pad;
		
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}
	vita2d_fini();
	vita2d_free_pgf(font);
	
	sceKernelExitProcess(0);
	return 0;
}
