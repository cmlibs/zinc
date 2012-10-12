if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian

use Cmiss::Variable_new;
use Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;
use Cmiss::Variable_new::Composite;

# check creating prolate_spheroidal_to_rectangular_cartesian variable
$var_1=new Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian(0.870552,0.479966,0.423234,35.25);
print "$var_1\n";
# check evaluating
$var_2=$var_1->evaluate();
print "$var_2 $var_1\n";
# make another variable to be used as an input when evaluating
$var_3=new Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian();
print "$var_3 $var_1\n";
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$var_4=$var_1->evaluate($var_1->input_focus(),new Cmiss::Variable_new::Vector(1));
print "$var_4 $var_1\n";
# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type
#   (Prolate_spheroidal_to_rectangular_cartesian, input_focus is only defined
#   for Prolate_spheroidal_to_rectangular_cartesian)
$var_5=$var_1->evaluate($var_3->input_focus(),new Cmiss::Variable_new::Vector(1));
print "$var_5 $var_1\n";
# check evaluating first derivative
$var_6=$var_1->evaluate_derivative(independent=>[$var_1->input_prolate()]);
print "$var_6\n";
$var_6a=$var_1->evaluate_derivative(independent=>[$var_1->input_focus()]);
print "$var_6a\n";
$var_7=$var_1->evaluate_derivative(independent=>[$var_3->input_prolate()]);
print "$var_7\n";
# check evaluating second derivative
$var_8=$var_1->evaluate_derivative(independent=>[$var_1->input_prolate(),$var_1->input_prolate()]);
print "$var_8\n";
$var_8a=$var_1->evaluate_derivative(independent=>[$var_1->input_prolate(),$var_1->input_focus()]);
print "$var_8a\n";
$var_8b=$var_1->evaluate_derivative(independent=>[$var_1->input_focus(),$var_1->input_focus()]);
print "$var_8b\n";
# check creating composite input
$input_2=new Cmiss::Variable_new_input::Composite($var_1->input_prolate(),$var_1->input_focus());
# check evaluating first derivative wrt composite
$var_9=$var_1->evaluate_derivative(independent=>[$input_2]);
print "$var_9\n";
# check evaluating second derivative wrt composite
$var_10=$var_1->evaluate_derivative(independent=>[$input_2,$input_2]);
print "$var_10\n";
# check getting and setting input values
$var_11=$var_1->get_input_value($var_1->input_lambda());
print "$var_11\n";
$var_11a=$var_1->get_input_value($var_1->input_mu());
print "$var_11a\n";
$var_11b=$var_1->get_input_value($var_1->input_theta());
print "$var_11b\n";
$var_11c=$var_1->get_input_value($var_1->input_focus());
print "$var_11c\n";
$var_11d=$var_1->get_input_value($var_1->input_prolate());
print "$var_11d\n";
$var_11e=$var_1->get_input_value($input_2);
print "$var_11e\n";
#$return_code=$var_1->set_input_value($input_1,$var_3);
#print "$return_code $var_1\n";
#$var_12=$var_1->get_input_value($var_2->input_value());
#print "$var_12\n";
#$return_code=$var_1->set_input_value($var_2->input_value(),$var_2);
#print "$return_code $var_1\n";
