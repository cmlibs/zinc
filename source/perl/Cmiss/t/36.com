if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Derivative

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Derivative;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
# check creating derivative function
$fun_1a=new Cmiss::Function::Finite_element(region=>$heart,name=>'coordinates');
$var_1a=$fun_1a->xi();
$fun_1=new Cmiss::Function::Derivative(dependent=>$fun_1a->output(),independent=>[$var_1a]);
print "$fun_1\n";
print "\n";

# check evaluating with no arguments
$var_1=$fun_1->output();
$fun_2=$var_1->evaluate();
print "$var_1\n";
print "$fun_1\n";
# before adding Function_variable_derivative::evaluate, following printed
#
if (defined($fun_2)&&($fun_2))
{
	print "$fun_2\n";
} else
{
	print "undefined\n";
}
# make another function to be used as an value when evaluating
$fun_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
# check evaluating with arguments with the arguments being for the function
#   being evaluated.  Check that function being evaluated isn't changed
$fun_4=$var_1->evaluate(input=>$fun_1a->element_xi(),value=>$fun_3);
# before adding Function_variable_derivative::evaluate, following printed
# d(coordinates[*])/d(xi)(element=21 xi=[3](0.5,0.5,0.5))=[9]([1,1]((-0.0101274)),[1,1]((-0.0569796)),[1,1]((0.163236)),[1,1]((0)),[1,1]((0.959931)),[1,1]((0)),[1,1]((-0.340357)),[1,1]((0)),[1,1]((0)))
print "$fun_1($fun_3)=$fun_4\n";
$fun_4a=($fun_1a->output())->evaluate_derivative(independent=>[$var_1a],input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_4a\n";
$return_code=($fun_1a->element_xi())->set_value($fun_3);
$fun_4b=$var_1->evaluate();
# before adding Function_variable_derivative::evaluate, following printed
# 1 d(coordinates[*])/d(xi), [9]([1,1]((-0.0101274)),[1,1]((-0.0569796)),[1,1]((0.163236)),[1,1]((0)),[1,1]((0.959931)),[1,1]((0)),[1,1]((-0.340357)),[1,1]((0)),[1,1]((0)))
print "$return_code $fun_1, $fun_4b\n";
print "\n";

# check evaluating first derivative
$fun_6=$var_1->evaluate_derivative(independent=>[$var_1a]);
# before adding Function_variable_derivative::evaluate_derivative, following printed
# [9,3]((-0.0115665,-0.0446861,0.0127515),(-0.0446861,0.0930016,-0.039685),(0.0127515,-0.039685,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0))
print "$fun_6\n";
$fun_6a=($fun_1a->output())->evaluate_derivative(independent=>[$var_1a,$var_1a]);
print "$fun_6a\n";
$fun_7=new Cmiss::Function::Derivative(dependent=>$fun_1->output(),independent=>[$var_1a]);
$var_7=$fun_7->output();
$fun_7a=$var_7->evaluate();
# before adding Function_variable_derivative::evaluate, following printed
# [27]([1,1]((-0.0115665)),[1,1]((-0.0446861)),[1,1]((0.0127515)),[1,1]((-0.0446861)),[1,1]((0.0930016)),[1,1]((-0.039685)),[1,1]((0.0127515)),[1,1]((-0.039685)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)),[1,1]((0)))
print "$fun_7a\n";
# wrong independent, but caused a core dump because
#   Function_variable_derivative::equality_atomic and
#   Function_variable_iterator_representation_atomic_derivative::equality were
#   wrong
$fun_7b=$var_7->evaluate_derivative(independent=>[$var_1]);
# before adding Function_variable_derivative::evaluate_derivative, following printed
# [27,9]((0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0),(0,0,0,0,0,0,0,0,0))
print "$fun_7b\n";
$fun_7b=$var_7->evaluate_derivative(independent=>[$var_1a]);
# before adding Function_variable_derivative::evaluate_derivative, following printed
# [27,3]((0.251093,-0.0556088,0.0169152),(-0.0556088,-0.114432,0.053423),(0.0169152,0.053423,0),(-0.0556088,-0.114432,0.053423),(-0.114432,0.827843,0.135738),(0.053423,0.135738,0),(0.0169152,0.053423,0),(0.053423,0.135738,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,0,0))
print "$fun_7b\n";
$fun_7c=($fun_1a->output())->evaluate_derivative(independent=>[$var_1a,$var_1a,$var_1a]);
print "$fun_7c\n";
print "\n";

$var_1b=$fun_1a->nodal_values();
$fun_8=new Cmiss::Function::Derivative(dependent=>$fun_1a->output(),independent=>[$var_1b,$var_1a]);
$var_8=$fun_8->output();
$fun_8d=$var_8->evaluate();
# Node 28
$sub_matrix=$fun_8d->sub_matrix(column_low=>514,column_high=>558);
print "Node 28\n$sub_matrix\n";
$fun_8d_28=($fun_1a->output())->evaluate_derivative(independent=>[$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),$var_1a]);
print "$fun_8d_28\n";
# Node 24
$sub_matrix=$fun_8d->sub_matrix(column_low=>442,column_high=>459);
print "Node 24\n$sub_matrix\n";
$fun_8d_24=($fun_1a->output())->evaluate_derivative(independent=>[$fun_1a->nodal_values(node=>$heart->get_node(name=>24)),$var_1a]);
print "$fun_8d_24\n";
# Node 81
$sub_matrix=$fun_8d->sub_matrix(column_low=>1522,column_high=>1539);
print "Node 81\n$sub_matrix\n";
$fun_8d_81=($fun_1a->output())->evaluate_derivative(independent=>[$fun_1a->nodal_values(node=>$heart->get_node(name=>81)),$var_1a]);
print "$fun_8d_81\n";
# Node 13
$sub_matrix=$fun_8d->sub_matrix(column_low=>217,column_high=>261);
print "Node 13\n$sub_matrix\n";
$fun_8d_13=($fun_1a->output())->evaluate_derivative(independent=>[$fun_1a->nodal_values(node=>$heart->get_node(name=>13)),$var_1a]);
print "$fun_8d_13\n";
# Node 9
$sub_matrix=$fun_8d->sub_matrix(column_low=>145,column_high=>162);
print "Node 9\n$sub_matrix\n";
$fun_8d_9=($fun_1a->output())->evaluate_derivative(independent=>[$fun_1a->nodal_values(node=>$heart->get_node(name=>9)),$var_1a]);
print "$fun_8d_9\n";
# Node 51
$sub_matrix=$fun_8d->sub_matrix(column_low=>982,column_high=>999);
print "Node 51\n$sub_matrix\n";
$fun_8d_51=($fun_1a->output())->evaluate_derivative(independent=>[$fun_1a->nodal_values(node=>$heart->get_node(name=>51)),$var_1a]);
print "$fun_8d_51\n";
print "\n";

