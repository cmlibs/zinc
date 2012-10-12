if (!defined $path)
{
  $path = ".";
}

# debugged and tested as part of simple finite elasticity, but formulation not
#   quite right.  OK for testing

# Model 1 from ENGSCI 371 LAB 1 2003: 50% uniaxial extension of a unit cube in x
#   - YZ, X=0 face fixed (x=0); YZ, X=1 extended to x=1.5; XZ, Y=0 face fixed
#     (y=0); XY, Z=0 face fixed (z=0)
#   - from constant volume and homogeneous/isotropic
#     x=3/2*X
#     y=sqrt(2/3)*Y
#     z=sqrt(2/3)*Z
#   - deformation gradient tensor F=dx/DX
#     3/2 0         0
#     0   sqrt(2/3) 0
#     0   0         sqrt(2/3)
#   - right Cauchy-Green deformation tensor C=F^T*F
#     9/4 0   0
#     0   2/3 0
#     0   0   2/3
#   - first principal invariant tr(C)=43/12
#   - third principal invariant det(C)=1
#   - Green's strain tensor E=(C-I)/2
#     5/8 0    0
#     0   -1/6 0
#     0   0    -1/6

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Composition;
use Cmiss::Function::Derivative;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Gradient;
use Cmiss::Function::Integral;
use Cmiss::Function::Linear_span;
use Cmiss::Function::Matrix;
use Cmiss::Function::Matrix::Determinant;
use Cmiss::Function::Matrix::Dot_product;
use Cmiss::Function::Matrix::Product;
use Cmiss::Function::Matrix::Resize;
use Cmiss::Function::Matrix::Sum;
use Cmiss::Function::Matrix::Trace;
use Cmiss::Function::Matrix::Transpose;
use Cmiss::Function::Inverse;
use Cmiss::Function_variable::Composite;
use Cmiss::Function_variable::Exclusion;

# creating fieldml file from exnode and exelem
#use Cmiss::Cmgui_command_data;
#$cmgui_command_data = new Cmiss::Cmgui_command_data();
##$cmgui_command_data->execute_command("gfx read nodes $path/finite_elasticity/cube");
##$cmgui_command_data->execute_command("gfx read elements $path/finite_elasticity/cube");
##$cmgui_command_data->execute_command("gfx write region group cube cube");
#$cmgui_command_data->execute_command("gfx read region $path/finite_elasticity/cube.fml");
#$root=$cmgui_command_data->get_cmiss_root_region();
#$cube=$root->get_sub_region(name=>'cube');

# set up regions
$cube=new Cmiss::Region();
#$cube->read_file(name=>"$path/finite_elasticity/cube.fml");
$cube->read_file(name=>"$path/finite_elasticity_cube.fml");

# set up nodes
$node_1=$cube->get_node(name=>1);
$node_2=$cube->get_node(name=>2);
$node_3=$cube->get_node(name=>3);
$node_4=$cube->get_node(name=>4);
$node_5=$cube->get_node(name=>5);
$node_6=$cube->get_node(name=>6);
$node_7=$cube->get_node(name=>7);
$node_8=$cube->get_node(name=>8);

# set up undeformed coordinates variable
$undeformed=new Cmiss::Function::Finite_element(region=>$cube,name=>'undeformed');

# set up deformed coordinates variable
#???DB.  Should be able to duplicate undeformed fe_field (so get same
#  interpolation)
$deformed=new Cmiss::Function::Finite_element(region=>$cube,name=>'deformed');

# set up q test coordinates variable
#???DB.  Should be able to duplicate undeformed fe_field (so get same
#  interpolation)
$q_test=new Cmiss::Function::Finite_element(region=>$cube,name=>'q_test');

