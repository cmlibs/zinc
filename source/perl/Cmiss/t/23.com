if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Composite

use Cmiss::Cmgui_command_data;
use Cmiss::Region;
use Cmiss::Variable_new;
use Cmiss::Variable_new::Composite;
use Cmiss::Variable_new::Element_xi;
use Cmiss::Variable_new::Finite_element;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;

# set up regions
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
# check creating composite variable
$var_1a=new Cmiss::Variable_new::Vector(1,5,-3,8);
$var_1b=new Cmiss::Variable_new::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$var_1c=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$var_1=new Cmiss::Variable_new::Composite(variables=>[$var_1a,$var_1b,$var_1c]);
print "$var_1\n";
$var_2=new Cmiss::Variable_new::Composite(variables=>[$var_1a,$var_1b]);
print "$var_2\n";
# check evaluating with no arguments
$var_3=$var_1->evaluate();
print "$var_3 $var_1\n";
$var_3=$var_2->evaluate();
print "$var_3 $var_2\n";
# make another variable to be used as an input when evaluating
$var_3=new Cmiss::Variable_new::Vector(2,4);
print "$var_3 $var_1\n";
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$var_4=$var_1->evaluate($var_1b->input_element_xi(),$var_1c,$var_1a->input_values(2,4),$var_3);
print "$var_4 $var_1\n";
$var_4=$var_2->evaluate($var_1b->input_element_xi(),$var_1c,$var_1a->input_values(2,4),$var_3);
print "$var_4 $var_2\n";
# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type (Scalar, input_value is only defined for
#   Scalar)
$var_5=$var_1->evaluate($var_4->input_values(1,3),$var_3);
print "$var_5 $var_1\n";
# check evaluating first derivative
$var_6=$var_1->evaluate_derivative(independent=>[$var_1b->input_element_xi()],values=>[$var_1b->input_element_xi(),$var_1c]);
print "$var_6\n";
$var_6=$var_1->evaluate_derivative(independent=>[new Cmiss::Variable_new_input::Composite($var_1a->input_values(),$var_1b->input_element_xi(),$var_1c->input_element_xi())],values=>[$var_1b->input_element_xi(),$var_1c]);
print "$var_6\n";
# check getting and setting input values
$return_code=$var_1->set_input_value($var_1b->input_element_xi(),$var_1c);
$var_7=$var_1->evaluate();
print "$return_code $var_1, $var_7\n";
$var_8=$var_1->get_input_value($var_1a->input_values(1,3,4));
print "$var_8\n";
