if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Inverse

use Cmiss::Region;
use Cmiss::Variable_new;
use Cmiss::Variable_new::Composition;
use Cmiss::Variable_new::Finite_element;
use Cmiss::Variable_new::Element_xi;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new::Matrix;
use Cmiss::Variable_new::Inverse;
use Cmiss::Variable_new::Scalar;
use Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian;
use Cmiss::Variable_new_input;
#use Cmiss::Variable_new_input::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
# check creating finite element variable
$var_1=new Cmiss::Variable_new::Finite_element(region=>$heart,name=>'coordinates');
# make variables to be used as an input when evaluating
$var_2=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$var_3=$var_1->evaluate($var_1->input_element_xi(),$var_2);
$var_4=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.1,0.4,0.7]);
$var_5=new Cmiss::Variable_new::Scalar(value=>0.000001);
$var_5a=new Cmiss::Variable_new::Scalar(value=>0.000001);
$var_6=new Cmiss::Variable_new::Scalar(value=>10);
$var_7=new Cmiss::Variable_new::Inverse(dependent=>$var_1->input_element_xi(),independent=>$var_1);
print "$var_7\n";
# check evaluating with arguments
$var_8=$var_7->evaluate($var_7->input_independent(),$var_3,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4);
print "$var_8\n";
print "\n";

# check evaluating first derivative
print "d(xi)/d(prolate)\n";
$var_9=$var_7->evaluate_derivative(independent=>[$var_7->input_independent()],values=>[$var_7->input_independent(),$var_3,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4]);
print "$var_9\n";
$var_10=$var_1->evaluate_derivative(independent=>[$var_1->input_element_xi()],values=>[$var_1->input_element_xi(),$var_2]);
print "$var_10\n";
$var_11=$var_10->matrix($var_1->input_element_xi());
$var_12=$var_11->solve(new Cmiss::Variable_new::Matrix(n_columns=>3,values=>[1,0,0,0,1,0,0,0,1]));
print "$var_12\n";
print "\n";

# check evaluating second derivative
print "d2(xi)/d(prolate)2\n";
$input_1=$var_7->input_independent();
$var_12=$var_7->evaluate_derivative(independent=>[$input_1,$input_1],values=>[$input_1,$var_3,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4]);
print "$var_12\n";
$var_13=$var_12->matrix($input_1);
print "$var_13\n";
print "\n";

# use composition to check inverse derivative
$var_14=new Cmiss::Variable_new::Composition(dependent=>$var_1,independent_source=>[$var_1->input_element_xi(),$var_7]);
$var_15=$var_14->evaluate($var_7->input_independent(),$var_3,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4);
print "$var_3 $var_15\n";
print "d(identity)/d(prolate)\n";
$var_16=$var_14->evaluate_derivative(independent=>[$var_7->input_independent()],values=>[$var_7->input_independent(),$var_3,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4]);
print "$var_16\n";
print "\n";

print "d2(identity)/d(prolate)2\n";
$var_17=$var_14->evaluate_derivative(independent=>[$input_1,$input_1],values=>[$input_1,$var_3,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4]);
print "$var_17\n";
$var_18=$var_17->matrix($input_1);
print "$var_18\n";
print "\n";

# a more complicated variable to invert
$var_21=new Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian();
$var_22=new Cmiss::Variable_new::Composition(dependent=>$var_21,independent_source=>[$var_21->input_prolate(),$var_1]);
$var_23=$var_22->evaluate($var_1->input_element_xi(),$var_2);
print "$var_23\n";
$var_24=new Cmiss::Variable_new::Inverse(dependent=>$var_1->input_element_xi(),independent=>$var_22);
print "$var_24\n";
# check evaluating with arguments
$var_25=$var_24->evaluate($var_24->input_independent(),$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4);
print "$var_25\n";
print "\n";

