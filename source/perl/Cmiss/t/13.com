if (!defined $path)
{
  $path = ".";
}
use Cmiss::Region;
use Cmiss::Value::Derivative_matrix;
use Cmiss::Value::Element_xi;
use Cmiss::Value::FE_value;
use Cmiss::Value::FE_value_vector;
use Cmiss::Variable::Composition;
use Cmiss::Variable::Coordinates;
use Cmiss::Variable::Derivative;
use Cmiss::Variable::Element_xi;
use Cmiss::Variable::Finite_element;
use Cmiss::Variable::Spheroidal_coordinates_focus;
use Cmiss::Variable::Prolate_spheroidal_to_rectangular_cartesian;

Cmiss::Value::Matrix->set_string_convert_max_columns(100);
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
$focus=new Cmiss::Value::FE_value(value=>35.25);
#$focus=new Cmiss::Value::FE_value(value=>1);
$element_xi=new Cmiss::Value::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$element_xi_var=new Cmiss::Variable::Element_xi(name=>'element_xi_var');
$prolate_spheroidal_coordinates_var=new Cmiss::Variable::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$result=$prolate_spheroidal_coordinates_var->evaluate($element_xi_var,$element_xi);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate\n";
}
$coordinates=new Cmiss::Variable::Coordinates(name=>'coordinates',dimension=>3);
$spheroidal_coordinates_focus=new Cmiss::Variable::Spheroidal_coordinates_focus(name=>'spheroidal_coordinates_focus');
$prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Prolate_spheroidal_to_rectangular_cartesian(name=>'prolate_spheroidal_to_rectangular_cartesian');
$result=$prolate_spheroidal_to_rectangular_cartesian->evaluate($coordinates,$result,$spheroidal_coordinates_focus,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate\n";
}
$rectangular_cartesian_coordinates_var=new Cmiss::Variable::Composition(name=>'rectangular_cartesian_coordinates_var',dependent=>$prolate_spheroidal_to_rectangular_cartesian,independent_source=>[$coordinates,$prolate_spheroidal_coordinates_var]);
$result=$rectangular_cartesian_coordinates_var->evaluate($element_xi_var,$element_xi,$spheroidal_coordinates_focus,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate\n";
}
$prolate_spheroidal_to_rectangular_cartesian=0;
$prolate_spheroidal_coordinates_var=0;
$element_xi=0;
$heart=0;
