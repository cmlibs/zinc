package Cmiss::Field;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
use AutoLoader;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Field ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.01';


use Cmiss;
use Cmiss::Cmgui_command_data;
Cmiss::require_library('cmgui_finite_element');

package Cmiss::Field;

sub get_field_module_for_wrap
{
  for my $field ( @_)
  {
  	if ("Cmiss::Field" eq ref $field)
  	{
  		return $field->get_field_module();
  	}
  }
	croak "Can't get field Cmiss::Field_module from any object in @_";
}

sub wrap_numbers_in_field
{
  my ($field_module, $field) = @_;

  if ($field =~ m/^([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?$/)
  {
    $field = $field_module->create_constant([$field]);
  }
  elsif ("ARRAY" eq ref $field)
  {
    $field = $field_module->create_constant($field);
  }

  if ("Cmiss::Field" ne ref $field)
  {
	  croak "Can't create Cmiss::Field from $field";
  }

  return $field;
}

sub add
{
  my ($source_one, $source_two) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  $source_one = wrap_numbers_in_field($field_module, $source_one);
  $source_two = wrap_numbers_in_field($field_module, $source_two);

	return $field_module->create_add($source_one, $source_two);
}

sub subtract
{
  my ($source_one, $source_two, $swap) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  #If $swap is undef then this will be false so works for two parameters.
  if ($swap)
  {
	 ($source_one, $source_two) = ($source_two, $source_one);
  }
  $source_one = wrap_numbers_in_field($field_module, $source_one);
  $source_two = wrap_numbers_in_field($field_module, $source_two);

	return $field_module->create_subtract($source_one, $source_two);
}

sub multiply
{
  my ($source_one, $source_two) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  $source_one = wrap_numbers_in_field($field_module, $source_one);
  $source_two = wrap_numbers_in_field($field_module, $source_two);

 	return $field_module->create_multiply($source_one, $source_two);
}

sub divide
{
  my ($source_one, $source_two, $swap) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  #If $swap is undef then this will be false so works for two parameters.
  if ($swap)
  {
	 ($source_one, $source_two) = ($source_two, $source_one);
  }
  $source_one = wrap_numbers_in_field($field_module, $source_one);
  $source_two = wrap_numbers_in_field($field_module, $source_two);

	return $field_module->create_divide($source_one, $source_two);
}

sub less_than
{
  my ($source_one, $source_two, $swap) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  #If $swap is undef then this will be false so works for two parameters.
  if ($swap)
  {
	 ($source_one, $source_two) = ($source_two, $source_one);
  }
  $source_one = wrap_numbers_in_field($field_module, $source_one);
  $source_two = wrap_numbers_in_field($field_module, $source_two);

	return $field_module->create_less_than($source_one, $source_two);
}

sub greater_than
{
  my ($source_one, $source_two, $swap) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  #If $swap is undef then this will be false so works for two parameters.
  if ($swap)
  {
	 ($source_one, $source_two) = ($source_two, $source_one);
  }
  $source_one = wrap_numbers_in_field($field_module, $source_one);
  $source_two = wrap_numbers_in_field($field_module, $source_two);

	return $field_module->create_greater_than($source_one, $source_two);
}

sub sqrt
{
  my ($source) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  $source = wrap_numbers_in_field($field_module, $source);

	return $field_module->create_sqrt($source);
}

sub log
{
  my ($source) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  $source = wrap_numbers_in_field($field_module, $source);

	return $field_module->create_log($source);
}

sub exp
{
  my ($source) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  $source = wrap_numbers_in_field($field_module, $source);

	return $field_module->create_exp($source);
}

sub if
{
  my ($source_one, $source_two, $source_three) = @_;
	my $field_module = get_field_module_for_wrap(@_);

  $source_one = wrap_numbers_in_field($field_module, $source_one);
  $source_two = wrap_numbers_in_field($field_module, $source_two);
  $source_three = wrap_numbers_in_field($field_module, $source_three);

	return $field_module->create_if($source_one, $source_two, $source_three);
}

use overload
  '+' => 'add',
  '-' => 'subtract',
  '*' => 'multiply',
  '/' => 'divide',
  '<' => 'less_than',
  '>' => 'greater_than',
  'sqrt' => 'sqrt',
  'log' => 'log',
  'exp' => 'exp',
  fallback => 1;

require XSLoader;
XSLoader::load('Cmiss::Field', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Field - Perl extension for Cmiss regions

=head1 SYNOPSIS

  use Cmiss::Field;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Field.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Field, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.


=head2 EXPORT

None by default.



=head1 SEE ALSO

=head1 AUTHOR

Shane Blackett, <shane@blackett.co.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

