#ifndef MAIN_H
#define MAIN_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>

#define restrict __restrict__

class appdata {
public:
char *glade_name=(char*)"window.glade";
GtkApplication *restrict app;
GtkWidget *restrict win;
GtkNotebook *restrict notebook;
GtkImageMenuItem *restrict filenew;
GtkImageMenuItem *restrict fileopen;
GtkImageMenuItem *restrict filesave;
GtkImageMenuItem *restrict filesaveas;
GtkImageMenuItem *restrict fileexit;
GArray *restrict pages;
};

#endif
