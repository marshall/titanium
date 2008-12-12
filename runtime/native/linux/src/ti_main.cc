#include "ti_gtk_types.h"

int main(int argc, char* argv[]) {
    gtk_init (&argc, &argv);

    TiGtkUserWindow w = TiGtkUserWindow();

    gtk_main();
}
