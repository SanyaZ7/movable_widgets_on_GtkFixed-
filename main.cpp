
#include "main.hpp"
#include "movable_widgets.hpp"

void builder_init(gpointer user_data)
{
	appdata *data=(appdata*) user_data;

	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new();
	/*if (!gtk_builder_add_from_resource (builder, "/com/example/YourApp/window.glade", &error))
    {
        // загрузить файл не удалось
        g_critical ("Не могу загрузить файл: %s", error->message);
        g_error_free (error);
    }*/
    if (!gtk_builder_add_from_file (builder, "window.glade", &error))
    {
        // загрузить файл не удалось
        g_critical ("Не могу загрузить файл: %s", error->message);
        g_error_free (error);
    }
    data->win=GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    data->notebook=GTK_NOTEBOOK(gtk_builder_get_object(builder, "notebook1"));
    gtk_notebook_remove_page(data->notebook,0); ///лишняя вкладка не нужна
    gtk_builder_connect_signals (builder,data);
    g_clear_object(&builder);
}

void application_activate(GtkApplication *application, gpointer user_data)
{
	appdata *data=(appdata*) user_data;
	builder_init(data);
	gtk_widget_set_size_request(data->win,320,240);
	gtk_application_add_window(data->app,GTK_WINDOW(data->win));

	page_body *page=new page_body(data, G_OBJECT(data->notebook));
	const gchar *text =
"<span foreground=\"blue\" size=\"x-large\">Blue text</span>" ;
GtkWidget *label = gtk_label_new (NULL);
gtk_label_set_markup (GTK_LABEL (label), text);
GtkWidget *image=gtk_image_new_from_file("opennet2.gif");
GtkWidget *image2=gtk_image_new_from_file("n_temp.png");

	page->add_widget(label,label_t,10,10);
	page->add_widget(image,image_t,20,20);
	page->add_widget(image2,image_t,40,40);

	gtk_widget_show_all(data->win);
}

void application_shutdown(const GtkApplication *restrict application, gpointer user_data)
{

}

int main (int argc, char *argv[])
{
	appdata data;
	gtk_init (&argc, &argv);
	gint res;
	data.app = gtk_application_new("gtk3.org", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(data.app, "activate", G_CALLBACK(application_activate), &data);
	g_signal_connect(data.app, "shutdown", G_CALLBACK(application_shutdown), &data);
	res = g_application_run(G_APPLICATION(data.app), 0, NULL);
return 0;
}
