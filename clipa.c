#include "clipa.h"

/** Global Variables **/

HWND hwnd_nxv = NULL; /* next viewer */
HWND hwnd_prev = NULL; /* previous window */
HMENU hmenu_all = NULL; /* main menu */
HMENU hmenu_pin = NULL; /* menu pin */
HMENU hmenu_mode = NULL; /* menu mode */

unsigned int hotkey_mod = MOD_ALT;
unsigned int hotkey_vk = '0';

int pause_mode = 0; /* on/off pause mode flag */
int copy_rm_mode = 0; /* on/off copy and remove mode flag */
int new_item_first_mode = 0; /* put new item first */
int unique_mode = 0; /* don't record duplicated item */

/** Implementation: DATA **/
wchar_t* pin_list[10];

void pin_list_free() {
  for (int i = 0; i < PIN_STR_LEN; ++i) {
    if (pin_list[i] != NULL)
      free(pin_list[i]);
  }
  DEBUG_LINE;
}

ClipItem* clip_list = NULL; /* clip list data */
static int clip_list_lock = 0; /* lock flag */

unsigned int max_item = 25;
static unsigned int counter = 0;

ClipItem* clip_list_new(void) {
  ClipItem* head = malloc(sizeof(ClipItem));
  head->data = NULL;
  head->next = NULL;
  counter = 0;
  DEBUG_LINE;
  return head;
}

int clip_list_append(ClipItem* head, ClipItem* item) {
  ClipItem* ptr = head;
  while (ptr->next != NULL) {
    ptr = ptr->next;
  }
  ptr->next = item;
  DEBUG_LINE;
  return ++counter;
}

int clip_list_del(ClipItem* head, int pos) {
  if (counter == 0) return -1;
  if (pos < 1 || pos > max_item) return -1;

  ClipItem* prev_item = NULL;
  ClipItem* del_item = NULL;

  ClipItem* ptr = head;
  int cnt = 0;
  while (ptr->next != NULL) {
    if (cnt == pos - 1) {
      prev_item = ptr;
      del_item = ptr->next;
      prev_item->next = del_item->next;

      DEBUG_NOTE("DELETED:");
      DEBUG_SUMMARY_WSTR(del_item->data);

      free(del_item->data);
      free(del_item);
      del_item = NULL;
      --counter;

      DEBUG_NOTE("DELETED SLOT [%d]\n", pos);
      break;
    }
    ++cnt;
    ptr = ptr->next;
  }

  return counter;
}

void clip_list_clean(ClipItem* head) {
  int times = counter;
  for (int i = 1; i <= times; ++i) {
    clip_list_del(head, 1);
  }
  DEBUG_LINE;
}

void clip_list_free(ClipItem** head) {
  clip_list_clean(*head);
  free(*head);
  *head = NULL;
  DEBUG_LINE;
}

wchar_t* clip_list_get_item(ClipItem* head, int pos) {
  int cnt = 0;
  ClipItem* ptr = head;
  while (ptr->next != NULL) {
    ptr = ptr->next;
    ++cnt;
    if (cnt == pos) {
      DEBUG_LINE;
      return ptr->data;
    }
  }
  return NULL;
}

int clip_list_unique_item(ClipItem* head, wchar_t* str) {
  DEBUG_LINE;
  int cnt = 0;
  ClipItem* ptr = head;
  while (ptr->next != NULL) {
    ptr = ptr->next;
    ++cnt;
    if (!wcscmp(str, ptr->data)) {
      clip_list_del(head, cnt);
    }
  }
  DEBUG_LINE;
  return 0;
}

ClipItem* clip_list_make_item(wchar_t* data, size_t n) {
  ClipItem* item = malloc(sizeof(ClipItem));
  item->data = malloc(n);
  wcscpy(item->data, data);
  item->next = NULL;
  DEBUG_LINE;
  return item;
}

int clip_list_enter(ClipItem* head, ClipItem *item) {
  if (counter == max_item) {
    clip_list_del(head, 1);
  }
  clip_list_append(head, item);
  DEBUG_LINE;
  DEBUG_NOTE("SAVE TO SLOT [%d]\n", counter);
  return counter;
}

/** Implementation: UI **/

