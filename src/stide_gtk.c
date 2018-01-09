/**
 * Basic GUI for the Stide steganography tool
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define _GNU_SOURCE
int asprintf(char **strp, const char *fmt, ...);

char *get_file();
void on_error();
void execute(char *com, int where);

/* global references */
GtkWidget *window;
GtkEntry *pass_c;
GtkEntry *pass_e;
GtkEntry *msg;
GtkEntry *img_c_in;
GtkEntry *img_c_out;
GtkEntry *img_e;
GtkEntry *db_c;
GtkEntry *db_e;
GtkTextBuffer *buff_c;
GtkTextBuffer *buff_e;
GtkTextView *term_c;
GtkTextView *term_e;

/* program parameters */
char *_img_c_out = "out.png";
char *_db_c = "stide.db";
char *_db_e = "stide.db";
char *_strict_c = "-s";
char *_strict_e = "-s";
char *_verbose_c = "-v";
char *_verbose_e = "-v";
char *_debug_c = " ";
char *_debug_e = " ";
char *_p = " ";


int main(int argc, char *argv[])
{
	GtkBuilder *builder;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "stide.glade", NULL);

	window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
	gtk_builder_connect_signals(builder, NULL);

	pass_c =
	    GTK_ENTRY(gtk_builder_get_object(builder, "pass_c"));
	pass_e =
	    GTK_ENTRY(gtk_builder_get_object(builder, "pass_e"));
	img_c_in =
	    GTK_ENTRY(gtk_builder_get_object(builder, "img_c_in"));
	img_c_out =
	    GTK_ENTRY(gtk_builder_get_object(builder, "img_c_out"));
	img_e =
	    GTK_ENTRY(gtk_builder_get_object(builder, "img_e"));
	db_c =
	    GTK_ENTRY(gtk_builder_get_object(builder, "db_c"));
	db_e =
	    GTK_ENTRY(gtk_builder_get_object(builder, "db_e"));
	msg =
	    GTK_ENTRY(gtk_builder_get_object(builder, "msg"));
	buff_c =
	    GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "buff_c"));
	buff_e =
	    GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "buff_e"));
	term_c =
	    GTK_TEXT_VIEW(gtk_builder_get_object(builder, "term_c"));
	term_e =
	    GTK_TEXT_VIEW(gtk_builder_get_object(builder, "term_e"));

	gtk_entry_set_text(img_c_out, _img_c_out);
	gtk_entry_set_text(db_c, _db_c);
	gtk_entry_set_text(db_e, _db_e);

	g_object_unref(builder);
	gtk_widget_show(window);
	gtk_main();

	return 0;
}

/* called when window is closed */
void on_window_destroy()
{
	gtk_main_quit();
}

/* switch 'strict' in -create- mode */
void strict_c_set(GtkComboBox *widget, gpointer data)
{
	const char *str = gtk_entry_get_text(data);
	_strict_c = (strcmp(str, "loose") == 0)
		? " " : "-s";
	g_print("...strict_c: '%s'\n", _strict_c);
}

/* switch 'strict' in -extract- mode */
void strict_e_set(GtkComboBox *widget, gpointer data)
{
	const char *str = gtk_entry_get_text(data);
	_strict_e = (strcmp(str, "loose") == 0)
		? " " : "-s";
	g_print("...strict_e: '%s'\n", _strict_e);
}

/* switch 'verbose'/'debug' -create- mode */
void verbose_c_set(GtkComboBox *widget, gpointer data)
{
	const char *str = gtk_entry_get_text(data);
	_verbose_c = (strcmp(str, "silent") == 0)
		? " " : (strcmp(str, "verbose") == 0)
			? "-v" : "-d";
	g_print("...verbose_c: '%s'\n", _verbose_c);
}

/* switch 'verbose'/'debug' -extract- mode */
void verbose_e_set(GtkComboBox *widget, gpointer data)
{
	const char *str = gtk_entry_get_text(data);
	_verbose_e = (strcmp(str, "silent") == 0)
		? " " : (strcmp(str, "verbose") == 0)
			? "-v" : "-d";
	g_print("...verbose_e: '%s'\n", _verbose_e);
}

/* switch 'print' */
void p_toggle(void) {
	_p = (strcmp(_p, " ") == 0)
		? "-p" : " ";
	g_print("...p: '%s'\n", _p);
}

/* choose input file for -create- mode */
void img_c_in_click()
{
	char *filename = get_file();
	if (filename) {
		gtk_entry_set_text(img_c_in, filename);
		g_free(filename);
	}
}

