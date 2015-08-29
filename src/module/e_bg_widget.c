#include "../widgets/Elementary_Ext.h"
#include "e_bg_widget.eo.h"

typedef struct {
   int iAmNotEmpty;
} E_Bg_Widget_Data;

EOLIAN static Elm_File_Icon *
_e_bg_widget_elm_file_selector_icon_generate(Eo *obj, E_Bg_Widget_Data *pd, Efm_File *file)
{
   Evas_Object *ic;
   Elm_File_MimeType_Cache *cache;

   cache = elm_file_selector_cache_get();

   ic = eo_add(ELM_FILE_ICON_CLASS, obj,
   elm_obj_file_icon_install(cache, file, EINA_TRUE));
   evas_object_show(ic);

   return ic;
}
#include "e_bg_widget.eo.x"