void menu_str_fill(wchar_t* dest, wchar_t* src, size_t n) {
  int maxlen = MIN(n, MSTR_LEN);
  DEBUG_VAR(maxlen, "d");
  ZeroMemory(dest, maxlen);
  int i;
  for (i = 0; i < maxlen; ++i) {
    dest[i] = REPLACE_AMPERSAND(src[i]);
  }
  DEBUG_VAR(i, "d");
  if (n > MSTR_LEN) {
    int i;
    for (i = 1; i < MSTR_LEN / 2 - 1; ++i) {
      dest[MSTR_LEN - i] = REPLACE_AMPERSAND(src[n - i]);
    }
    dest[MSTR_LEN - i - 0] = ELLIPSIS;
    dest[MSTR_LEN - i - 1] = ELLIPSIS;
    /* dest[MSTR_LEN - i - 2] = ELLIPSIS; */
    dest[MSTR_LEN] = '\0';
    return;
  }
  dest[i] = '\0';
}

#define menu_add_long_item(hmenu, str, id) \
  do { \
    wchar_t menu_str[MSTR_LEN]; \
    menu_str_fill(menu_str, str, wcslen(str)); \
    AppendMenuW(hmenu, MF_STRING, id, menu_str); \
    DEBUG_NOTE("long item id = %d\n", id); \
  } while(0);

static HMENU menu_pin_build() {
  if (hmenu_pin != NULL)
    return hmenu_pin;

  hmenu_pin = CreatePopupMenu();

  for (int i = 0; i < PIN_STR_LEN; ++i) {
    if (pin_list[i] != NULL) {
      wchar_t* str = pin_list[i];
      menu_add_long_item(hmenu_pin, str, i+IDM_PIN_STR);
    }
  }

  return hmenu_pin;
}

static HMENU menu_mode_build() {
  if (hmenu_mode != NULL)
    return hmenu_mode;

  hmenu_mode = CreatePopupMenu();

  AppendMenuW(hmenu_mode, MF_STRING
      | pause_mode ? MF_CHECKED : MF_UNCHECKED
      , IDM_PAUSE, L"Pause\t(&P)");
  AppendMenuW(hmenu_mode, MF_STRING
      | copy_rm_mode ? MF_CHECKED : MF_UNCHECKED,
      IDM_COPY_AND_REMOVE, L"Copy And Remove\t(&C)");
  AppendMenuW(hmenu_mode, MF_STRING
      | new_item_first_mode ? MF_CHECKED : MF_UNCHECKED,
      IDM_NEW_ITEM_FIRST, L"New Item First\t(&F)");
  AppendMenuW(hmenu_mode, MF_STRING
      | unique_mode ? MF_CHECKED : MF_UNCHECKED,
      IDM_UNIQUE, L"Unique Item\t(&U)");

  return hmenu_mode;
}

HMENU menu_build(ClipItem* head) {
  HMENU hmenu_pop = CreatePopupMenu();

  if (counter > 0) {
    if (new_item_first_mode) {
      for (int i = counter; i > 0; --i) {
        wchar_t* str = clip_list_get_item(head, i);
        menu_add_long_item(hmenu_pop, str, i);
      }
    } else {
      for (int i = 1; i <= counter; ++i) {
        wchar_t* str = clip_list_get_item(head, i);
        menu_add_long_item(hmenu_pop, str, i);
      }
    }
  }

  AppendMenuW(hmenu_pop, MF_SEPARATOR, IDM_VOID, 0);

  AppendMenuW(hmenu_pop, MF_STRING | MF_POPUP,
      (UINT_PTR)menu_pin_build(), L"Pin String\t(&S)");

  AppendMenuW(hmenu_pop, MF_STRING | MF_POPUP,
      (UINT_PTR)menu_mode_build(), L"Quick Settings\t(&Q)");

  AppendMenuW(hmenu_pop, MF_SEPARATOR, IDM_VOID, 0);

  AppendMenuW(hmenu_pop, MF_STRING, IDM_COPY_AND_REMOVE_ALL, L"Copy And Remove All\t(&L)");
  AppendMenuW(hmenu_pop, MF_STRING, IDM_EXIT, L"Exit\t(&X)");
  DEBUG_LINE;
  return hmenu_pop;
}

