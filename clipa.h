#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define APP_NAME "clipa"
#define APP_TITLE "clipa"

/** DATA **/
#define IDM_PIN_STR 1000
#define PIN_STR_LEN 10
extern wchar_t* pin_list[PIN_STR_LEN];

extern unsigned int max_item;
typedef struct _t_ClipItem {
  wchar_t* data;
  struct _t_ClipItem* next;
} ClipItem;

ClipItem* clip_list_new(void);
int clip_list_append(ClipItem* head, ClipItem* item);
int clip_list_del(ClipItem* head, int pos);
void clip_list_clean(ClipItem* head);
void clip_list_free(ClipItem** head);
wchar_t* clip_list_get_item(ClipItem* head, int pos);
int clip_list_unique_item(ClipItem* head, wchar_t* str);
ClipItem* clip_list_make_item(wchar_t* data, size_t n);
int clip_list_enter(ClipItem* head, ClipItem *item);

/** UI **/
#define MSTR_LEN 80 /* menu string length */
#define MIN(A,B) ((A) > (B) ? (B) : (A))
/** replace half shape '&' to full shape '＆' **/
#define REPLACE_AMPERSAND(A) ((A) == L'&' ? L'＆' : (A))
#define ELLIPSIS L'…'
void menu_str_fill(wchar_t* dest, wchar_t* src, size_t n);
HMENU menu_build(ClipItem* head);
void menu_show(HWND hwnd, HMENU hmenu);
void menu_hide(HMENU hmenu);

/** LOGIC **/
#define IDM_VOID 2000
#define IDM_PAUSE 2001
#define IDM_COPY_AND_REMOVE 2002
#define IDM_NEW_ITEM_FIRST 2003
#define IDM_UNIQUE 2004
#define IDM_COPY_AND_REMOVE_ALL 2005
#define IDM_EXIT 2006

extern int pause_mode;
extern int copy_rm_mode;
extern int new_item_first_mode;
extern int unique_mode;

void do_menu_action_copy_and_remove_all(HWND hwnd, ClipItem* head, wchar_t join);
void do_menu_action(HWND hwnd, WPARAM wParam, ClipItem* head);
void clipboard_read(HWND hwnd, ClipItem* head);
void clipboard_write(HWND hwnd, wchar_t* str, size_t n);

/** EVENT **/
#define WM_USER_CHECK_CLIPBOARD (WM_USER + 1)
#define HOTKEY_ID 9001
extern unsigned int hotkey_mod;
extern unsigned int hotkey_vk;
int on_create(HWND hwnd);
void on_destroy(HWND hwnd);
void on_hotkey(HWND hwnd, WPARAM wParam);

/** Declaration: DEBUG **/
#if defined(DEBUG)
// Print first n characters
#define DEBUG_WSTR_START(S, N) wprintf(L"%" #N "." #N "s", S)
// Print last n characters
#define DEBUG_WSTR_END(S, N) wprintf(L"%" #N "." #N "s", S + wcslen(S) - N)
#define DEBUG_SUMMARY_WSTR(S) \
  if (wcslen(S) > 80) { \
    DEBUG_WSTR_START(S, 37); \
    printf("..."); \
    DEBUG_WSTR_END(S, 40); \
    printf("\n"); \
  } else { \
    wprintf(L"%s\n", S); \
  }
#define DEBUG_NOTE(fmt, ...) printf(fmt, ## __VA_ARGS__)
#define DEBUG_LINE printf("%s L:%d\n", __func__, __LINE__);
#define DEBUG_VAR(VAR, fmt) printf("{" #VAR "=%" fmt "}\n", VAR);
#else
#define DEBUG_WSTR_START(S, N)
#define DEBUG_WSTR_END(S, N)
#define DEBUG_SUMMARY_WSTR(S)
#define DEBUG_NOTE(fmt, ...)
#define DEBUG_LINE
#define DEBUG_VAR(VAR, fmt)
#endif

#include "cfg.h"