# check evaluating first derivative
print "d(xi)/d(rectangular)\n";
$var_26=$var_24->evaluate_derivative(independent=>[$var_24->input_independent()],values=>[$var_24->input_independent(),$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
print "$var_26\n";
$var_27=$var_22->evaluate_derivative(independent=>[$var_1->input_element_xi()],values=>[$var_1->input_element_xi(),$var_2]);
$var_28=$var_27->matrix($var_1->input_element_xi());
$var_29=$var_28->solve(new Cmiss::Variable_new::Matrix(n_columns=>3,values=>[1,0,0,0,1,0,0,0,1]));
print "$var_29\n";
print "\n";

# check evaluating second derivative
print "d2(xi)/d(rectangular)2\n";
$input_2=$var_24->input_independent();
$var_30=$var_24->evaluate_derivative(independent=>[$input_2,$input_2],values=>[$input_2,$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
print "$var_30\n";
$var_31=$var_30->matrix($input_2);
print "$var_31\n";
print "\n";

# use composition to check inverse derivative
$var_32=new Cmiss::Variable_new::Composition(dependent=>$var_22,independent_source=>[$var_1->input_element_xi(),$var_24]);
$var_33=$var_32->evaluate($var_24->input_independent(),$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4);
print "$var_23 $var_33\n";
print "d(identity)/d(rectangular)\n";
$var_34=$var_32->evaluate_derivative(independent=>[$var_24->input_independent()],values=>[$var_24->input_independent(),$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
print "$var_34\n";
print "\n";

print "d2(identity)/d(rectangular)2\n";
$var_35=$var_32->evaluate_derivative(independent=>[$input_2,$input_2],values=>[$input_2,$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
print "$var_35\n";
$var_36=$var_35->matrix($input_2);
print "$var_36\n";
print "\n";

# differentiate with respect to a variable other than input_independent
print "d(rectangular)/d(focus)\n";
$var_41=$var_22->evaluate_derivative(independent=>[$var_21->input_focus()],values=>[$var_1->input_element_xi(),$var_2]);
print "$var_41\n";
print "d(xi)/d(focus)\n";
$var_42=$var_24->evaluate_derivative(independent=>[$var_21->input_focus()],values=>[$input_2,$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
print "$var_42\n";
print "d(prolate)/d(focus)\n";
$var_42a=new Cmiss::Variable_new::Inverse(dependent=>$var_21->input_prolate(),independent=>$var_21);
$var_42b=$var_42a->evaluate_derivative(independent=>[$var_21->input_focus()],values=>[$var_42a->input_independent(),$var_23,$var_42a->input_value_tolerance(),$var_5,$var_42a->input_step_tolerance(),$var_5a,$var_42a->input_maximum_iterations(),$var_6,$var_42a->input_dependent_estimate(),$var_3]);
print "$var_42b\n";
print "  d(xi1)/d(focus) is 0 because d(theta)/d(focus) is zero for rectangular\n";
print "  to prolate and d(xi1)/d(lambda) and d(xi1)/d(mu) are zero for prolate\n";
print "  to element/xi\n";
print "\n";

print "d(identity)/d(focus)\n";
$var_43=$var_32->evaluate_derivative(independent=>[$var_21->input_focus()],values=>[$input_2,$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
print "$var_43\n";
print "\n";

print "d2(xi)/d(rectangular)d(focus)\n";
$var_44=$var_24->evaluate_derivative(independent=>[$input_2,$var_21->input_focus()],values=>[$input_2,$var_23,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
print "$var_44\n";
print "\n";

#???DB.  Still testing
## try identity the other way round
#$var_51=new Cmiss::Variable_new::Composition(dependent=>$var_24,independent_source=>[$var_24->input_independent(),$var_22]);
#$var_52=$var_51->evaluate($var_1->input_element_xi(),$var_2,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4);
#print "$var_2 $var_52\n";
#print "d(identity)/d(xi)\n";
#print "  ???DB.  Why is it -identity?\n";
#$var_53=$var_51->evaluate_derivative(independent=>[$var_1->input_xi()],values=>[$var_1->input_element_xi(),$var_2,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_4]);
#print "$var_53\n";
#$var_54=$var_51->evaluate($var_1->input_element_xi(),$var_4,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_2);
#print "$var_4 $var_54\n";
#$var_55=$var_51->evaluate_derivative(independent=>[$var_1->input_xi()],values=>[$var_1->input_element_xi(),$var_4,$var_24->input_value_tolerance(),$var_5,$var_24->input_step_tolerance(),$var_5a,$var_24->input_maximum_iterations(),$var_6,$var_24->input_dependent_estimate(),$var_2]);
#print "$var_55\n";
#print "\n";

## use composition to check inverse derivative
#$var_61=new Cmiss::Variable_new::Composition(dependent=>$var_7,independent_source=>[$var_7->input_independent(),$var_1]);
#$var_62=$var_61->evaluate($var_1->input_element_xi(),$var_2,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4);
#print "$var_2 $var_62\n";
#print "d(identity)/d(xi)\n";
#$var_63=$var_61->evaluate_derivative(independent=>[$var_1->input_element_xi()],values=>[$var_1->input_element_xi(),$var_2,$var_7->input_value_tolerance(),$var_5,$var_7->input_step_tolerance(),$var_5a,$var_7->input_maximum_iterations(),$var_6,$var_7->input_dependent_estimate(),$var_4]);
#print "$var_63\n";
#print "$var_9\n";
#print "$var_10\n";
#print "\n";