# set up pressure variable
#???DB.  Interpolation order needs to be 1 less than for undeformed fe_field
#???DB.  This means constant over each element for this example
#  - only element based fields are grid based and these need at least 2 points
#    in each direction (#xi>=1)
#  - could have grid-based with #xi=1 and then have all grid values come from
#    a scalar, but haven't implemented element_values for
#    Function::Finite_element
#  - use node based and have all nodal values come from same scalar
$pressure_linear=new Cmiss::Function::Finite_element(region=>$cube,name=>'pressure');
$pressure_element_1_function=new Cmiss::Function::Matrix(n_columns=>1,values=>[1]);
$pressure_element_1=$pressure_element_1_function->output();
$pressure_nodes_element_1=new Cmiss::Function_variable::Composite($pressure_element_1,$pressure_element_1,$pressure_element_1,$pressure_element_1,$pressure_element_1,$pressure_element_1,$pressure_element_1,$pressure_element_1);
$pressure_function=new Cmiss::Function::Composition(output=>$pressure_linear->output(),input=>$pressure_linear->nodal_values(),value=>$pressure_nodes_element_1);
$pressure=$pressure_function->output();

#???debug
$element_xi=new Cmiss::Function::Element_xi(element=>$cube->get_element(name=>1),xi=>[0.5,0.5,0.5]);
print "$element_xi\n";
$debug=$pressure_linear->output()->evaluate(input=>$pressure_linear->element_xi(),value=>$element_xi);
print "$debug\n";
$debug=$pressure->evaluate(input=>$pressure_linear->element_xi(),value=>$element_xi);
print "$debug\n";
$debug=$pressure->evaluate(input=>new Cmiss::Function_variable::Composite($pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite($element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "pressure=$debug\n";
print "\n";

# set up r test coordinates variable
#???DB.  Should be able to duplicate pressure fe_field (so get same
#  interpolation)
$r_test_linear=new Cmiss::Function::Finite_element(region=>$cube,name=>'r_test');
$r_test_element_1_function=new Cmiss::Function::Matrix(n_columns=>1,values=>[1]);
$r_test_element_1=$r_test_element_1_function->output();
$r_test_nodes_element_1=new Cmiss::Function_variable::Composite($r_test_element_1,$r_test_element_1,$r_test_element_1,$r_test_element_1,$r_test_element_1,$r_test_element_1,$r_test_element_1,$r_test_element_1);
$r_test_function=new Cmiss::Function::Composition(output=>$r_test_linear->output(),input=>$r_test_linear->nodal_values(),value=>$r_test_nodes_element_1);
$r_test=$r_test_function->output();
#???debug
$debug=$r_test_element_1->evaluate_derivative(independent=>[$r_test_element_1_function->input()]);
print "r_test derivative=$debug\n";
#???debug
$debug=$r_test_nodes_element_1->evaluate_derivative(independent=>[$r_test_element_1_function->input()]);
print "r_test derivative=$debug\n";
#???debug
$debug=$r_test_linear->output()->evaluate_derivative(independent=>[$r_test_linear->nodal_values()],input=>$r_test_linear->element_xi(),value=>$element_xi);
print "r_test derivative=$debug\n";
#???debug
$debug=$r_test->evaluate_derivative(independent=>[$r_test_element_1_function->input()],input=>$r_test_linear->element_xi(),value=>$element_xi);
print "r_test derivative=$debug\n";
print "\n";

# set displacement boundary conditions
$scalar_1=new Cmiss::Function::Matrix(n_columns=>1,values=>[0.0]);
$scalar_2=new Cmiss::Function::Matrix(n_columns=>1,values=>[1.5]);
# YZ, X=0, fixed (x=0)
$deformed->nodal_values(node=>$node_1,component_name=>'x')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_3,component_name=>'x')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_5,component_name=>'x')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_7,component_name=>'x')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_1,component_name=>'x')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_3,component_name=>'x')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_5,component_name=>'x')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_7,component_name=>'x')->set_value($scalar_1);
# YZ, X=1, extended to x=1.5
$deformed->nodal_values(node=>$node_2,component_name=>'x')->set_value($scalar_2);
$deformed->nodal_values(node=>$node_4,component_name=>'x')->set_value($scalar_2);
$deformed->nodal_values(node=>$node_6,component_name=>'x')->set_value($scalar_2);
$deformed->nodal_values(node=>$node_8,component_name=>'x')->set_value($scalar_2);
$q_test->nodal_values(node=>$node_2,component_name=>'x')->set_value($scalar_2);
$q_test->nodal_values(node=>$node_4,component_name=>'x')->set_value($scalar_2);
$q_test->nodal_values(node=>$node_6,component_name=>'x')->set_value($scalar_2);
$q_test->nodal_values(node=>$node_8,component_name=>'x')->set_value($scalar_2);
# XZ, Y=0, fixed (y=0)
$deformed->nodal_values(node=>$node_1,component_name=>'y')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_2,component_name=>'y')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_5,component_name=>'y')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_6,component_name=>'y')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_1,component_name=>'y')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_2,component_name=>'y')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_5,component_name=>'y')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_6,component_name=>'y')->set_value($scalar_1);
# XY, Z=0, fixed (z=0)
$deformed->nodal_values(node=>$node_1,component_name=>'z')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_2,component_name=>'z')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_3,component_name=>'z')->set_value($scalar_1);
$deformed->nodal_values(node=>$node_4,component_name=>'z')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_1,component_name=>'z')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_2,component_name=>'z')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_3,component_name=>'z')->set_value($scalar_1);
$q_test->nodal_values(node=>$node_4,component_name=>'z')->set_value($scalar_1);

