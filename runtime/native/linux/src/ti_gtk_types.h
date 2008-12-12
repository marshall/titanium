
#ifndef __TI_GTK_TYPES_H
#define __TI_GTK_TYPES_H
#include "ti_types.h"

class TiGtkUserWindow : public TiUserWindow {

    public:
        TiGtkUserWindow();
        void change_title(const gchar*);

        void hide(bool animate);
        void show(bool animate);

    protected:
        GtkWindow* gtk_window;
        gchar* window_title;
};

#endif
