#ifndef MOVABLE_WIDGETS_H
#define MOVABLE_WIDGETS_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>

#define restrict __restrict__
#define DEBUG1

gboolean
fixed_motion_notify (GtkWidget *widget,GdkEvent  *event, gpointer   user_data);
gboolean eventbox_press_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean eventbox_release_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data);
void menu_delete_activate (GtkMenuItem *menuitem, gpointer user_data);
gboolean eventbox_leave_cb (GtkWidget *widget, GdkEvent  *event, gpointer user_data);
gboolean eventbox_enter_cb (GtkWidget *widget, GdkEvent  *event, gpointer user_data);
void scrolled_window_destroy_cb (GtkWidget *object, gpointer user_data);

typedef enum {icon_t,drawing_t,svg_t,label_t, image_t} widget_type;

typedef struct{
        GtkWidget *restrict widget_ptr;
        GtkWidget *restrict eventbox;
        GtkWidget *restrict frame;  //нужно удялть самостоятельно
        GtkWidget *restrict pmenu;
        widget_type type;
        int x_position;
        int y_position;
        bool button_not_pressed;
    }InsertedWidgetWithProperty;

class page_body{
public:
    GtkWidget *restrict scrolledwindow;
    GtkWidget *restrict viewport;
    GtkWidget *restrict fixed;
    GArray *restrict widgets=g_array_new(FALSE, TRUE, sizeof(InsertedWidgetWithProperty));  ///GArray с виджетами
    GtkAdjustment *restrict h_adj;
    GtkAdjustment *restrict v_adj;
    int num_of_current_widget=0;
    double x_correction=0;
    double y_correction=0;
    GtkWidget *restrict window; ///локальная копия указателя на главное окно
    ///так как указатель не меняется, то его можно объявить как restrict
    int widget_count=0;
    bool click_order=FALSE; //TRUE - перекрывание

page_body(appdata *data, GObject *container)
{
    window=data->win;
    printf("конструктор вызван\n");
    h_adj=gtk_adjustment_new(0.0,4.0,900.0,1.0,5.0,10.0);
    v_adj=gtk_adjustment_new(0.0,4.0,900.0,1.0,5.0,10.0);
    scrolledwindow=gtk_scrolled_window_new(h_adj, v_adj);
    viewport=gtk_viewport_new(h_adj, v_adj);
    fixed=gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(scrolledwindow),GTK_WIDGET(viewport));
    gtk_container_add(GTK_CONTAINER(viewport),GTK_WIDGET(fixed));
    if(GTK_IS_NOTEBOOK(container))
    {gtk_notebook_append_page ((GtkNotebook*)container,scrolledwindow,NULL);}
    else
    if(GTK_IS_WIDGET(container))
    {gtk_container_add(GTK_CONTAINER(container),scrolledwindow);}
    g_signal_connect(fixed,"motion-notify-event",G_CALLBACK(fixed_motion_notify), this);
    g_signal_connect(scrolledwindow,"destroy",G_CALLBACK(scrolled_window_destroy_cb), this);
}

~page_body()
{
    printf("деструктор вызван\n");
    int i=widgets->len;
    if(widget_count>0)
    {
        for(i;i>=0;i--)
        {
            InsertedWidgetWithProperty *widget_with_property;
            widget_with_property=&g_array_index(widgets,InsertedWidgetWithProperty,i);
        }
    }
    g_array_free(widgets,TRUE);
}

void add_widget(GtkWidget *widget, widget_type type, int x, int y)
{
    ++widget_count;
    InsertedWidgetWithProperty *widget_with_property=(InsertedWidgetWithProperty*)
    g_malloc0(sizeof(InsertedWidgetWithProperty));

    widget_with_property->eventbox=gtk_event_box_new();
    widget_with_property->type=type;
    widget_with_property->widget_ptr=widget;
    printf("Вставлено, eventbox=%d\n",widget_with_property->eventbox);


    gtk_container_add(GTK_CONTAINER(widget_with_property->eventbox),widget);

    gtk_fixed_put(GTK_FIXED(fixed),widget_with_property->eventbox,x,y);

    widget_with_property->pmenu=gtk_menu_new();
    GtkWidget *menu_items = gtk_menu_item_new_with_label ("Удалить");
    gtk_widget_show(menu_items);
    gtk_menu_shell_append (GTK_MENU_SHELL (widget_with_property->pmenu), menu_items);

    g_signal_connect(widget_with_property->eventbox,"button-press-event",G_CALLBACK(eventbox_press_cb),this);
    g_signal_connect(widget_with_property->eventbox,"button-release-event",G_CALLBACK(eventbox_release_cb),this);
    g_signal_connect(menu_items,"activate",G_CALLBACK(menu_delete_activate),this);
    g_signal_connect(widget_with_property->eventbox,"leave-notify-event",G_CALLBACK(eventbox_leave_cb),this);
    g_signal_connect(widget_with_property->eventbox,"enter-notify-event",G_CALLBACK(eventbox_enter_cb),this);
    gtk_widget_set_events(widget_with_property->eventbox,GDK_LEAVE_NOTIFY_MASK|GDK_ENTER_NOTIFY_MASK|GDK_STRUCTURE_MASK);
    g_array_append_val(widgets, *widget_with_property);
}

inline void change_cursor(char *cursor_name)
{
    GdkDisplay *display;
    GdkCursor *cursor;
    display = gtk_widget_get_display (window);
    if(cursor_name)
    cursor = gdk_cursor_new_from_name (display, cursor_name);
    else cursor = gdk_cursor_new_from_name (display, "default");
    GdkWindow *gdkwindow=gtk_widget_get_window (window);
    gdk_window_set_cursor (gdkwindow, cursor);
}

