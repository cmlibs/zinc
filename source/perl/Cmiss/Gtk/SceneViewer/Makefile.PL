use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Cmiss::Gtk::SceneViewer',
    'VERSION_FROM'	=> 'SceneViewer.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
	 'PM'             => {'SceneViewer.pm' => '$(INST_LIBDIR)/SceneViewer.pm',
								 'GtkCmissSceneViewerTypes.pm' => '$(INST_LIBDIR)/Types.pm'},
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'SceneViewer.pm', # retrieve abstract from module
       AUTHOR     => '"Shane Blackett <s.blackett@auckland.ac.nz>"') : ()),
    'LIBS'		=> [''], # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
	# Insert -I. if you add *.h files later:
    'INC'		=> "-I$ENV{CMGUI_DEV_SOURCE} -I/usr/include/gtk-1.2/ -I/usr/include/glib-1.2/ -I/usr/lib/glib/include/ -I/usr/lib/perl5/site_perl/5.6.1/i386-linux/Gtk/Install/",
	# Un-comment this if you add C files to link with later:
    'OBJECT'		=> '$(O_FILES)', # link all the C files too
	 'TYPEMAPS'   => ['/usr/lib/perl5/5.6.1/ExtUtils/typemap',  '/home/blackett/gtk/Gtk-Perl-0.7008/GtkGLArea/../Gtk/blib/arch/Gtk/Install//GtkTypemap','/home/blackett/gtk/Gtk-Perl-0.7008/GtkGLArea/../Gtk/blib/arch/Gtk/Install//gtk-0.99.typemap','/home/blackett/gtk/Gtk-Perl-0.7008/GtkGLArea/../Gtk/blib/arch/Gtk/Install//gtk-1.2.typemap'],
	'depend' => 'GtkCmissSceneViewerDefs.c',
);
