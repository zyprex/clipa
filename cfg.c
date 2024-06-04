#include "clipa.h"

static void set_pin_str(int pos, const char* filename) {
  char* nl = strrchr(filename, '\n');
  if (nl) *nl = '\0';

  FILE* fp = fopen(filename, "rb");
  if (!fp) return;

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  pin_list[pos] = malloc(fsize + sizeof(wchar_t));
  fread(pin_list[pos], fsize, 1, fp);
  pin_list[pos][fsize] = L'\0';

  fclose(fp);
}


int load_cfg() {

  FILE* fp = _wfopen(L"cfg.txt", L"r");
  if (fp == NULL) {
    return 1;
  }

  hotkey_mod = 0;
  hotkey_vk = 0;

  char buf[MAX_LINE];
  while(fgets(buf, MAX_LINE, fp)) {

    if (buf[0] == ';') continue;

    if (FIND("hotkey.alt", buf))   hotkey_mod |= MOD_ALT;
    if (FIND("hotkey.ctrl", buf))  hotkey_mod |= MOD_CONTROL;
    if (FIND("hotkey.shift", buf)) hotkey_mod |= MOD_SHIFT;
    if (FIND("hotkey.win", buf))   hotkey_mod |= MOD_WIN;
    if (FIND("hotkey.char ", buf)) {
      hotkey_vk = toupper(buf[strlen("hotkey.char ")]);
    }
    if (FIND("max_item ", buf)) {
      max_item = atoi(buf + strlen("max_item "));
    }
    if (FIND("mode.pause", buf)) pause_mode = 1;
    if (FIND("mode.copy_rm", buf)) copy_rm_mode = 1;
    if (FIND("mode.new_item_first", buf)) new_item_first_mode = 1;
    if (FIND("mode.unique", buf)) unique_mode = 1;

    if (FIND("p0 ", buf)) set_pin_str(0, &buf[strlen("p0 ")]);
    if (FIND("p1 ", buf)) set_pin_str(1, &buf[strlen("p1 ")]);
    if (FIND("p2 ", buf)) set_pin_str(2, &buf[strlen("p2 ")]);
    if (FIND("p3 ", buf)) set_pin_str(3, &buf[strlen("p3 ")]);
    if (FIND("p4 ", buf)) set_pin_str(4, &buf[strlen("p4 ")]);
    if (FIND("p5 ", buf)) set_pin_str(5, &buf[strlen("p5 ")]);
    if (FIND("p6 ", buf)) set_pin_str(6, &buf[strlen("p6 ")]);
    if (FIND("p7 ", buf)) set_pin_str(7, &buf[strlen("p7 ")]);
    if (FIND("p8 ", buf)) set_pin_str(8, &buf[strlen("p8 ")]);
    if (FIND("p9 ", buf)) set_pin_str(9, &buf[strlen("p9 ")]);
  }

  if (hotkey_mod == 0 && hotkey_vk == 0) {
    hotkey_mod = MOD_ALT;
    hotkey_vk = '0';
  }

  if (max_item < 1 || max_item > 999) {
    max_item = 25;
  }

  fclose(fp);
  return 0;
}
