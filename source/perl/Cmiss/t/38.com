if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Inverse

use Cmiss::Cmgui_command_data;
use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Composition;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Matrix;
use Cmiss::Function::Derivative;
use Cmiss::Function::Inverse;
use Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;

# set up regions
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
# check creating finite element function
$fun_1=new Cmiss::Function::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$var_1=$fun_1->output();
# make functions to be used as an input when evaluating
$fun_2=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$fun_3=$var_1->evaluate(input=>$fun_1->element_xi(),value=>$fun_2);
$fun_4=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.1,0.4,0.7]);
$fun_5=new Cmiss::Function::Matrix(n_columns=>1,values=>[0.000001]);
$fun_5a=new Cmiss::Function::Matrix(n_columns=>1,values=>[0.000001]);
$fun_6=new Cmiss::Function::Matrix(n_columns=>1,values=>[10]);
$fun_7=new Cmiss::Function::Inverse(dependent=>$fun_1->element_xi(),independent=>$var_1);
print "$fun_7\n";
$var_2=$fun_7->output();
print "$var_2\n";
# check evaluating with arguments
#???DB.  Order is important - dependent_estimate first so that element/xi
#  dimension is set
$fun_8a=new Cmiss::Function::Composite($fun_4,$fun_5,$fun_5a,$fun_6,$fun_3);
$var_8a=new Cmiss::Function_variable::Composite($fun_7->dependent_estimate(),$fun_7->value_tolerance(),$fun_7->step_tolerance(),$fun_7->maximum_iterations(),$fun_7->independent());
$fun_8=$var_2->evaluate(input=>$var_8a,value=>$fun_8a);
print "$fun_8\n";
print "\n";

# check evaluating first derivative
print "d(xi)/d(prolate)\n";
#$fun_9=$var_2->evaluate_derivative(independent=>[$fun_7->independent()],values=>[$fun_7->independent(),$fun_3,$fun_7->value_tolerance(),$fun_5,$fun_7->step_tolerance(),$fun_5a,$fun_7->maximum_iterations(),$fun_6,$fun_7->dependent_estimate(),$fun_4]);
$fun_9=$var_2->evaluate_derivative(independent=>[$fun_7->independent()],input=>$var_8a,value=>$fun_8a);
print "$fun_9\n";
#$fun_10=$fun_1->evaluate_derivative(independent=>[$fun_1->element_xi()],values=>[$fun_1->element_xi(),$fun_2]);
$fun_10=$var_1->evaluate_derivative(independent=>[$fun_1->element_xi()],input=>$fun_1->element_xi(),value=>$fun_2);
print "$fun_10\n";
$fun_11=$fun_10->solve(new Cmiss::Function::Matrix(n_columns=>3,values=>[1,0,0,0,1,0,0,0,1]));
print "$fun_11\n";
print "\n";

# check evaluating second derivative
print "d2(xi)/d(prolate)2\n";
$var_3=$fun_7->independent();
#$fun_12=$fun_7->evaluate_derivative(independent=>[$var_3,$var_3],values=>[$var_3,$fun_3,$fun_7->value_tolerance(),$fun_5,$fun_7->step_tolerance(),$fun_5a,$fun_7->maximum_iterations(),$fun_6,$fun_7->dependent_estimate(),$fun_4]);
$fun_12=$var_2->evaluate_derivative(independent=>[$var_3,$var_3],input=>$var_8a,value=>$fun_8a);
print "$fun_12\n";
#$fun_13=$fun_12->matrix($var_3);
#print "$fun_13\n";
print "\n";

