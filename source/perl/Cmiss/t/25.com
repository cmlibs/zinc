if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Composition

use Cmiss::Cmgui_command_data;
use Cmiss::Region;
use Cmiss::Variable_new;
use Cmiss::Variable_new::Composite;
use Cmiss::Variable_new::Composition;
use Cmiss::Variable_new::Element_xi;
use Cmiss::Variable_new::Finite_element;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new::Derivative;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;

# set up regions
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
# check creating composition variable
$var_1a=new Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian();
$var_1b=new Cmiss::Variable_new::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$var_1c=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$var_1=new Cmiss::Variable_new::Composition(dependent=>$var_1a,independent_source=>[$var_1a->input_prolate(),$var_1b]);
print "$var_1\n";
# check evaluating with no arguments
$var_2=$var_1->evaluate();
print "$var_2\n";
$var_2b=$var_1b->evaluate($var_1b->input_element_xi(),$var_1c);
print "$var_2b\n";
$var_3=$var_1->evaluate($var_1b->input_element_xi(),$var_1c);
print "$var_3\n";
$var_3a=$var_1->evaluate($var_1b->input_element_xi(),$var_1c,$var_1a->input_focus(),new Cmiss::Variable_new::Vector(35.25));
print "$var_3a\n";
# check evaluating first derivative
$var_4=$var_1->evaluate_derivative(independent=>[$var_1b->input_element_xi()],values=>[$var_1b->input_element_xi(),$var_1c,$var_1a->input_focus(),new Cmiss::Variable_new::Vector(35.25)]);
print "$var_4\n";
$var_5=new Cmiss::Variable_new::Derivative(dependent=>$var_1,independent=>[$var_1b->input_element_xi()]);
$var_6=$var_5->evaluate($var_1b->input_element_xi(),$var_1c,$var_1a->input_focus(),new Cmiss::Variable_new::Vector(35.25));
print "$var_6\n";
$var_1->set_input_value($var_1b->input_element_xi(),$var_1c);
$var_1->set_input_value($var_1a->input_focus(),new Cmiss::Variable_new::Vector(35.25));
$var_4a=$var_1->evaluate_derivative(independent=>[$var_1b->input_element_xi()]);
print "$var_4a\n";
$var_6a=$var_5->evaluate();
print "$var_6a\n";
$var_4b=$var_1->evaluate_derivative(independent=>[$var_1a->input_focus()]);
print "$var_4b\n";
# check evaluating second derivative
$input_1=$var_1b->input_element_xi();
$input_2=$var_1a->input_focus();
$input_3=new Cmiss::Variable_new_input::Composite($input_1,$input_2);
$var_7=$var_1->evaluate_derivative(independent=>[$input_1,$input_1]);
print "$var_7\n";
$var_7a=$var_1->evaluate_derivative(independent=>[$input_1,$input_2]);
print "$var_7a\n";
$var_7b=$var_1->evaluate_derivative(independent=>[$input_3,$input_3]);
print "$var_7b\n";
# check getting input values (setting already done)
$var_8=$var_1->get_input_value($input_3);
print "$var_8\n";
