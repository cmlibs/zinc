if (!defined $path)
{
  $path = ".";
}
use Cmiss::Region;
use Cmiss::Value::Derivative_matrix;
use Cmiss::Value::Element_xi;
use Cmiss::Value::FE_value;
use Cmiss::Value::FE_value_vector;
use Cmiss::Variable::Composite;
use Cmiss::Variable::Composition;
use Cmiss::Variable::Coordinates;
use Cmiss::Variable::Derivative;
use Cmiss::Variable::Element_xi;
use Cmiss::Variable::Finite_element;
use Cmiss::Variable::Identity;
use Cmiss::Variable::Spheroidal_coordinates_focus;
use Cmiss::Variable::Prolate_spheroidal_to_rectangular_cartesian;

Cmiss::Value::Matrix->set_string_convert_max_columns(100);
Cmiss::Value::Matrix->set_string_convert_max_rows(100);

$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
$focus=new Cmiss::Value::FE_value(value=>35.25);
#$focus=new Cmiss::Value::FE_value(value=>1);

$element_xi=new Cmiss::Value::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$element_xi_var=new Cmiss::Variable::Element_xi(name=>'element_xi_var',dimension=>3);

$prolate_spheroidal_var=new Cmiss::Variable::Finite_element(region=>$heart,name=>'coordinates');
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