inline void delete_widget(int i)
{
    InsertedWidgetWithProperty *widget_with_property=
    &g_array_index(this->widgets,InsertedWidgetWithProperty,i);
    GtkWidget *eventbox=widget_with_property->eventbox;
    g_object_ref(eventbox);
    gtk_container_remove(GTK_CONTAINER(this->fixed),eventbox);
    if(widget_with_property->frame!=NULL)
        {gtk_widget_destroy(widget_with_property->frame);}
    gtk_widget_destroy(widget_with_property->eventbox);
    this->widgets=g_array_remove_index_fast(this->widgets,i);
    --this->widget_count;
}
};

void scrolled_window_destroy_cb (GtkWidget *object, gpointer user_data)
{
    page_body *page=(page_body*) user_data;
    delete page;
}

void menu_delete_activate (GtkMenuItem *menuitem, gpointer user_data)
{
    page_body *page=(page_body*) user_data;
    page->delete_widget(page->num_of_current_widget);

}

gboolean
eventbox_leave_cb (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data)
{
    page_body *page=(page_body*) user_data;
    page->change_cursor(NULL);
}

gboolean
eventbox_enter_cb (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data)
{
    page_body *page=(page_body*) user_data;
    page->change_cursor("pointer");
}

gboolean eventbox_press_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    page_body *page=(page_body*) user_data;
    page->x_correction=event->button.x;
    page->y_correction=event->button.y;
    int i=0;
    InsertedWidgetWithProperty *widget_compare;
    for(i;i<=page->widgets->len;i++)
    {
        widget_compare=(InsertedWidgetWithProperty*) page->widgets->data+i;
        if(widget==widget_compare->eventbox)
        {
            page->num_of_current_widget=i;
            #ifdef DEBUG1
            printf("Номер текущего виджета =%d\n", i);
            #endif
            break;
        }
    }
    if(widget_compare->button_not_pressed==FALSE)
    {
        GtkWidget *eventbox=widget_compare->eventbox;
        if(page->click_order)
        {
            int x, y;
            gtk_widget_translate_coordinates(page->fixed, eventbox,0,0,&x, &y);
            gtk_container_remove(GTK_CONTAINER(page->fixed),eventbox);
            gtk_fixed_put(GTK_FIXED(page->fixed),eventbox,-x,-y);
        }
        g_object_ref(widget_compare->widget_ptr);
        gtk_container_remove(GTK_CONTAINER(eventbox),widget_compare->widget_ptr);
        if(widget_compare->frame==NULL)
        widget_compare->frame=gtk_frame_new(NULL);
        gtk_container_add(GTK_CONTAINER(widget_compare->frame),widget_compare->widget_ptr);
        gtk_container_add(GTK_CONTAINER(eventbox),widget_compare->frame);
        gtk_widget_show_all(eventbox);
        widget_compare->button_not_pressed=TRUE;
    }

    ///обработка контексного меню
     const gint RIGHT_CLICK = 3;
    if (event->type == GDK_BUTTON_PRESS)
    {
        GdkEventButton *bevent = (GdkEventButton *) event;
        if (bevent->button == RIGHT_CLICK) {
          gtk_menu_popup(GTK_MENU(widget_compare->pmenu), NULL, NULL, NULL, NULL,
              bevent->button, bevent->time);
          }
    }
    return FALSE;
}

gboolean eventbox_release_cb (GtkWidget *eventbox, GdkEvent *event, gpointer user_data)
{
    page_body *page=(page_body*) user_data;
    InsertedWidgetWithProperty *widget_with_property=
    &g_array_index(page->widgets,InsertedWidgetWithProperty,page->num_of_current_widget);
    ///событие отпускания кнопки мыши может не сработать, если нажимать часто
    if(widget_with_property->button_not_pressed==TRUE)
    {
        widget_with_property->frame=(GtkWidget*) g_object_ref(widget_with_property->frame);
        widget_with_property->widget_ptr=(GtkWidget*) g_object_ref(widget_with_property->widget_ptr);
        GtkWidget *frame=widget_with_property->frame;
        GtkWidget *widget=widget_with_property->widget_ptr;
        gtk_container_remove(GTK_CONTAINER(eventbox), frame);
        gtk_container_remove(GTK_CONTAINER(frame), widget);
        gtk_container_add(GTK_CONTAINER(eventbox), widget);
        widget_with_property->button_not_pressed=FALSE;
    }
}


gboolean
fixed_motion_notify (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data)
{

page_body *page=(page_body*) user_data;
 int x_win, y_win, x_fixed, y_fixed;
    gtk_window_get_position(GTK_WINDOW(page->window),&x_win,&y_win);
    gtk_widget_translate_coordinates(page->window,page->fixed,x_win,y_win,&x_fixed,&y_fixed);
    double correction_y=(-y_fixed+y_win)*2+25;
    double correction_x=(-x_fixed+x_win);
    double x_corr=page->x_correction;
    double y_corr=page->y_correction;
    int position_x=event->motion.x_root-x_corr-x_win-correction_x;
    int position_y=event->motion.y_root-y_corr-y_fixed-correction_y;

    InsertedWidgetWithProperty *widget_with_property=&g_array_index(page->widgets,InsertedWidgetWithProperty,page->num_of_current_widget);
    GtkWidget *fixed=page->fixed;
    GtkWidget *eventbox=widget_with_property->eventbox;
    if(position_x<-1) position_x=0;
    if(position_y<-1) position_y=0;

    gtk_fixed_move(GTK_FIXED(fixed), eventbox, position_x, position_y);
    return FALSE;
}

#endif
