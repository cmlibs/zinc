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
$element_xi_var=new Cmiss::Variable::Element_xi(name=>'element_xi_var',dimension=>3);

$prolate_spheroidal_var=new Cmiss::Variable::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$result=$prolate_spheroidal_var->evaluate($element_xi_var,$element_xi);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_var\n";
}
$prolate_spheroidal=$result;
$result=$prolate_spheroidal_var->evaluate_derivative(independent=>[$element_xi_var],values=>[$element_xi_var,$element_xi]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_var derivative\n";
}
$result=$prolate_spheroidal_var->evaluate_derivative(independent=>[$element_xi_var,$element_xi_var],values=>[$element_xi_var,$element_xi]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_var derivative\n";
}

$coordinates_var=new Cmiss::Variable::Coordinates(name=>'coordinates_var',dimension=>3);
$spheroidal_focus_var=new Cmiss::Variable::Spheroidal_coordinates_focus(name=>'spheroidal_focus_var');
$prolate_spheroidal_to_rectangular_cartesian_var=new Cmiss::Variable::Prolate_spheroidal_to_rectangular_cartesian(name=>'prolate_spheroidal_to_rectangular_cartesian_var');
$result=$prolate_spheroidal_to_rectangular_cartesian_var->evaluate($coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_to_rectangular_cartesian_var\n";
}
$result=$prolate_spheroidal_to_rectangular_cartesian_var->evaluate_derivative(independent=>[$coordinates_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_to_rectangular_cartesian_var derivative\n";
}
$result=$prolate_spheroidal_to_rectangular_cartesian_var->evaluate_derivative(independent=>[$spheroidal_focus_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_to_rectangular_cartesian_var derivative\n";
}

$rectangular_cartesian_var=new Cmiss::Variable::Composition(name=>'rectangular_cartesian_var',dependent=>$prolate_spheroidal_to_rectangular_cartesian_var,independent_source=>[$coordinates_var,$prolate_spheroidal_var]);
$result=$rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $rectangular_cartesian_var\n";
}
$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$element_xi_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $rectangular_cartesian_var derivative\n";
}
$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$spheroidal_focus_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $rectangular_cartesian_var derivative\n";
}
$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$element_xi_var,$element_xi_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
if ($result)
{
	$matrix=$result->matrix(independent=>[$element_xi_var]);
	print "$matrix\n";
	print "$result\n";
} else
{
	print "Could not evaluate $rectangular_cartesian_var derivative\n";
}
$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$element_xi_var,$spheroidal_focus_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
if ($result)
{
	$matrix=$result->matrix(independent=>[$element_xi_var]);
	print "$matrix\n";
	$matrix=$result->matrix(independent=>[$spheroidal_focus_var]);
	print "$matrix\n";
	print "$result\n";
} else
{
	print "Could not evaluate $rectangular_cartesian_var derivative\n";
}

$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$element_xi_var]);
$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $d_rectangular_cartesian_var\n";
}

$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$spheroidal_focus_var]);
$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $d_rectangular_cartesian_var\n";
}

$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$element_xi_var,$element_xi_var]);
$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
if ($result)
{
	$matrix=$result->matrix(independent=>[$element_xi_var]);
	print "$matrix\n";
	print "$result\n";
} else
{
	print "Could not evaluate $d_rectangular_cartesian_var\n";
}

$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$element_xi_var,$spheroidal_focus_var]);
$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
if ($result)
{
	$matrix=$result->matrix(independent=>[$element_xi_var]);
	print "$matrix\n";
	$matrix=$result->matrix(independent=>[$spheroidal_focus_var]);
	print "$matrix\n";
	print "$result\n";
} else
{
	print "Could not evaluate $d_rectangular_cartesian_var\n";
}

$focus=new Cmiss::Value::FE_value(value=>1);
$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$element_xi_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $rectangular_cartesian_var derivative\n";
}

$prolate_spheroidal_to_rectangular_cartesian_var=0;
$prolate_spheroidal_var=0;
$element_xi=0;
$heart=0;