/* choose output file for -create- mode */
void img_c_out_click()
{
	char *filename = get_file();
	if (filename) {
		gtk_entry_set_text(img_c_out, filename);
		g_free(filename);
	}
}

/* choose file for -extract- mode */
void img_e_click()
{
	char *filename = get_file();
	if (filename) {
		gtk_entry_set_text(img_e, filename);
		g_free(filename);
	}
}

/* choose db for -create- mode */
void db_c_click()
{
	char *filename = get_file();
	if (filename) {
		gtk_entry_set_text(db_c, filename);
		g_free(filename);
	}
}

/* choose db for -extract- mode */
void db_e_click()
{
	char *filename = get_file();
	if (filename) {
		gtk_entry_set_text(db_e, filename);
		g_free(filename);
	}
}

/* choose file */
char *get_file()
{
	GtkWidget *dialog;
	char *filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Open File", NULL,
					     GTK_FILE_CHOOSER_ACTION_OPEN,
					     "_Cancel", GTK_RESPONSE_CANCEL,
					     "_Open", GTK_RESPONSE_ACCEPT,
					     NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
					    g_get_home_dir());

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		filename =
		    gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		g_print("%s", filename);
		g_print("\n");
	}

	gtk_widget_destroy(dialog);
	return filename;
}

/* check if all fields are filled (create) and execute */
void check_fields_c()
{
	if (strlen(gtk_entry_get_text(pass_c)) == 0
	    || strlen(gtk_entry_get_text(img_c_in)) == 0
	    || strlen(gtk_entry_get_text(msg)) == 0) {
		on_error();
		return;
	}
	char *t;              /* -s -p -v -d -f db  pass   msg   in out */
	asprintf(&t, "./stide -c %s %s %s %s -f %s \"%s\" \"%s\" %s %s 2>&1",
		_strict_c,
		_p,
		_verbose_c,
		_debug_c,
		gtk_entry_get_text(db_c),
		gtk_entry_get_text(pass_c),
		gtk_entry_get_text(msg),
		gtk_entry_get_text(img_c_in),
		gtk_entry_get_text(img_c_out));
	g_print("Calling stide with: %s", t);
	g_print("\n");
	execute(t, 0);
	free(t);
}

/* check if all fields are filled (extract) and execute */
void check_fields_e()
{
	if (strlen(gtk_entry_get_text(pass_e)) == 0
	    || strlen(gtk_entry_get_text(img_e)) == 0) {
		on_error();
		return;
	}
	char *t;              /* -s -v -d -f db  pass  in */
	asprintf(&t, "./stide -e %s %s %s -f %s \"%s\" %s 2>&1",
		_strict_e,
		_verbose_e,
		_debug_e,
		gtk_entry_get_text(db_e),
		gtk_entry_get_text(pass_e),
		gtk_entry_get_text(img_e));
	g_print("Calling stide with: %s", t);
	g_print("\n");
	execute(t, 0);
	free(t);
}

/* simple error pop-up */
void on_error()
{
	GtkWidget *dialog = gtk_message_dialog_new(NULL,
						   GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_MESSAGE_ERROR,
						   GTK_BUTTONS_CLOSE,
						   "All input fields are required!");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

/* execute command */
void execute(char *com, int where)
{
	FILE *in;
	extern FILE *popen();
	char buff[2048];

	if (!(in = popen(com, "r"))) {
		exit(1);
	}

	while (fgets(buff, sizeof(buff), in) != NULL) {
		if (where == 0) {
			gtk_text_buffer_insert_at_cursor(buff_c, buff,
							 strlen(buff));
			gtk_text_view_scroll_to_mark(term_c,
						     gtk_text_buffer_get_insert
						     (buff_c), 0.0, TRUE,
						     0.5, 0.5);
		} else {
			gtk_text_buffer_insert_at_cursor(buff_e, buff,
							 strlen(buff));
			gtk_text_view_scroll_to_mark(term_e,
						     gtk_text_buffer_get_insert
						     (buff_e), 0.0, TRUE,
						     0.5, 0.5);
		}
	}
	if (where == 0) {
		gtk_text_buffer_insert_at_cursor(buff_c, "\n\n",
						 sizeof(char) * 2);
	} else {
		gtk_text_buffer_insert_at_cursor(buff_e, "\n\n",
						 sizeof(char) * 2);
	}

	g_print("Exit code: %i\n", WEXITSTATUS(pclose(in)));
	//pclose(in);
}