#???debug
$debug=$undeformed->nodal_values()->evaluate();
print "$debug\n";
$debug=$deformed->nodal_values()->evaluate();
print "$debug\n";
$debug=$q_test->nodal_values()->evaluate();
print "$debug\n";
print "\n";

$deformed_fixed_values=new Cmiss::Function_variable::Composite($deformed->nodal_values(node=>$node_1,component_name=>'x'),$deformed->nodal_values(node=>$node_3,component_name=>'x'),$deformed->nodal_values(node=>$node_5,component_name=>'x'),$deformed->nodal_values(node=>$node_7,component_name=>'x'),$deformed->nodal_values(node=>$node_2,component_name=>'x'),$deformed->nodal_values(node=>$node_4,component_name=>'x'),$deformed->nodal_values(node=>$node_6,component_name=>'x'),$deformed->nodal_values(node=>$node_8,component_name=>'x'),$deformed->nodal_values(node=>$node_1,component_name=>'y'),$deformed->nodal_values(node=>$node_2,component_name=>'y'),$deformed->nodal_values(node=>$node_5,component_name=>'y'),$deformed->nodal_values(node=>$node_6,component_name=>'y'),$deformed->nodal_values(node=>$node_1,component_name=>'z'),$deformed->nodal_values(node=>$node_2,component_name=>'z'),$deformed->nodal_values(node=>$node_3,component_name=>'z'),$deformed->nodal_values(node=>$node_4,component_name=>'z'));
$q_test_fixed_values=new Cmiss::Function_variable::Composite($q_test->nodal_values(node=>$node_1,component_name=>'x'),$q_test->nodal_values(node=>$node_3,component_name=>'x'),$q_test->nodal_values(node=>$node_5,component_name=>'x'),$q_test->nodal_values(node=>$node_7,component_name=>'x'),$q_test->nodal_values(node=>$node_2,component_name=>'x'),$q_test->nodal_values(node=>$node_4,component_name=>'x'),$q_test->nodal_values(node=>$node_6,component_name=>'x'),$q_test->nodal_values(node=>$node_8,component_name=>'x'),$q_test->nodal_values(node=>$node_1,component_name=>'y'),$q_test->nodal_values(node=>$node_2,component_name=>'y'),$q_test->nodal_values(node=>$node_5,component_name=>'y'),$q_test->nodal_values(node=>$node_6,component_name=>'y'),$q_test->nodal_values(node=>$node_1,component_name=>'z'),$q_test->nodal_values(node=>$node_2,component_name=>'z'),$q_test->nodal_values(node=>$node_3,component_name=>'z'),$q_test->nodal_values(node=>$node_4,component_name=>'z'));

#???debug
$debug=$deformed_fixed_values->evaluate();
print "$debug\n";
$debug=$q_test_fixed_values->evaluate();
print "$debug\n";
print "\n";

$deformed_unknown_values=new Cmiss::Function_variable::Exclusion($deformed->nodal_values(),$deformed_fixed_values);
$q_test_unknown_values=new Cmiss::Function_variable::Exclusion($q_test->nodal_values(),$q_test_fixed_values);

