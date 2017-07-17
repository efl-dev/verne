#ifndef ELM_FILE_CONFIG_H
#define ELM_FILE_CONFIG_H

typedef struct
{
   Eina_List *bookmarks;
   char display_gtk;
   const char *viewname;
   int icon_size;
   char hidden_files;
   char only_folder;
   char image_preview;
   char hide_fileinfo;
   char hide_bookmarks;
   struct{
      int folder_placement;
      int type;
      char reverse;
      char casesensetive;
   }sort;
} Config ;

extern Config *config;

/*
 * init the config sutff
 */
void elm_ext_config_init(void);

/*
 * Save the config
 */
void elm_ext_config_save(void);

/*
 * Load the config
 */
void elm_ext_config_read(void);

/*
 * Shutdown the system, the config will be cleared
 */
void elm_ext_config_shutdown(void);
/*
 * Return a list of gtk bookmarks
 *
 * @return a list of stringshares
 */
Eina_List* util_bookmarks_load_gtk(void);

/*
 * Helper function to add/del bookmark,
 * save is done.
 */
void helper_bookmarks_add(const char *ptr);
void helper_bookmarks_del(const char *ptr);

#endif