# use composition to check inverse derivative
$fun_14=new Cmiss::Function::Composition(output=>$fun_1->output(),input=>$fun_1->element_xi(),value=>$fun_7->output());
$var_4=$fun_14->output();
#$fun_15=$fun_14->evaluate($fun_7->independent(),$fun_3,$fun_7->value_tolerance(),$fun_5,$fun_7->step_tolerance(),$fun_5a,$fun_7->maximum_iterations(),$fun_6,$fun_7->dependent_estimate(),$fun_4);
$fun_15=$var_4->evaluate(input=>$var_8a,value=>$fun_8a);
print "$fun_3 $fun_15\n";
print "d(identity)/d(prolate)\n";
#$fun_16=$fun_14->evaluate_derivative(independent=>[$fun_7->independent()],values=>[$fun_7->independent(),$fun_3,$fun_7->value_tolerance(),$fun_5,$fun_7->step_tolerance(),$fun_5a,$fun_7->maximum_iterations(),$fun_6,$fun_7->dependent_estimate(),$fun_4]);
$fun_16=$var_4->evaluate_derivative(independent=>[$fun_7->independent()],input=>$var_8a,value=>$fun_8a);
print "$fun_16\n";
print "\n";

print "d2(identity)/d(prolate)2\n";
#$fun_17=$fun_14->evaluate_derivative(independent=>[$var_3,$var_3],values=>[$var_3,$fun_3,$fun_7->value_tolerance(),$fun_5,$fun_7->step_tolerance(),$fun_5a,$fun_7->maximum_iterations(),$fun_6,$fun_7->dependent_estimate(),$fun_4]);
$fun_17=$var_4->evaluate_derivative(independent=>[$var_3,$var_3],input=>$var_8a,value=>$fun_8a);
print "$fun_17\n";
#$fun_18=$fun_17->matrix($var_3);
#print "$fun_18\n";
print "\n";

# a more complicated function to invert
$fun_21=new Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian();
#$fun_22=new Cmiss::Function::Composition(dependent=>$fun_21,independent_source=>[$fun_21->prolate(),$fun_1]);
$fun_22=new Cmiss::Function::Composition(output=>$fun_21->output(),input=>$fun_21->prolate(),value=>$fun_1->output());
$var_22=$fun_22->output();
$fun_23=$var_22->evaluate(input=>$fun_1->element_xi(),value=>$fun_2);
print "$fun_23\n";
#$fun_24=new Cmiss::Function::Inverse(dependent=>$fun_1->element_xi(),independent=>$fun_22);
$fun_24=new Cmiss::Function::Inverse(dependent=>$fun_1->element_xi(),independent=>$var_22);
print "$fun_24\n";
# check evaluating with arguments
$var_24=$fun_24->output();
#$fun_25=$fun_24->evaluate($fun_24->independent(),$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4);
$fun_25a=new Cmiss::Function::Composite($fun_4,$fun_5,$fun_5a,$fun_6,$fun_23);
$var_25a=new Cmiss::Function_variable::Composite($fun_24->dependent_estimate(),$fun_24->value_tolerance(),$fun_24->step_tolerance(),$fun_24->maximum_iterations(),$fun_24->independent());
$fun_25=$var_24->evaluate(input=>$var_25a,value=>$fun_25a);
print "$fun_25\n";
print "\n";

# check evaluating first derivative
print "d(xi)/d(rectangular)\n";
#$fun_26=$fun_24->evaluate_derivative(independent=>[$fun_24->independent()],values=>[$fun_24->independent(),$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_26=$var_24->evaluate_derivative(independent=>[$fun_24->independent()],input=>$var_25a,value=>$fun_25a);
print "$fun_26\n";
#$fun_27=$fun_22->evaluate_derivative(independent=>[$fun_1->element_xi()],values=>[$fun_1->element_xi(),$fun_2]);
$fun_27=$var_22->evaluate_derivative(independent=>[$fun_1->element_xi()],input=>$fun_1->element_xi(),value=>$fun_2);
#$fun_28=$fun_27->matrix($fun_1->element_xi());
#$fun_29=$fun_28->solve(new Cmiss::Function::Matrix(n_columns=>3,values=>[1,0,0,0,1,0,0,0,1]));
$fun_29=$fun_27->solve(new Cmiss::Function::Matrix(n_columns=>3,values=>[1,0,0,0,1,0,0,0,1]));
print "$fun_29\n";
print "\n";