#???debug
$debug=$deformed_unknown_values->evaluate();
print "deformed_unknown_values=$debug\n";
$debug=$q_test_unknown_values->evaluate();
print "q_test_unknown_values=$debug\n";
print "\n";

# identity tensor
$I=new Cmiss::Function::Matrix(n_columns=>3,values=>[1,0,0,0,1,0,0,0,1]);

$undeformed_inverse=new Cmiss::Function::Inverse(independent=>$undeformed->output(),dependent=>$undeformed->element_xi());
$deformed_inverse=new Cmiss::Function::Inverse(independent=>$deformed->output(),dependent=>$deformed->element_xi());

$maximum_iterations=new Cmiss::Function::Matrix(n_columns=>1,values=>[10]);
$tolerance=new Cmiss::Function::Matrix(n_columns=>1,values=>[0.000001]);
$undeformed_inverse->maximum_iterations()->set_value($maximum_iterations);
$undeformed_inverse->step_tolerance()->set_value($tolerance);
$undeformed_inverse->value_tolerance()->set_value($tolerance);
$undeformed_inverse->dependent_estimate()->set_value($element_xi);
$deformed_inverse->maximum_iterations()->set_value($maximum_iterations);
$deformed_inverse->step_tolerance()->set_value($tolerance);
$deformed_inverse->value_tolerance()->set_value($tolerance);
$deformed_inverse->dependent_estimate()->set_value($element_xi);

