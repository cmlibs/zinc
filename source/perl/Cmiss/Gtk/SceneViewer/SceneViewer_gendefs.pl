
require '/home/blackett/gtk/Gtk-Perl-0.7008/tools/gendefs.pl';

@a = ('-f', 'GtkCmissSceneViewer',
		'-p', 'GtkCmiss=Cmiss::Gtk',
		'-p', 'Gtk=Gtk',
		'-p', 'GdkRgb=Gtk::Gdk::Rgb',
		'-p', 'Gdk=Gtk::Gdk',
		'-p', 'Gnome=Gnome',
		'-p', 'Panel=Gnome::Panel',
		'-m', 'Cmiss::Gtk::scene_viewer',
	   '-d', 'pkg.defs',
	   '-i', '"/usr/people/blackett/cmgui/source/gtk/gtk_cmiss_scene_viewer.h"');

gendefs::gendefs(@a);