void menu_show(HWND hwnd, HMENU hmenu) {
  POINT pt;
  GetCursorPos(&pt);
  /* GetCaretPos(&pt); */
  TrackPopupMenu(hmenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
  DEBUG_LINE;
}

void menu_hide(HMENU hmenu) {
  DestroyMenu(hmenu);
  DEBUG_LINE;
}

/** Implementation: LOGIC **/
void do_menu_action_copy_and_remove_all(HWND hwnd, ClipItem* head, wchar_t join) {
  ClipItem* ptr = head;
  unsigned int len = 0;
  while (ptr->next != NULL) {
    ptr = ptr->next;
    len += wcslen(ptr->data);
  }
  len += counter;
  DEBUG_NOTE("all len = %d\n", len);
  wchar_t* content = malloc(len*sizeof(wchar_t));
  wchar_t* tail = content;
  wchar_t* src = NULL;
  ptr = head;
  while (ptr->next != NULL) {
    ptr = ptr->next;
    src = ptr->data;
    while (*src) {
      *tail++ = *src++;
    }
    *tail++ = join;
  }
  clipboard_write(hwnd, content, len + 1);
  clip_list_clean(head);
  free(content);
}


void do_menu_action(HWND hwnd, WPARAM wParam, ClipItem* head) {
  if (wParam == IDM_COPY_AND_REMOVE_ALL) {
    do_menu_action_copy_and_remove_all(hwnd, head, L'\n');
    DEBUG_NOTE("menu action: copy and remove all\n");
    return;
  }

  if (wParam == IDM_COPY_AND_REMOVE) {
    copy_rm_mode = !copy_rm_mode;
    DEBUG_NOTE("menu action: toggle copy and remove mode to %d\n", copy_rm_mode);
    return;
  }

  if (wParam == IDM_PAUSE) {
    pause_mode = !pause_mode;
    DEBUG_NOTE("menu action: toggle pause mode to %d\n", pause_mode);
    return;
  }

  if (wParam == IDM_NEW_ITEM_FIRST) {
    new_item_first_mode = !new_item_first_mode;
    DEBUG_NOTE("menu action: toggle new item first mode to %d\n", new_item_first_mode);
    return;
  }

  if (wParam == IDM_UNIQUE) {
    unique_mode = !unique_mode;
    DEBUG_NOTE("menu action: toggle unique mode to %d\n", unique_mode);
    return;

  }

  if (wParam == IDM_EXIT) {
    SendMessage(hwnd, WM_DESTROY, 0, 0);
    DEBUG_NOTE("menu action: exit\n");
    return;
  }


  SetForegroundWindow(hwnd_prev);

  int pos = (int)wParam;

  wchar_t* content;
  if (pos >= IDM_PIN_STR) {
    content = pin_list[pos-IDM_PIN_STR];
  } else {
    content = clip_list_get_item(head, pos);
  }

  unsigned int len = wcslen(content);

  if (copy_rm_mode) {
    clipboard_write(hwnd, content, len + 1);
    clip_list_del(head, pos);
    return;
  }

  /* FIXME: some edit control can't recive this message! */
  for (int i = 0; i < len; ++i) {
    SendMessageW(hwnd_prev, WM_CHAR, content[i], 0);
  }

  clipboard_write(hwnd, content, len + 1);
}

void clipboard_read(HWND hwnd, ClipItem* head) {
  if (pause_mode)
    return;
  if (clip_list_lock) {
    clip_list_lock = 0;
    DEBUG_NOTE("CLIP LIST LOCKED\n");
    return;
  }
  if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    OpenClipboard(hwnd);
    HGLOBAL hglobal = GetClipboardData(CF_UNICODETEXT);
    if (hglobal) {
      wchar_t* content = (wchar_t*)GlobalLock(hglobal);
      DEBUG_NOTE("CLIPBOARD [CF_UNICODETEXT]:");
      DEBUG_SUMMARY_WSTR(content);
      size_t hglobal_size = GlobalSize(hglobal);
      DEBUG_VAR(hglobal_size, "I64d");

      if (unique_mode) {
        clip_list_unique_item(head, content);
      }

      clip_list_enter(head, clip_list_make_item(content, hglobal_size));
      GlobalUnlock(hglobal);
    }
    CloseClipboard();
  }
}

void clipboard_write(HWND hwnd, wchar_t* str, size_t n) {
  clip_list_lock = 1;

  HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, n * sizeof(wchar_t));
  wchar_t* content = GlobalLock(hglobal);
  wcscpy(content, str);
  GlobalUnlock(hglobal);
  OpenClipboard(hwnd);
  EmptyClipboard();
  SetClipboardData(CF_UNICODETEXT, hglobal);
  CloseClipboard();
  DEBUG_LINE;
}

