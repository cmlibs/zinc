use Cmiss::Value::Derivative_matrix;
use Cmiss::Value::FE_value;
use Cmiss::Value::FE_value_vector;
use Cmiss::Variable::Coordinates;
use Cmiss::Variable::Derivative;
use Cmiss::Variable::Spheroidal_coordinates_focus;
use Cmiss::Variable::Prolate_spheroidal_to_rectangular_cartesian;
Cmiss::Value::Matrix->set_string_convert_max_columns(100);
$prolate_spheroidal_coordinates_var=new Cmiss::Variable::Coordinates(name=>'prolate_spheroidal_coordinates',dimension=>3);
$spheroidal_coordinates_focus=new Cmiss::Variable::Spheroidal_coordinates_focus(name=>'spheroidal_coordinates_focus');
$prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Prolate_spheroidal_to_rectangular_cartesian(name=>'prolate_spheroidal_to_rectangular_cartesian');
$focus=new Cmiss::Value::FE_value(value=>35.25);
#$focus=new Cmiss::Value::FE_value(value=>1);
$prolate_spheroidal_coordinates=new Cmiss::Value::FE_value_vector(values=>[0.870552,0.479966,0.423234]);
$result=$prolate_spheroidal_to_rectangular_cartesian->evaluate($spheroidal_coordinates_focus,$focus,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates);
print "$result\n";
$d_prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Derivative(name=>"d_prolate_spheroidal_to_rectangular_cartesian",dependent=>$prolate_spheroidal_to_rectangular_cartesian,independent=>[$prolate_spheroidal_coordinates_var]);
$result=$d_prolate_spheroidal_to_rectangular_cartesian->evaluate($spheroidal_coordinates_focus,$focus,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates);
print "$result\n";
$d_prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Derivative(name=>"d_prolate_spheroidal_to_rectangular_cartesian",dependent=>$prolate_spheroidal_to_rectangular_cartesian,independent=>[$spheroidal_coordinates_focus]);
$result=$d_prolate_spheroidal_to_rectangular_cartesian->evaluate($spheroidal_coordinates_focus,$focus,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates);
print "$result\n";
$d_prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Derivative(name=>"d_prolate_spheroidal_to_rectangular_cartesian",dependent=>$prolate_spheroidal_to_rectangular_cartesian,independent=>[$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates_var]);
$result=$d_prolate_spheroidal_to_rectangular_cartesian->evaluate($spheroidal_coordinates_focus,$focus,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates);
print "$result\n";
$d_prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Derivative(name=>"d_prolate_spheroidal_to_rectangular_cartesian",dependent=>$prolate_spheroidal_to_rectangular_cartesian,independent=>[$prolate_spheroidal_coordinates_var,$spheroidal_coordinates_focus]);
$result=$d_prolate_spheroidal_to_rectangular_cartesian->evaluate($spheroidal_coordinates_focus,$focus,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates);
print "$result\n";
$d_prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Derivative(name=>"d_prolate_spheroidal_to_rectangular_cartesian",dependent=>$prolate_spheroidal_to_rectangular_cartesian,independent=>[$spheroidal_coordinates_focus,$spheroidal_coordinates_focus]);
$result=$d_prolate_spheroidal_to_rectangular_cartesian->evaluate($spheroidal_coordinates_focus,$focus,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates);
print "$result\n";
$d_prolate_spheroidal_to_rectangular_cartesian=new Cmiss::Variable::Derivative(name=>"d_prolate_spheroidal_to_rectangular_cartesian",dependent=>$prolate_spheroidal_to_rectangular_cartesian,independent=>[$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates_var]);
$result=$d_prolate_spheroidal_to_rectangular_cartesian->evaluate($spheroidal_coordinates_focus,$focus,$prolate_spheroidal_coordinates_var,$prolate_spheroidal_coordinates);
if ($result)
{
	print "$result\n";
}
$prolate_spheroidal_coordinates_var=0;
$spheroidal_coordinates_focus=0;
$prolate_spheroidal_to_rectangular_cartesian=0;
$focus=0;
$prolate_spheroidal_coordinates=0;