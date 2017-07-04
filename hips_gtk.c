/**
 * Basic GUI for the H.I.P.S. steganography tool
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#define _GNU_SOURCE

char* get_file();
void on_error();
void execute(char* com, int where);


// global references
GtkWidget *window;
GtkEntry *pass_c;
GtkEntry *pass_e;
GtkEntry *text;
GtkEntry *image_c_in;
GtkEntry *image_c_out;
GtkEntry *image_e;
GtkTextBuffer *b_console_c;
GtkTextBuffer *b_console_e;
GtkTextView *scroll_view_c;
GtkTextView *scroll_view_e;
 
int main(int argc, char *argv[])
{
    GtkBuilder *builder; 
 
    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "hips.glade", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);

	pass_c = GTK_ENTRY(gtk_builder_get_object(builder, "password_c"));
	pass_e = GTK_ENTRY(gtk_builder_get_object(builder, "password_e"));
	image_c_in = GTK_ENTRY(gtk_builder_get_object(builder, "image_c_in"));
	image_c_out = GTK_ENTRY(gtk_builder_get_object(builder, "image_c_out"));
	image_e = GTK_ENTRY(gtk_builder_get_object(builder, "image_e"));
	text = GTK_ENTRY(gtk_builder_get_object(builder, "text_to_hide"));
	b_console_c = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "b_console_c"));
	b_console_e = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "b_console_e"));
	scroll_view_c = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "console_c"));
	scroll_view_e = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "console_e"));
    
    g_object_unref(builder);
 
    gtk_widget_show(window);                
    gtk_main();
	
    return 0;
}
 
// called when window is closed
void on_window1_destroy()
{
    gtk_main_quit();
}

// choose input file for hips_create
void on_button_image_c_in_clicked()
{
	char *filename = get_file();
	if (filename)
	{
		gtk_entry_set_text(image_c_in, filename);
		g_free (filename);
	}
}

// choose output file for hips_create
void on_button_image_c_out_clicked()
{
	char *filename = get_file();
	if (filename)
	{
		gtk_entry_set_text(image_c_out, filename);
		g_free (filename);
	}
}

// choose file for hips_extract
void on_button_image_e_clicked()
{
	char *filename = get_file();
	if (filename)
	{
		gtk_entry_set_text(image_e, filename);
		g_free (filename);
	}
}

// choose file
char* get_file()
{
    GtkWidget *dialog;
	char *filename = NULL;
	
	dialog = gtk_file_chooser_dialog_new ("Open File", NULL,
								GTK_FILE_CHOOSER_ACTION_OPEN,
								"_Cancel", GTK_RESPONSE_CANCEL,
								"_Open", GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
     
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		g_print("%s",filename);
		g_print("\n");
	}

	gtk_widget_destroy (dialog);
	return filename;
}

// check if all fields are filled (create) and execute
void check_fields_c()
{
	if (strlen(gtk_entry_get_text(pass_c)) == 0 || strlen(gtk_entry_get_text(image_c_in)) == 0 || strlen(gtk_entry_get_text(image_c_out)) == 0 || strlen(gtk_entry_get_text(text)) == 0)
		on_error();
	else
	{
		char *t;
		asprintf(&t, "./hips_c %s \"%s\" %s %s 2>&1", 
			gtk_entry_get_text(pass_c), gtk_entry_get_text(text), gtk_entry_get_text(image_c_in), gtk_entry_get_text(image_c_out));
		g_print("%s",t);
		g_print("\n");
		execute(t,0);
		free(t);
	}
}

// check if all fields are filled (extract) and execute
void check_fields_e()
{
	if (strlen(gtk_entry_get_text(pass_e)) == 0 || strlen(gtk_entry_get_text(image_e)) == 0)
		on_error();
	else
	{
		char *t;
		asprintf(&t, "./hips_e %s %s 2>&1", gtk_entry_get_text(pass_e), gtk_entry_get_text(image_e));
		g_print("%s",t);
		g_print("\n");
		execute(t,1);
		free(t);
	}
}

// simple error pop-up
void on_error()
{
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
								  GTK_DIALOG_DESTROY_WITH_PARENT,
								  GTK_MESSAGE_ERROR,
								  GTK_BUTTONS_CLOSE,
								  "All input fields are required!");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

// execute command
void execute(char* com, int where)
{
	FILE *in;
	extern FILE *popen();
	char buff[2048];

	if(!(in = popen(com, "r"))){
		exit(1);
	}

	while(fgets(buff, sizeof(buff), in)!=NULL){
		if (where == 0)
		{
			gtk_text_buffer_insert_at_cursor(b_console_c, buff, strlen(buff));
			gtk_text_view_scroll_to_mark(scroll_view_c, gtk_text_buffer_get_insert(b_console_c), 0.0, TRUE, 0.5, 0.5);
		}
		else
		{
			gtk_text_buffer_insert_at_cursor(b_console_e, buff, strlen(buff));
			gtk_text_view_scroll_to_mark(scroll_view_e, gtk_text_buffer_get_insert(b_console_e), 0.0, TRUE, 0.5, 0.5);
		}
	}
	if (where == 0)
	{
		gtk_text_buffer_insert_at_cursor(b_console_c, "\n\n", sizeof(char) * 2);
	}
	else
	{
		gtk_text_buffer_insert_at_cursor(b_console_e, "\n\n", sizeof(char) * 2);
	}

	g_print("Exit code: %i\n", WEXITSTATUS(pclose(in)));	
	//pclose(in);
}