/** Implementation: EVENT **/

int on_create(HWND hwnd) {

  load_cfg();

  DEBUG_VAR(hotkey_mod, "d");
  DEBUG_VAR(hotkey_vk, "d");

  DEBUG_VAR(max_item, "d");

  DEBUG_VAR(pause_mode, "d");
  DEBUG_VAR(copy_rm_mode, "d");
  DEBUG_VAR(new_item_first_mode, "d");
  DEBUG_VAR(unique_mode, "d");

  clip_list = clip_list_new();
  hwnd_nxv = SetClipboardViewer(hwnd);

  if(!RegisterHotKey(hwnd, HOTKEY_ID, hotkey_mod, hotkey_vk)) {
    MessageBox(hwnd, "Error on RegisterHotKey, program exit!", APP_NAME, MB_OK| MB_ICONERROR);
    return 0;
  }
  return 1;
}

void on_destroy(HWND hwnd) {
  pin_list_free();
  clip_list_free(&clip_list);
  ChangeClipboardChain(hwnd, hwnd_nxv);
  UnregisterHotKey(hwnd, HOTKEY_ID);
  PostQuitMessage(0);
}

void on_hotkey(HWND hwnd, WPARAM wParam) {
  if (wParam == HOTKEY_ID) {
    hwnd_prev = GetForegroundWindow();
    SetForegroundWindow(hwnd);
    hmenu_all = menu_build(clip_list);
    menu_show(hwnd, hmenu_all);
  }
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam) {

  switch (wm) {
    case WM_CREATE:
      if (!on_create(hwnd)) {
        SendMessage(hwnd, WM_DESTROY, 0, 0);
      }
      break;
    case WM_DESTROY:
      on_destroy(hwnd);
      break;
    case WM_CHANGECBCHAIN:
      if ((HWND)wParam == hwnd_nxv)
        hwnd_nxv = (HWND)lParam;
      else if (hwnd_nxv)
        SendMessage(hwnd_nxv, wm, wParam, lParam);
      break;
    case WM_DRAWCLIPBOARD:
      if (hwnd_nxv)
        SendMessage(hwnd_nxv, wm, wParam, lParam);
      SendMessage(hwnd, WM_USER_CHECK_CLIPBOARD, 0, 0);
      break;
    case WM_USER_CHECK_CLIPBOARD:
      clipboard_read(hwnd, clip_list);
      break;
    case WM_HOTKEY:
      on_hotkey(hwnd, wParam);
      break;
    case WM_COMMAND:
      do_menu_action(hwnd, wParam, clip_list);
      break;
    default:
      return DefWindowProc(hwnd, wm, wParam, lParam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR szCmdLine, int iCmdShow) {

  HANDLE mutex = CreateMutex(NULL, TRUE, "clipa-single-instance"); 
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBox(NULL, "Error clipa is running, new instance exit!", APP_NAME, MB_OK| MB_ICONERROR);
    return 0;
  }

#if defined(DEBUG)
  system("CHCP 65001");
#endif
  setlocale(LC_ALL, "");

  LPCSTR szAppName = TEXT(APP_NAME);
  LPCSTR szTitle   = TEXT(APP_TITLE);

  const WNDCLASS wc = {
    .style         = CS_HREDRAW | CS_VREDRAW,
    .lpfnWndProc   = WndProc,
    .cbClsExtra    = 0,
    .cbWndExtra    = 0,
    .hInstance     = hInstance,
    .hIcon         = LoadIcon(NULL, IDI_APPLICATION),
    .hCursor       = LoadCursor(NULL, IDC_ARROW),
    .hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH),
    .lpszMenuName  = NULL,
    .lpszClassName = szAppName
  };
  if (!RegisterClass(&wc)) return 0;

  HWND hwnd;
  hwnd = CreateWindow(szAppName, szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      NULL,
      NULL,
      hInstance,
      NULL);

  ShowWindow(hwnd, 0);

  /* ShowWindow(hwnd, iCmdShow); */
  /* UpdateWindow(hwnd); */
  /* SetWindowsHookEx(WH_CALLWNDPROC, WndProc, NULL, 0); */

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

