#ifndef __PSXPORT_H__
#define __PSXPORT_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <assert.h>

struct ps1_controller {
	uint8_t id;
	uint8_t joy_right_ax0;
	uint8_t joy_right_ax1;
	uint8_t joy_left_ax0;
	uint8_t joy_left_ax1;
	uint8_t Vib[2];
	uint8_t VibF[2];
	uint8_t pad_mode;
	uint8_t pad_controllertype;
	uint8_t configmode;
};

extern struct ps1_controller player_controller[2];

///////////////////////////
// Windows compatibility //
///////////////////////////
#if defined(_WIN32) && !defined(__CYGWIN__)
// Windows lacks fsync():
static inline int fsync(int f) { return 0; }
#endif

#ifndef INLINE
#if defined(__WIN32__) || defined(__INTEL_COMPILER)
#define INLINE __inline
#elif defined(__STDC_VERSION__) && __STDC_VERSION__>=199901L
#define INLINE inline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif
#endif

INLINE int string_is_empty(const char *data)
{
	return (!data || (*data == '\0')) ? 1 : 0;
}

int path_file_exists(const char *path);
void set_cdrom_name(const char *filepath);

enum ipu_filter_type
{
	IPU_FILTER_BICUBIC  = 0,
	IPU_FILTER_BILINEAR = 1,
	IPU_FILTER_NEAREST  = 2
};

#define	CONFIG_VERSION	0

void config_get_override_filename(const char *diskname, char *filename);
int config_load(const char *diskname);
int config_save(const char *diskname);

extern int config_override_enabled;
extern int config_override_active;

unsigned get_ticks(void);
void wait_ticks(unsigned s);
void pad_update(void);
unsigned short pad_read(int num);

void video_flip(void);
#ifdef GPU_DFXVIDEO
void video_set(unsigned short* pVideo,unsigned int width,unsigned int height);
#endif
void video_clear(void);
void video_clear_cache();
void port_printf(int x, int y, const char *text);

extern unsigned short *SCREEN;
extern int SCREEN_WIDTH, SCREEN_HEIGHT;

int state_load(int slot);
int state_save(int slot);

void update_memcards(int load_mcd);
const char *bios_file_get();
void bios_file_set(const char *filename);
void check_spec_bios();

#ifdef RUMBLE
int set_rumble_gain(unsigned gain);
int trigger_rumble(uint8_t low, uint8_t high);
#endif

int SelectGame();
int GameMenu();

#endif