#???debug
$debug=$undeformed_inverse->output()->evaluate(input=>$undeformed_inverse->independent(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "$debug\n";
$debug=$deformed_inverse->output()->evaluate(input=>$deformed_inverse->independent(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "$debug\n";
print "\n";

$undeformed_to_deformed=new Cmiss::Function::Composition(output=>$deformed->output(),input=>$deformed->element_xi(),value=>$undeformed_inverse->output());

#???debug
$debug=$undeformed_to_deformed->output()->evaluate(input=>$undeformed_to_deformed->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "$debug\n";
print "\n";
#$derivative=($undeformed_to_deformed->output())->evaluate_derivative(independent=>[$deformed->element_xi()]);
#$debug=($deformed->element_xi())->evaluate();
#print "$debug\n";
#print "\n";

# deformation gradient tensor in terms of element/xi
$F=new Cmiss::Function::Derivative(dependent=>$undeformed_to_deformed->output(),independent=>[$undeformed_inverse->independent()]);
# deformation gradient tensor in terms of deformed coordinates
$F=new Cmiss::Function::Composition(output=>$F->output(),input=>$deformed->element_xi(),value=>$deformed_inverse->output());

#???debug
$debug=$F->output()->evaluate(input=>$F->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "F=$debug\n";
print "\n";

$q_test=new Cmiss::Function::Composition(output=>$q_test->output(),input=>$q_test->element_xi(),value=>$deformed_inverse->output());
# gradient of q_test function in terms of deformed coordinates
$deriv_q_test=new Cmiss::Function::Derivative(dependent=>$q_test->output(),independent=>[$deformed_inverse->input()]);

#???debug
$debug=$q_test->output()->evaluate(input=>$q_test->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "$debug\n";
$debug=$deriv_q_test->output()->evaluate(input=>$deriv_q_test->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "$debug\n";
print "\n";

# Green's deformation tensor
$E=new Cmiss::Function::Matrix(n_columns=>3,values=>[0,0,0,0,0,0,0,0,0]);
#???debug
print "$E $I\n";
$E_input=$E->input();
# Cauchy-Green deformation tensor.  C=I+2*E
#  ???DB.  Extend Product to have multiplier or multiplicand scalar?
$C=new Cmiss::Function::Matrix::Sum($I->output(),(new Cmiss::Function::Matrix::Product((new Cmiss::Function::Matrix(n_columns=>3,values=>[2,0,0,0,2,0,0,0,2]))->output(),$E->output()))->output());
#???debug
$debug=$C->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "$debug\n";
# trace(C) (first principal invariant)
$trace_C=new Cmiss::Function::Matrix::Trace($C->output());
#???debug
$debug=$trace_C->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "I1=$debug\n";
$debug=$trace_C->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "I1=$debug\n";
#???debug
$debug=$trace_C->output()->evaluate_derivative(independent=>[$E_input],input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "dI1/dE=$debug\n";
$debug=$trace_C->output()->evaluate_derivative(independent=>[$E_input],input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "dI1/dE=$debug\n";
# second principal invariant - I2=((tr(C))^2+tr(C^2))/2
$I2=new Cmiss::Function::Matrix::Product((new Cmiss::Function::Matrix(n_columns=>1,values=>[0.5]))->output(),(new Cmiss::Function::Matrix::Sum((new Cmiss::Function::Matrix::Product($trace_C->output(),$trace_C->output()))->output(),(new Cmiss::Function::Matrix::Trace((new Cmiss::Function::Matrix::Product($C->output(),$C->output()))->output()))->output()))->output());
#???debug
$debug=$I2->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "I2=$debug\n";
$debug=$I2->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "I2=$debug\n";
#???debug
$debug=$I2->output()->evaluate_derivative(independent=>[$E_input],input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "dI2/dE=$debug\n";
$debug=$I2->output()->evaluate_derivative(independent=>[$E_input],input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "dI2/dE=$debug\n";
# third principal invariant.  I3=det(C)
$I3=new Cmiss::Function::Matrix::Determinant($C->output());
#???debug
$debug=$I3->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "I3=$debug\n";
$debug=$I3->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "I3=$debug\n";
#???debug
$debug=$I3->output()->evaluate_derivative(independent=>[$E_input],input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "dI3/dE=$debug\n";
$debug=$I3->output()->evaluate_derivative(independent=>[$E_input],input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "dI3/dE=$debug\n";
# Mooney-Rivlin constants
$c1=new Cmiss::Function::Matrix(n_columns=>1,values=>[1]);
$c2=new Cmiss::Function::Matrix(n_columns=>1,values=>[0.2]);
# Mooney-Rivlin strain energy function = c1*(I1-3)+c2*(I2-3)-(pressure/2)*(I3-1)
$W=new Cmiss::Function::Matrix::Sum((new Cmiss::Function::Matrix::Sum((new Cmiss::Function::Matrix::Product($c1->output(),(new Cmiss::Function::Matrix::Sum($trace_C->output(),(new Cmiss::Function::Matrix(n_columns=>1,values=>[-3]))->output()))->output()))->output(),(new Cmiss::Function::Matrix::Product($c2->output(),(new Cmiss::Function::Matrix::Sum($I2->output(),(new Cmiss::Function::Matrix(n_columns=>1,values=>[-3]))->output()))->output()))->output()))->output(),(new Cmiss::Function::Matrix::Product((new Cmiss::Function::Matrix(n_columns=>1,values=>[-0.5]))->output(),(new Cmiss::Function::Matrix::Product($pressure,(new Cmiss::Function::Matrix::Sum($I3->output(),(new Cmiss::Function::Matrix(n_columns=>1,values=>[-1]))->output()))->output()))->output()))->output());
#???debug
$debug=$pressure->evaluate();
print "pressure=$debug\n";
$debug=$W->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "W=$debug\n";
$debug=$W->output()->evaluate(input=>new Cmiss::Function_variable::Composite($E_input,$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "W=$debug\n";
$debug=$W->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "W=$debug\n";
$debug=$W->output()->evaluate(input=>new Cmiss::Function_variable::Composite($E_input,$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "W=$debug\n";
# Cauchy stress tensor
$sigma=new Cmiss::Function::Derivative(dependent=>$W->output(),independent=>[$E_input]);
# need to resize sigma from 1x9 instead of 3x3
$sigma=new Cmiss::Function::Matrix::Resize(variable=>$sigma->output(),n_columns=>3);
$debug=$sigma->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]));
print "sigma=$debug\n";
$debug=$sigma->output()->evaluate(input=>new Cmiss::Function_variable::Composite($E_input,$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>3,values=>[1,7,-2,3,1,0,3,-4,-5]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "sigma=$debug\n";
$debug=$sigma->output()->evaluate(input=>$E_input,value=>new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]));
print "sigma=$debug\n";
$debug=$sigma->output()->evaluate(input=>new Cmiss::Function_variable::Composite($E_input,$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>3,values=>[0.625,0,0,0,0,0,0,0,0]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "sigma=$debug\n";

# Cauchy-Green deformation tensor.  C=(F^T)*F
$C=new Cmiss::Function::Matrix::Product((new Cmiss::Function::Matrix::Transpose($F->output()))->output(),$F->output());
#???debug
$debug=$C->output()->evaluate(input=>$F->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "C=$debug\n";
# Green's deformation tensor.  E=(C-I)/2
$E=new Cmiss::Function::Matrix::Product((new Cmiss::Function::Matrix(n_columns=>3,values=>[0.5,0,0,0,0.5,0,0,0,0.5]))->output(),(new Cmiss::Function::Matrix::Sum($C->output(),(new Cmiss::Function::Matrix(n_columns=>3,values=>[-1,0,0,0,-1,0,0,0,-1]))->output()))->output());
#???debug
$debug=$E->output()->evaluate(input=>$F->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
print "E=$debug\n";
# Cauchy stress tensor in terms of deformed coordinates
$sigma=new Cmiss::Function::Composition(output=>$sigma->output(),input=>$E_input,value=>$E->output());
$debug=$sigma->output()->evaluate(input=>new Cmiss::Function_variable::Composite($F->input(),$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "sigma=$debug\n";

# principle of virtual work integral
#???debug
$debug=$deriv_q_test->output()->evaluate(input=>new Cmiss::Function_variable::Composite($F->input(),$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "deriv_q_test=$debug\n";
$virtual_work_integrand=new Cmiss::Function::Matrix::Dot_product($deriv_q_test->output(),$sigma->output());
$debug=$virtual_work_integrand->output()->evaluate(input=>new Cmiss::Function_variable::Composite($F->input(),$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "integrand=$debug\n";
#$virtual_work=new Cmiss::Function::Integral(integrand=>new Cmiss::Function::Dot_product(new Cmiss::Function::Transpose($q_test_gradient),$sigma),independent=>$deformed,region=>$cube);
$virtual_work=new Cmiss::Function::Integral(integrand_output=>$virtual_work_integrand->output(),integrand_input=>$F->input(),independent_output=>$deformed->output(),independent_input=>$deformed->element_xi(),region=>$cube,quadrature_scheme=>"l.Lagrange*l.Lagrange*l.Lagrange");
#???debug
$debug=$virtual_work->output()->evaluate(input=>new Cmiss::Function_variable::Composite($pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite($element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "virtual_work=$debug\n";

$virtual_work_lhs=new Cmiss::Function::Linear_span(spanned=>$virtual_work->output(),spanning=>$q_test_unknown_values);
#???debug
$debug=$virtual_work_lhs->output()->evaluate(input=>$pressure_element_1_function->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[2]));
print "virtual_work_lhs=$debug\n";
$debug=$virtual_work->output()->evaluate(input=>new Cmiss::Function_variable::Composite($pressure_linear->element_xi(),$pressure_element_1_function->input(),$q_test_unknown_values),value=>new Cmiss::Function::Composite($element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2]),new Cmiss::Function::Matrix(n_columns=>1,values=>[1,0,0,0,0,0,0,0])));
print "virtual_work=$debug\n";

# incompressibility integral
$incompressible_integrand=new Cmiss::Function::Matrix::Product($r_test,(new Cmiss::Function::Matrix::Sum((new Cmiss::Function::Matrix::Determinant($C->output()))->output(),(new Cmiss::Function::Matrix(n_columns=>1,values=>[-1]))->output()))->output());
$debug=$incompressible_integrand->output()->evaluate(input=>new Cmiss::Function_variable::Composite($F->input(),$r_test_linear->element_xi(),$r_test_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
print "incompressible_integrand=$debug\n";
$incompressible=new Cmiss::Function::Integral(integrand_output=>$incompressible_integrand->output(),integrand_input=>$F->input(),independent_output=>$deformed->output(),independent_input=>$deformed->element_xi(),region=>$cube,quadrature_scheme=>"l.Lagrange*l.Lagrange*l.Lagrange");
#???debug
#$debug=$incompressible->output()->evaluate(input=>new Cmiss::Function_variable::Composite($pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite($element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
$debug=$incompressible->output()->evaluate(input=>new Cmiss::Function_variable::Composite($pressure_linear->element_xi(),$r_test_element_1_function->input()),value=>new Cmiss::Function::Composite($element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[1])));
print "incompressible=$debug\n";

$incompressible_lhs=new Cmiss::Function::Linear_span(spanned=>$incompressible->output(),spanning=>$r_test_element_1_function->input());
#???debug
$debug=$incompressible_lhs->output()->evaluate();
print "incompressible_lhs=$debug\n";

#$lhs=new Cmiss::Function::Composite($virtual_work_lhs,$incompressible_lhs);
$lhs=new Cmiss::Function_variable::Composite($virtual_work_lhs->output(),$incompressible_lhs->output());
$unknowns=new Cmiss::Function_variable::Composite($deformed_unknown_values,$pressure_element_1_function->input());

# solve $lhs=0 for $unknowns using Newton-Raphson minimization
#$solution=new Cmiss::Function::Inverse(independent=>$lhs->output(),dependent=>$unknowns);
$solution=new Cmiss::Function::Inverse(independent=>$lhs,dependent=>$unknowns);
$solution->maximum_iterations()->set_value($maximum_iterations);
$solution->step_tolerance()->set_value($tolerance);
$solution->value_tolerance()->set_value($tolerance);
#$solution->dependent_estimate()->set_value($element_xi);

#$unknown_values=$solution->output()->evaluate(input=>$solution->independent(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0]));
#print "solution=$unknown_values\n";
$unknown_values=$unknowns->evaluate();
#???debug
print "0) unknown_values=$unknown_values\n";
#$rhs=$lhs->output()->evaluate();
$rhs=$lhs->evaluate();
#???debug
print "0) rhs=$rhs\n";
$derivative=$lhs->evaluate_derivative(independent=>[$unknowns]);
#$derivative=$lhs->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($virtual_work_lhs->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($virtual_work->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($virtual_work->output())->evaluate_derivative(independent=>[$deformed_unknown_values]);
#$derivative=$virtual_work_integrand->output()->evaluate_derivative(independent=>[$unknowns],input=>new Cmiss::Function_variable::Composite($F->input(),$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
#???DB.  Able to calculate derivative from here
#$derivative=$sigma->output()->evaluate_derivative(independent=>[$unknowns],input=>new Cmiss::Function_variable::Composite($F->input(),$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
#$derivative=$E->output()->evaluate_derivative(independent=>[$unknowns],input=>new Cmiss::Function_variable::Composite($F->input(),$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
#$derivative=$C->output()->evaluate_derivative(independent=>[$unknowns],input=>new Cmiss::Function_variable::Composite($F->input(),$pressure_linear->element_xi(),$pressure_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
#$derivative=($incompressible_lhs->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($incompressible->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($incompressible_integrand->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($F->output())->evaluate_derivative(independent=>[$deformed->nodal_values(node=>$node_2,component_name=>'z')],input=>$F->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
#$derivative=(new Cmiss::Function::Derivative(dependent=>$undeformed_to_deformed->output(),independent=>[$undeformed_inverse->independent()]))->output()->evaluate_derivative(independent=>[$deformed->nodal_values(node=>$node_2,component_name=>'z')],input=>$undeformed_to_deformed->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
#$derivative=($undeformed_to_deformed->output())->evaluate_derivative(independent=>[$deformed->nodal_values(node=>$node_2,component_name=>'z')],input=>$undeformed_to_deformed->input(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]));
#$derivative=($incompressible_integrand->output())->evaluate_derivative(independent=>[$deformed->nodal_values(node=>$node_2,component_name=>'z')],input=>new Cmiss::Function_variable::Composite($F->input(),$r_test_linear->element_xi(),$r_test_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
#$derivative=($incompressible_integrand->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()],input=>new Cmiss::Function_variable::Composite($F->input(),$r_test_linear->element_xi(),$r_test_element_1_function->input()),value=>new Cmiss::Function::Composite(new Cmiss::Function::Matrix(n_columns=>1,values=>[0.3,0.6,0.2]),$element_xi,new Cmiss::Function::Matrix(n_columns=>1,values=>[2])));
#$derivative=($incompressible_integrand->output())->evaluate_derivative(independent=>[$deformed_unknown_values]);
#$derivative=($C->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($F->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$F=new Cmiss::Function::Derivative(dependent=>$undeformed_to_deformed->output(),independent=>[$undeformed_inverse->independent()]);
#$F=new Cmiss::Function::Composition(output=>$F->output(),input=>$deformed->element_xi(),value=>$deformed_inverse->output());
#$derivative=((new Cmiss::Function::Composition(output=>$undeformed_to_deformed->output(),input=>$deformed->element_xi(),value=>$deformed_inverse->output()))->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($undeformed_to_deformed->output())->evaluate_derivative(independent=>[$deformed->element_xi()]);
#$derivative=((new Cmiss::Function::Derivative(dependent=>$undeformed_to_deformed->output(),independent=>[$undeformed_inverse->independent()]))->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#$derivative=($undeformed_to_deformed->output())->evaluate_derivative(independent=>[$pressure_element_1_function->input()]);
#???debug
print "0) derivative=$derivative\n";
##$delta_unknown_values=$derivative->solve(rhs=>$rhs);
$delta_unknown_values=$derivative->solve($rhs);
print "0) delta_unknown_values=$delta_unknown_values\n";
#???debug
print "Where I'm up to\n";
#$unknown_values -= $delta_unknown_values;
##???debug
#print "1) unknown_values=$unknown_values\n";
#$lhs->set_value($unknowns,$unknown_values);
#$rhs=$lhs->evaluate();
##???debug
#print "1) rhs=$rhs\n";

##???DB.  Where I'm up to.  More than one step for NR and convergence criterion

##???DB.  Composition is being set up implicitly in many places/everywhere?
##???DB.  Need a result type method for variables?
##???DB.  Could have a trace method on Matrix to give another Variable?
##???DB.  Use templates and compile?

## Notes:
## ======
## principle of virtual work
## - at equilibrium, the work of the external forces on any virtual displacement
##   equals the work of the internal stresses
## - 0 = -(volume integral over cube of (grad(q)^T * sigma))
##   for any test function q.  NB
##   - no surface integral because in this example no part of the cube's
##     boundary sustains a load.
##   - no body forces or accelerations
##   - grad(q)=(dq/dx_i), deformed coordinates (x)
##   - sigma is in deformed configuration and per deformed area
##   - hyperelastic material ie. sigma_ij=dW/dE_ij
##   - W=(I1-3)+0.2*(I2-3)
##     - I1=tr(C)
##     - I2=((tr(C))^2+tr(C^2))/2
##   - C=I+2*E
##     - I1=2*tr(E)+3
##     - tr(C^2)=tr(4*E^2+4*E+I)=4*tr(E^2)+4*tr(E)+3
##     - I2=(4*tr(E)^2+4*tr(E)+9+4*tr(E^2)+4*tr(E)+3)/2
##         =2*tr(E^2)+2*(tr(E))^2+2*tr(E)+6
##     - E_ij=E_ik*E_kj
##     - tr(E^2)=E_1i*E_i1+E_2j*E_j2+E_3k*E_k3
##   - W_11=2+0.2*(4*E_11+2*(2*E_11+E_22+E_33)+2)
##         =2+0.2*(2*(C_11-1)+(2*(C_11-1)+C_22-1+C_33-1)+2)
##         =2+0.2*(4*C_11+C_22+C_33-4)
##   - W_12=0.2*2*E_21=0.2*C_21
##   - W_13=0.2*2*E_31=0.2*C_31
##   - W_21=0.2*2*E_12=0.2*C_12
##   - W_22=2+0.2*(4*E_22+2*(E_11+2*E_22+E_33)+2)
##         =2+0.2*(C_11+4*C_22+C_33-4)
##   - W_23=0.2*2*E_32=0.2*C_32
##   - W_31=0.2*2*E_13=0.2*C_13
##   - W_32=0.2*2*E_23=0.2*C_23
##   - W_33=2+0.2*(4*E_33+2*(E_11+E_22+2*E_33)+2)
##         =2+0.2*(C_11+C_22+4*C_33-4)
##   - F=(dxi/dXM), C=(F^T)F