$fun_9a=new Cmiss::Function::Derivative(dependent=>$fun_1a->output(),independent=>[$var_1b]);
$fun_9=new Cmiss::Function::Derivative(dependent=>$fun_9a->output(),independent=>[$var_1a]);
$var_9=$fun_9->output();
$fun_9d=$var_9->evaluate();
# Node 28
#$sub_matrix=$fun_9d->sub_matrix(column_low=>514,column_high=>558);
#print "Node 28\n$sub_matrix\n";
print "Node 28\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>172,row_high=>186);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>793,row_high=>807);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>1414,row_high=>1428);
print "$sub_matrix\n";
# Node 24
#$sub_matrix=$fun_9d->sub_matrix(column_low=>442,column_high=>459);
#print "Node 24\n$sub_matrix\n";
print "Node 24\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>148,row_high=>153);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>769,row_high=>774);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>1390,row_high=>1395);
print "$sub_matrix\n";
# Node 81
#$sub_matrix=$fun_9d->sub_matrix(column_low=>1522,column_high=>1539);
#print "Node 81\n$sub_matrix\n";
print "Node 81\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>508,row_high=>513);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>1129,row_high=>1134);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>1750,row_high=>1755);
print "$sub_matrix\n";
# Node 13
#$sub_matrix=$fun_9d->sub_matrix(column_low=>217,column_high=>261);
#print "Node 13\n$sub_matrix\n";
print "Node 13\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>73,row_high=>87);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>694,row_high=>708);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>1315,row_high=>1329);
print "$sub_matrix\n";
# Node 9
#$sub_matrix=$fun_9d->sub_matrix(column_low=>145,column_high=>162);
#print "Node 9\n$sub_matrix\n";
print "Node 9\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>49,row_high=>54);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>670,row_high=>675);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>1291,row_high=>1296);
print "$sub_matrix\n";
# Node 51
#$sub_matrix=$fun_9d->sub_matrix(column_low=>982,column_high=>999);
#print "Node 51\n$sub_matrix\n";
print "Node 51\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>328,row_high=>333);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>949,row_high=>954);
print "$sub_matrix\n";
$sub_matrix=$fun_9d->sub_matrix(row_low=>1570,row_high=>1575);
print "$sub_matrix\n";
