use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
#
#???DB.  Notes:
# setenv CMGUI_DEV_SOURCE /dosD/cmiss/cmgui/source
# To install locally, use
#   perl Makefile.PL PREFIX=/dosD/cmiss/cmgui/perl
#   make
#   make install
# To find local
#   setenv PERLLIB /dosD/cmiss/cmgui/perl/lib/perl5/site_perl/5.8.0/i686-linux/
# To test without installing
#   perl -Mblib -MCmiss::Function::Matrix::Sum -e 'Cmiss::Function::Matrix::Sum::create'
#

#Recursively look up from here for typemap files.
my $typemap_file = "typemap";
my @typemap_array = ();
while ( -f $typemap_file )
  {
	 #We want the typemap closest to us to have the highest precedence which
	 #is last in the list.
	 unshift @typemap_array, $typemap_file;
	 $typemap_file = "../$typemap_file";
  }
#Other typemaps we require
unshift @typemap_array, "$ENV{CMGUI_DEV_SOURCE}/perl/Cmiss/Value/typemap";

WriteMakefile(
    'NAME'    => 'Cmiss::Function::Matrix::Sum',
  #          
    'VERSION_FROM'  => 'Sum.pm', # finds $VERSION
    'PREREQ_PM'    => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Sum.pm', # retrieve abstract from module
       AUTHOR     => '"David Bullivant <d.bullivant@auckland.ac.nz>"') : ()),
    'LIBS'    => [''], # e.g., '-lm'
    'DEFINE'    => '-DFULL_NAMES', # e.g., '-DHAVE_SOMETHING'
  # Insert -I. if you add *.h files later:
    'INC'    => "-I$ENV{CMGUI_DEV_SOURCE}", # e.g., '-I/usr/include/other'
	# Turn off "Manifying" because can't create files with names containing :'s in
	#   a FAT32 filesystem
		'INSTALLMAN3DIR' => 'none',
  # Un-comment this if you add C files to link with later:
    # 'OBJECT'    => '$(O_FILES)', # link all the C files too
  # Include the typemap files of any directories above here
    'TYPEMAPS'    =>  \@typemap_array,
);
