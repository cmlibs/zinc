use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
#
#???DB.  Notes:
# setenv CMGUI_DEV_SOURCE /dosD/cmiss/cmgui-rc-region-1/source
# To install locally, use
#   perl Makefile.PL PREFIX=/dosD/cmiss/cmgui-rc-region-1/perl
#   make
#   make install
# To find local
#   setenv PERLLIB /dosD/cmiss/cmgui-rc-region-1/perl/lib/perl5/site_perl/5.8.0/i686-linux/
# To test without installing
#   perl -Mblib -MCmiss::Variable::Nodal_value -e '$bob=new Cmiss::Variable::Nodal_value'
#
# Problems with "Manifying" in a FAT32 filesystem because
#   Cmiss::Variable::Nodal_value.3 is not a valid file name
WriteMakefile(
    'NAME'    => 'Cmiss::Variable::Nodal_value',
  #          
    'VERSION_FROM'  => 'Nodal_value.pm', # finds $VERSION
    'PREREQ_PM'    => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Nodal_value.pm', # retrieve abstract from module
       AUTHOR     => '"David Bullivant <d.bullivant@auckland.ac.nz>"') : ()),
    'LIBS'    => [''], # e.g., '-lm'
    'DEFINE'    => '-DFULL_NAMES', # e.g., '-DHAVE_SOMETHING'
  # Insert -I. if you add *.h files later:
    'INC'    => "-I$ENV{CMGUI_DEV_SOURCE}", # e.g., '-I/usr/include/other'
  # Un-comment this if you add C files to link with later:
    'OBJECT'    => '$(O_FILES)', # link all the C files too
);