# check evaluating second derivative
print "d2(xi)/d(rectangular)2\n";
$var_5=$fun_24->independent();
#$fun_30=$fun_24->evaluate_derivative(independent=>[$var_5,$var_5],values=>[$var_5,$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_30=$var_24->evaluate_derivative(independent=>[$var_5,$var_5],input=>$var_25a,value=>$fun_25a);
print "$fun_30\n";
#$fun_31=$fun_30->matrix($var_5);
#print "$fun_31\n";
print "\n";

# use composition to check inverse derivative
$fun_32=new Cmiss::Function::Composition(output=>$fun_22->output(),input=>$fun_1->element_xi(),value=>$fun_24->output());
$var_32=$fun_32->output();
#$fun_33=$fun_32->evaluate($fun_24->independent(),$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4);
$fun_33=$var_32->evaluate(input=>$var_25a,value=>$fun_25a);
print "$fun_23 $fun_33\n";
print "d(identity)/d(rectangular)\n";
#$fun_34=$fun_32->evaluate_derivative(independent=>[$fun_24->independent()],values=>[$fun_24->independent(),$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_34=$var_32->evaluate_derivative(independent=>[$fun_24->independent()],input=>$var_25a,value=>$fun_25a);
print "$fun_34\n";
print "\n";

print "d2(identity)/d(rectangular)2\n";
#$fun_35=$fun_32->evaluate_derivative(independent=>[$var_5,$var_5],values=>[$var_5,$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_35=$var_32->evaluate_derivative(independent=>[$var_5,$var_5],input=>$var_25a,value=>$fun_25a);
print "$fun_35\n";
#$fun_36=$fun_35->matrix($var_5);
#print "$fun_36\n";
print "\n";

# differentiate with respect to a function other than independent
print "d(rectangular)/d(focus)\n";
$fun_41=$var_22->evaluate_derivative(independent=>[$fun_21->focus()],input=>$fun_1->element_xi(),value=>$fun_2);
print "$fun_41\n";
print "d(xi)/d(focus)\n";
#$fun_42=$fun_24->evaluate_derivative(independent=>[$fun_21->focus()],values=>[$var_5,$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_42=$var_24->evaluate_derivative(independent=>[$fun_21->focus()],input=>$var_25a,value=>$fun_25a);
print "$fun_42\n";
print "d(prolate)/d(focus)\n";
$fun_42a=new Cmiss::Function::Inverse(dependent=>$fun_21->prolate(),independent=>$fun_21->output());
$var_42a=$fun_42a->output();
#$fun_42b=$fun_42a->evaluate_derivative(independent=>[$fun_21->focus()],values=>[$fun_42a->independent(),$fun_23,$fun_42a->value_tolerance(),$fun_5,$fun_42a->step_tolerance(),$fun_5a,$fun_42a->maximum_iterations(),$fun_6,$fun_42a->dependent_estimate(),$fun_3]);
$fun_42b=new Cmiss::Function::Composite($fun_3,$fun_5,$fun_5a,$fun_6,$fun_23);
$var_42c=new Cmiss::Function_variable::Composite($fun_42a->dependent_estimate(),$fun_42a->value_tolerance(),$fun_42a->step_tolerance(),$fun_42a->maximum_iterations(),$fun_42a->independent());
$fun_42b=$var_42a->evaluate_derivative(independent=>[$fun_21->focus()],input=>$var_42c,value=>$fun_42b);
print "$fun_42b\n";
print "  d(xi1)/d(focus) is 0 because d(theta)/d(focus) is zero for rectangular\n";
print "  to prolate and d(xi1)/d(lambda) and d(xi1)/d(mu) are zero for prolate\n";
print "  to element/xi\n";
print "\n";

print "d(identity)/d(focus)\n";
#$fun_43=$fun_32->evaluate_derivative(independent=>[$fun_21->focus()],values=>[$2,$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_43=$var_32->evaluate_derivative(independent=>[$fun_21->focus()],input=>$var_25a,value=>$fun_25a);
print "$fun_43\n";
print "\n";

print "d2(xi)/d(rectangular)d(focus)\n";
#$fun_44=$fun_24->evaluate_derivative(independent=>[$var_5,$fun_21->focus()],values=>[$var_5,$fun_23,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_44=$var_24->evaluate_derivative(independent=>[$var_5,$fun_21->focus()],input=>$var_25a,value=>$fun_25a);
print "$fun_44\n";
print "\n";

