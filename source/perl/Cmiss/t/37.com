if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Composition

use Cmiss::Cmgui_command_data;
use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Composition;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Matrix;
use Cmiss::Function::Derivative;
use Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian;
use Cmiss::Function_variable::Composite;

# set up regions
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
# check creating composition function
$fun_1a=new Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian();
$fun_1b=new Cmiss::Function::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$var_1b=$fun_1b->output();
$fun_1c=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$fun_1=new Cmiss::Function::Composition(output=>$fun_1a->output(),input=>$fun_1a->prolate(),value=>$var_1b);
$var_1=$fun_1->output();
print "$fun_1\n";
print "\n";

# check evaluating with no arguments
$fun_2=$var_1->evaluate();
print "$fun_2\n";
$fun_2e=$fun_1b->element_xi()->evaluate();
print "$fun_2e\n";
$fun_2b=$var_1b->evaluate(input=>$fun_1b->element_xi(),value=>$fun_1c);
print "$fun_2b\n";
#???debug.  Checking if input is set back properly.  No.  Needs looking at.  To
#  do with changing element dimension in evaluate.  Defining
#  CHANGE_ELEMENT_DIMENSION in computed_variable/function_variable.cpp works for
#  this case (setting back to no element), but not in general
$fun_2e=$fun_1b->element_xi()->evaluate();
print "$fun_2e\n";
$fun_3=$var_1->evaluate(input=>$fun_1b->element_xi(),value=>$fun_1c);
print "$fun_3\n";
$fun_3a=new Cmiss::Function::Composite($fun_1c,new Cmiss::Function::Matrix(n_columns=>1,values=>[35.25]));
$var_3a=new Cmiss::Function_variable::Composite($fun_1b->element_xi(),$fun_1a->focus());
$fun_3b=$var_1->evaluate(input=>$var_3a,value=>$fun_3a);
print "$fun_3b\n";
print "\n";

# check evaluating first derivative
#$fun_4=$var_1->evaluate_derivative(independent=>[$fun_1b->element_xi()],values=>[$fun_1b->element_xi(),$fun_1c,$fun_1a->focus(),new Cmiss::Function::Matrix(n_columns=>1,values=>[35.25])]);
$fun_4=$var_1->evaluate_derivative(independent=>[$fun_1b->element_xi()],input=>$var_3a,value=>$fun_3a);
print "$fun_4\n";
$fun_5=new Cmiss::Function::Derivative(dependent=>$var_1,independent=>[$fun_1b->element_xi()]);
$var_5=$fun_5->output();
$fun_6=$var_5->evaluate(input=>$var_3a,value=>$fun_3a);
print "$fun_6\n";
$return_code=($fun_1b->element_xi())->set_value($fun_1c);
$return_code=($fun_1a->focus())->set_value(new Cmiss::Function::Matrix(n_columns=>1,values=>[35.25]));
$fun_4a=$var_1->evaluate_derivative(independent=>[$fun_1b->element_xi()]);
print "$fun_4a\n";
$fun_6a=$var_5->evaluate();
print "$fun_6a\n";
$fun_4b=$var_1->evaluate_derivative(independent=>[$fun_1a->focus()]);
print "$fun_4b\n";
print "\n";

# check evaluating second derivative
$var_7a=$fun_1b->element_xi();
$var_7b=$fun_1a->focus();
$var_7c=new Cmiss::Function_variable::Composite($var_7a,$var_7b);
$fun_7=$var_1->evaluate_derivative(independent=>[$var_7a,$var_7a]);
print "$fun_7\n";
$fun_7a=$var_1->evaluate_derivative(independent=>[$var_7a,$var_7b]);
print "$fun_7a\n";
$fun_7b=$var_1->evaluate_derivative(independent=>[$var_7c,$var_7c]);
print "$fun_7b\n";
print "\n";

# check getting values (setting already done)
$fun_8=$var_7c->evaluate();
print "$fun_8\n";
