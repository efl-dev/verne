#include "main.h"

typedef struct {
    const char *key; // the key which needs to be set
    struct modifier {
        Eina_Bool ctrl;
        Eina_Bool alt;
        Eina_Bool shift;
        Eina_Bool win;
        Eina_Bool meta;
        Eina_Bool hyper;
    } mods;
    void (*callback)(void); // callback which will get called
} Shortcut;


#define ONLY_CTRL {EINA_TRUE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE}
#define ALWAYS {EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE}

#define CHECK(v, k) \
    if (v && !evas_key_modifier_is_set(ev->modifiers, k)) \
      return EINA_FALSE;

static void window_close(void);

// the list of shortcuts
static Shortcut shortcuts[] = {
    {"c", ONLY_CTRL, preview_copy},
    {"v", ONLY_CTRL, preview_paste},
    {"x", ONLY_CTRL, preview_move},
    {"q", ONLY_CTRL, window_close},
    {"Delete", ALWAYS, preview_remove},
    {NULL, ONLY_CTRL, NULL}
};

static Eina_Bool
_mods_check(Shortcut *sc, Evas_Event_Key_Down *ev)
{
    Eina_Bool meta;

    CHECK(sc->mods.ctrl, "Control");
    CHECK(sc->mods.alt, "Alt");
    CHECK(sc->mods.shift, "Shift");
    CHECK(sc->mods.win, "Super");
    CHECK(sc->mods.hyper, "hyper")

    meta = evas_key_modifier_is_set(ev->modifiers, "Meta") ||
           evas_key_modifier_is_set(ev->modifiers, "AltGr") ||
           evas_key_modifier_is_set(ev->modifiers, "ISO_Level3_Shift");

    if (sc->mods.meta && !meta)
      return EINA_FALSE;

    return EINA_TRUE;

}

static void
_search_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Evas_Event_Key_Down *ev = event_info;
    for (int i = 0; shortcuts[i].key ; i++)
      {
         if (!!strcmp(ev->key, shortcuts[i].key))
           continue;
         if (!_mods_check(&shortcuts[i], ev))
           continue;

         shortcuts[i].callback();
      }

}

static void
_hover(void *data EINA_UNUSED, const Efl_Event *event) {
   Elm_File_Icon *icon = event->info;
   const char *path;

   elm_file_selector_file_set(selector, elm_file_icon_file_get(icon));
   path = efm_file_path_get(icon);
   titlebar_path_set(path);
}

static Eina_List*
_convert( Elm_Selection_Data *data)
{
   Eina_List *result = NULL;
   char **files;
   char *string;
   //first check if this is a string which ends with \0

   string = malloc(sizeof(char) * (data->len + 1));

   memcpy(string, data->data, data->len);
   string[data->len] = '\0';

   files = eina_str_split(string, "\n", 0);
   for (int i = 0; files[i]; i++)
     {
        if (ecore_file_exists(files[i]))
          result = eina_list_append(result, files[i]);
     }

   return result;
}

static void
_move(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_List *passes;
   Efm_File *f;

   passes = efl_key_data_get(data, "__passes");
   f = efl_key_data_get(data, "__file");

   fs_operations_move(passes, efm_file_path_get(f));

   evas_object_del(data);
}

static void
_copy(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_List *passes;
   Efm_File *f;

   passes = efl_key_data_get(data, "__passes");
   f = efl_key_data_get(data, "__file");

   fs_operations_copy(passes, efm_file_path_get(f));

   evas_object_del(data);
}

static void
_cancel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_ask(Eina_List *passes, Efm_File *basic)
{
   const char *path;
   Eina_Strbuf *buf;
   Evas_Object *obj, *o, *bx;
   Eina_List *n;


   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, "Copy or Move the files: <br>");
   EINA_LIST_FOREACH(passes, n, path)
     {
        eina_strbuf_append_printf(buf, "\t<b>%s</b>\n <br>", path);
     }

   path = efm_file_path_get(basic);
   eina_strbuf_append_printf(buf, "to <b>%s</b> <br>\n", path);

   obj = efl_add(ELM_DIALOG_CLASS, NULL);
   elm_win_title_set(obj, "Copy or Move");
   elm_dialog_icon_set(obj, "dialog-question");
   elm_dialog_text_set(obj, eina_strbuf_string_get(buf));
   efl_key_data_set(obj, "__passes", passes);
   efl_key_data_set(obj, "__file", basic);
   bx = elm_dialog_control_box_get(obj);

   o = elm_button_add(obj);
   elm_object_text_set(o, "Move");
   evas_object_smart_callback_add(o, "clicked", _move, obj);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(obj);
   elm_object_text_set(o, "Copy");
   evas_object_smart_callback_add(o, "clicked", _copy, obj);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(obj);
   elm_object_text_set(o, "Cancel");
   evas_object_smart_callback_add(o, "clicked", _cancel, obj);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   evas_object_show(obj);
}

static void
_handle_action(Elm_Selection_Data *data, Efm_File *basic)
{
   Eina_List *passes = NULL;

   passes = _convert(data);

   switch(data->action){
     case ELM_XDND_ACTION_ASK:
        _ask(passes, basic);
        break;
     case ELM_XDND_ACTION_COPY:
        fs_operations_copy(passes, efm_file_path_get(basic));
        break;
     case ELM_XDND_ACTION_MOVE:
        fs_operations_move(passes, efm_file_path_get(basic));
        break;
     case ELM_XDND_ACTION_DESCRIPTION:
       /* tell what the data is */
     case ELM_XDND_ACTION_LINK:
       /* TODO create symlinks */
     case ELM_XDND_ACTION_LIST:
       /* just list the data */
       printf("HELP i am not implemented %d\n", data->action);
     default:
      printf("Error action not understood\n");
     break;
   }
}

static void
_item_drop(void *data EINA_UNUSED, const Efl_Event *event) {
   Elm_File_Selector_Dnd_Drop_Event *ev;

   ev = event->info;

   _handle_action(ev->selection_data, ev->file);
}

static void
_drop(void *data EINA_UNUSED, const Efl_Event *event) {
   _handle_action(event->info,
        elm_file_selector_file_get(event->object));
}

void
shortcuts_init()
{
   //init key shortcuts
   evas_object_event_callback_add(selector, EVAS_CALLBACK_KEY_DOWN, _search_key_down, NULL);

   //add dnd shortcut
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_DND_ITEM_HOVER, _hover, NULL);

   //add dnd shortcut
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_DND_ITEM_DROPED, _item_drop, NULL);

   //add dnd shortcut
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_DND_DROPED, _drop, NULL);
}

static void
window_close(void)
{
  elm_exit();
}