# try identity the other way round
$fun_51=new Cmiss::Function::Composition(output=>$fun_24->output(),input=>$fun_24->input(),value=>$fun_22->output());
$var_51=$fun_51->output();
#$fun_52=$fun_51->evaluate($fun_1->element_xi(),$fun_2,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4);
$fun_52a=new Cmiss::Function::Composite($fun_4,$fun_5,$fun_5a,$fun_6,$fun_2);
$var_52a=new Cmiss::Function_variable::Composite($fun_24->dependent_estimate(),$fun_24->value_tolerance(),$fun_24->step_tolerance(),$fun_24->maximum_iterations(),$fun_1->element_xi());
$fun_52=$var_51->evaluate(input=>$var_52a,value=>$fun_52a);
print "$fun_2 $fun_52\n";
print "d(identity)/d(xi)\n";
#$fun_53=$fun_51->evaluate_derivative(independent=>[$fun_1->xi()],values=>[$fun_1->element_xi(),$fun_2,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_4]);
$fun_53=$var_51->evaluate_derivative(independent=>[$fun_1->xi()],input=>$var_52a,value=>$fun_52a);
print "$fun_53\n";
$fun_54a=new Cmiss::Function::Composite($fun_2,$fun_5,$fun_5a,$fun_6,$fun_4);
$var_54a=new Cmiss::Function_variable::Composite($fun_24->dependent_estimate(),$fun_24->value_tolerance(),$fun_24->step_tolerance(),$fun_24->maximum_iterations(),$fun_1->element_xi());
#$fun_54=$fun_51->evaluate($fun_1->element_xi(),$fun_4,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_2);
$fun_54=$var_51->evaluate(input=>$var_54a,value=>$fun_54a);
print "$fun_4 $fun_54\n";
#$fun_55=$fun_51->evaluate_derivative(independent=>[$fun_1->xi()],values=>[$fun_1->element_xi(),$fun_4,$fun_24->value_tolerance(),$fun_5,$fun_24->step_tolerance(),$fun_5a,$fun_24->maximum_iterations(),$fun_6,$fun_24->dependent_estimate(),$fun_2]);
$fun_55=$var_51->evaluate_derivative(independent=>[$fun_1->xi()],input=>$var_54a,value=>$fun_54a);
print "$fun_55\n";
print "\n";

# use composition to check inverse derivative
$fun_61=new Cmiss::Function::Composition(output=>$fun_7->output(),input=>$fun_7->independent(),value=>$fun_1->output());
$var_61=$fun_51->output();
#$fun_62=$fun_61->evaluate($fun_1->element_xi(),$fun_2,$fun_7->value_tolerance(),$fun_5,$fun_7->step_tolerance(),$fun_5a,$fun_7->maximum_iterations(),$fun_6,$fun_7->dependent_estimate(),$fun_4);
$fun_62a=new Cmiss::Function::Composite($fun_4,$fun_5,$fun_5a,$fun_6,$fun_2);
$var_62a=new Cmiss::Function_variable::Composite($fun_7->dependent_estimate(),$fun_7->value_tolerance(),$fun_7->step_tolerance(),$fun_7->maximum_iterations(),$fun_1->element_xi());
$fun_62=$var_61->evaluate(input=>$var_62a,value=>$fun_62a);
print "$fun_2 $fun_62\n";
print "d(identity)/d(xi)\n";
#$fun_63=$fun_61->evaluate_derivative(independent=>[$fun_1->element_xi()],values=>[$fun_1->element_xi(),$fun_2,$fun_7->value_tolerance(),$fun_5,$fun_7->step_tolerance(),$fun_5a,$fun_7->maximum_iterations(),$fun_6,$fun_7->dependent_estimate(),$fun_4]);
$fun_63=$var_61->evaluate_derivative(independent=>[$fun_1->element_xi()],input=>$var_62a,value=>$fun_62a);
print "$fun_63\n";
print "\n";

