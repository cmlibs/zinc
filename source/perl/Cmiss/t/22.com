if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Derivative

use Cmiss::cmgui_command_data;
use Cmiss::Region;
use Cmiss::Variable_new;
use Cmiss::Variable_new::Derivative;
use Cmiss::Variable_new::Finite_element;
use Cmiss::Variable_new::Element_xi;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new::Matrix;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;

# set up regions
$cmgui_command_data = new Cmiss::cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
# check creating finite element variable
$var_1a=new Cmiss::Variable_new::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$input_1=$var_1a->input_xi();
$var_1=new Cmiss::Variable_new::Derivative(dependent=>$var_1a,independent=>[$input_1]);
print "$var_1\n";
# check evaluating with no arguments
$var_2=$var_1->evaluate();
print "$var_1, $var_2\n";
# make another variable to be used as an input when evaluating
$var_3=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$var_4=$var_1->evaluate($var_1a->input_element_xi(),$var_3);
print "$var_1($var_3)=$var_4\n";
$var_4a=$var_1a->evaluate_derivative(independent=>[$input_1],values=>[$var_1a->input_element_xi(),$var_3]);
print "$var_4a\n";
$return_code=$var_1a->set_input_value($var_1a->input_element_xi(),$var_3);
$var_4b=$var_1->evaluate();
print "$return_code $var_1, $var_4b\n";
print "\n";
# check evaluating first derivative
$var_6=$var_1->evaluate_derivative(independent=>[$input_1]);
print "$var_6\n";
$var_6a=$var_1a->evaluate_derivative(independent=>[$input_1,$input_1]);
print "$var_6a\n";
$var_7=new Cmiss::Variable_new::Derivative(dependent=>$var_1,independent=>[$input_1]);
$var_7a=$var_7->evaluate();
print "$var_7a\n";
$var_7b=$var_7->evaluate_derivative(independent=>[$input_1]);
print "$var_7b\n";
$var_7c=$var_1a->evaluate_derivative(independent=>[$input_1,$input_1,$input_1]);
print "$var_7c\n";
print "\n";
$input_1b=$var_1a->input_nodal_values();
$var_8=new Cmiss::Variable_new::Derivative(dependent=>$var_1,independent=>[$input_1b]);
$var_8d=$var_8->evaluate();
$matrix=$var_8d->matrix($input_1,$input_1b);
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>514,column_high=>558);
print "Node 28\n$sub_matrix\n";
$var_8d_28=$var_1a->evaluate_derivative(independent=>[$input_1,$var_1a->input_nodal_values(node=>$heart->get_node(name=>28))]);
print "$var_8d_28\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>442,column_high=>459);
print "Node 24\n$sub_matrix\n";
$var_8d_24=$var_1a->evaluate_derivative(independent=>[$input_1,$var_1a->input_nodal_values(node=>$heart->get_node(name=>24))]);
print "$var_8d_24\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>1522,column_high=>1539);
print "Node 81\n$sub_matrix\n";
$var_8d_81=$var_1a->evaluate_derivative(independent=>[$input_1,$var_1a->input_nodal_values(node=>$heart->get_node(name=>81))]);
print "$var_8d_81\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>217,column_high=>261);
print "Node 13\n$sub_matrix\n";
$var_8d_13=$var_1a->evaluate_derivative(independent=>[$input_1,$var_1a->input_nodal_values(node=>$heart->get_node(name=>13))]);
print "$var_8d_13\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>145,column_high=>162);
print "Node 9\n$sub_matrix\n";
$var_8d_9=$var_1a->evaluate_derivative(independent=>[$input_1,$var_1a->input_nodal_values(node=>$heart->get_node(name=>9))]);
print "$var_8d_9\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>982,column_high=>999);
print "Node 51\n$sub_matrix\n";
$var_8d_51=$var_1a->evaluate_derivative(independent=>[$input_1,$var_1a->input_nodal_values(node=>$heart->get_node(name=>51))]);
print "$var_8d_51\n";
print "\n";
$var_9=new Cmiss::Variable_new::Derivative(dependent=>$var_1a,independent=>[$input_1,$input_1b]);
$var_9d=$var_9->evaluate();
$matrix=$var_9d->matrix($input_1,$input_1b);
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>514,column_high=>558);
print "Node 28\n$sub_matrix\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>442,column_high=>459);
print "Node 24\n$sub_matrix\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>1522,column_high=>1539);
print "Node 81\n$sub_matrix\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>217,column_high=>261);
print "Node 13\n$sub_matrix\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>145,column_high=>162);
print "Node 9\n$sub_matrix\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>982,column_high=>999);
print "Node 51\n$sub_matrix\n";