$identity_var=new Cmiss::Variable::Identity(name=>'identity_var',variable=>$spheroidal_focus_var);
$result=$identity_var->evaluate($spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $identity_var\n";
}
$result=$identity_var->evaluate_derivative(independent=>[$spheroidal_focus_var],values=>[$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $identity_var derivative\n";
}

$composite_var=new Cmiss::Variable::Composite(name=>'composite_var',variables=>[$prolate_spheroidal_to_rectangular_cartesian_var,$identity_var]);
$result=$composite_var->evaluate($coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $composite_var\n";
}
$result=$composite_var->evaluate_derivative(independent=>[$coordinates_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $composite_var derivative\n";
}
$result=$composite_var->evaluate_derivative(independent=>[$spheroidal_focus_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $composite_var derivative\n";
}

$composite_var=new Cmiss::Variable::Composite(name=>'composite_var',variables=>[$coordinates_var,$element_xi_var,$spheroidal_focus_var]);
$result=$prolate_spheroidal_to_rectangular_cartesian_var->evaluate_derivative(independent=>[$composite_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_to_rectangular_cartesian_var derivative\n";
}
$f_derivative=$prolate_spheroidal_to_rectangular_cartesian_var->evaluate_derivative(independent=>[$composite_var,$composite_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($f_derivative)
{
	$matrix=$f_derivative->matrix(independent=>[$composite_var]);
	print "$matrix\n";
	print "$f_derivative\n";
} else
{
	print "Could not evaluate f_derivative\n";
}

$composite_var=new Cmiss::Variable::Composite(name=>'composite_var',variables=>[$prolate_spheroidal_var,new Cmiss::Variable::Identity(name=>'identity_element_xi_var',variable=>$element_xi_var),new Cmiss::Variable::Identity(name=>'identity_spheroidal_focus_var',variable=>$spheroidal_focus_var)]);
$result=$composite_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $composite_var\n";
}
$g_derivative=$composite_var->evaluate_derivative(independent=>[$element_xi_var,$spheroidal_focus_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
if ($g_derivative)
{
	$matrix=$g_derivative->matrix(independent=>[$element_xi_var]);
	print "$matrix\n";
	$matrix=$g_derivative->matrix(independent=>[$spheroidal_focus_var]);
	print "$matrix\n";
	print "$g_derivative\n";
} else
{
	print "Could not evaluate g_derivative\n";
}

$composite_var=new Cmiss::Variable::Composite(name=>'composite_var',variables=>[$coordinates_var,$element_xi_var,$element_xi_var]);
$result=$prolate_spheroidal_to_rectangular_cartesian_var->evaluate_derivative(independent=>[$composite_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $prolate_spheroidal_to_rectangular_cartesian_var derivative\n";
}
$f_derivative=$prolate_spheroidal_to_rectangular_cartesian_var->evaluate_derivative(independent=>[$composite_var,$composite_var],values=>[$coordinates_var,$prolate_spheroidal,$spheroidal_focus_var,$focus]);
if ($f_derivative)
{
	$matrix=$f_derivative->matrix(independent=>[$composite_var]);
	print "$matrix\n";
	print "$f_derivative\n";
} else
{
	print "Could not evaluate f_derivative\n";
}

$composite_var=new Cmiss::Variable::Composite(name=>'composite_var',variables=>[$prolate_spheroidal_var,new Cmiss::Variable::Identity(name=>'identity_element_xi_var',variable=>$element_xi_var),new Cmiss::Variable::Identity(name=>'identity_element_xi_var',variable=>$element_xi_var)]);
$result=$composite_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
if ($result)
{
	print "$result\n";
} else
{
	print "Could not evaluate $composite_var\n";
}
$g_derivative=$composite_var->evaluate_derivative(independent=>[$element_xi_var,$element_xi_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
if ($g_derivative)
{
	$matrix=$g_derivative->matrix(independent=>[$element_xi_var]);
	print "$matrix\n";
	print "$g_derivative\n";
} else
{
	print "Could not evaluate g_derivative\n";
}

#$rectangular_cartesian_var=new Cmiss::Variable::Composition(name=>'rectangular_cartesian_var',dependent=>$prolate_spheroidal_to_rectangular_cartesian_var,independent_source=>[$coordinates_var,$prolate_spheroidal_var]);
#$result=$rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
#if ($result)
#{
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $rectangular_cartesian_var\n";
#}
#$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$element_xi_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
#if ($result)
#{
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $rectangular_cartesian_var derivative\n";
#}
#$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$spheroidal_focus_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
#if ($result)
#{
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $rectangular_cartesian_var derivative\n";
#}
#$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$element_xi_var,$element_xi_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
#if ($result)
#{
#	$matrix=$result->matrix(independent=>[$element_xi_var]);
#	print "$matrix\n";
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $rectangular_cartesian_var derivative\n";
#}
#$result=$rectangular_cartesian_var->evaluate_derivative(independent=>[$element_xi_var,$spheroidal_focus_var],values=>[$element_xi_var,$element_xi,$spheroidal_focus_var,$focus]);
#if ($result)
#{
#	$matrix=$result->matrix(independent=>[$element_xi_var]);
#	print "$matrix\n";
#	$matrix=$result->matrix(independent=>[$spheroidal_focus_var]);
#	print "$matrix\n";
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $rectangular_cartesian_var derivative\n";
#}

#$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$element_xi_var]);
#$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
#if ($result)
#{
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $d_rectangular_cartesian_var\n";
#}

#$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$spheroidal_focus_var]);
#$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
#if ($result)
#{
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $d_rectangular_cartesian_var\n";
#}

#$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$element_xi_var,$element_xi_var]);
#$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
#if ($result)
#{
#	$matrix=$result->matrix(independent=>[$element_xi_var]);
#	print "$matrix\n";
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $d_rectangular_cartesian_var\n";
#}

#$d_rectangular_cartesian_var=new Cmiss::Variable::Derivative(name=>"d_rectangular_cartesian_var",dependent=>$rectangular_cartesian_var,independent=>[$element_xi_var,$spheroidal_focus_var]);
#$result=$d_rectangular_cartesian_var->evaluate($element_xi_var,$element_xi,$spheroidal_focus_var,$focus);
#if ($result)
#{
#	$matrix=$result->matrix(independent=>[$element_xi_var]);
#	print "$matrix\n";
#	$matrix=$result->matrix(independent=>[$spheroidal_focus_var]);
#	print "$matrix\n";
#	print "$result\n";
#} else
#{
#	print "Could not evaluate $d_rectangular_cartesian_var\n";
#}

$prolate_spheroidal_to_rectangular_cartesian_var=0;
$prolate_spheroidal_var=0;
$element_xi=0;
$heart=0;
