if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Finite_element

use Cmiss::Cmgui_command_data;
use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;

# set up regions
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
# check creating finite element variable
$fun_1=new Cmiss::Function::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
print "$fun_1\n";
print "\n";

# check evaluating with no arguments
$var_1=$fun_1->output();
$fun_2=$var_1->evaluate();
print "$var_1\n";
print "$fun_1\n";
if (defined($fun_2)&&($fun_2))
{
	print "$fun_2\n";
} else
{
	print "undefined\n";
}
# make another variable to be used as an variable when evaluating
$fun_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$fun_4=$var_1->evaluate(input=>$fun_1->element_xi(),value=>$fun_3);
print "$fun_1($fun_3)=$fun_4\n";
$return_code=($fun_1->element_xi())->set_value($fun_3);
$fun_4a=$var_1->evaluate();
print "$return_code $fun_1 $fun_4a\n";
$fun_4b=$var_1->evaluate(input=>$fun_1->xi(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,1]));
print "$fun_1 $fun_4b\n";
print "\n";

# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type (Scalar, value is only defined for Scalar)
$fun_5a=new Cmiss::Function::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$fun_5=$var_1->evaluate(input=>$fun_5a->xi(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,1]));
print "$fun_1 $fun_5\n";
print "\n";

# check evaluating first derivative
$fun_6=$var_1->evaluate_derivative(independent=>[$fun_1->xi()]);
print "$fun_1 derivative $fun_6\n";
$fun_6a=$var_1->evaluate_derivative(independent=>[$fun_1->element_xi()]);
print "$fun_6a\n";
$fun_6b=$var_1->evaluate_derivative(independent=>[new Cmiss::Function_variable::Composite($fun_1->xi(index=>1),$fun_1->xi(index=>3))]);
print "$fun_6b\n";
$fun_6c=$var_1->evaluate_derivative(independent=>[$fun_5a->xi()]);
print "$fun_6c\n";
$fun_7=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values()]);
print "$fun_7\n";
print "\n";

# Because nodes 28 and 13 are on the central axis the modify (decreasing in xi1)
#   for theta uses the values at the outside nodes (9, 24, 51 and 81) - quick
#   fix for when don't have versions/calculation on axis.  This means that the
#   values at 28 and 13 aren't contributing and you could say that the third
#   rows of the derivative matrices for 28 and 13 should be zero.  However this
#   is a special point/discontinuity and should be ignored?
# Node 28
$sub_matrix=$fun_7->sub_matrix(column_low=>172,column_high=>186);
print "Node 28\n$sub_matrix\n";
$fun_7_28=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>28))]);
print "$fun_7_28\n";
# Node 24
$sub_matrix=$fun_7->sub_matrix(column_low=>148,column_high=>153);
print "Node 24\n$sub_matrix\n";
$fun_7_24=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>24))]);
print "$fun_7_24\n";
# Node 81
$sub_matrix=$fun_7->sub_matrix(column_low=>508,column_high=>513);
print "Node 81\n$sub_matrix\n";
$fun_7_81=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>81))]);
print "$fun_7_81\n";
# Node 13
$sub_matrix=$fun_7->sub_matrix(column_low=>73,column_high=>87);
print "Node 13\n$sub_matrix\n";
$fun_7_13=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>13))]);
print "$fun_7_13\n";
# Node 9
$sub_matrix=$fun_7->sub_matrix(column_low=>49,column_high=>54);
print "Node 9\n$sub_matrix\n";
$fun_7_9=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>9))]);
print "$fun_7_9\n";
# Node 51
$sub_matrix=$fun_7->sub_matrix(column_low=>328,column_high=>333);
print "Node 51\n$sub_matrix\n";
$fun_7_51=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>51))]);
print "$fun_7_51\n";
print "\n";

# check evaluating second derivative
$var_2=$fun_1->xi();
$fun_8=$var_1->evaluate_derivative(independent=>[$var_2,$var_2]);
print "$fun_8\n";
$var_2a=new Cmiss::Function_variable::Composite($fun_1->xi(index=>1),$fun_1->xi(index=>3));
#$fun_8a=$var_1->evaluate_derivative(independent=>[$var_2,$var_2a]);
$fun_8a=$var_1->evaluate_derivative(independent=>[$var_2a,$var_2]);
print "$fun_8a\n";
print "\n";

$var_2b=$fun_1->nodal_values();
$fun_8c=$var_1->evaluate_derivative(independent=>[$var_2,$var_2b]);
# Node 28
$sub_matrix=$fun_8c->sub_matrix(column_low=>172,column_high=>186);
print "Node 28, xi1\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>793,column_high=>807);
print "Node 28, xi2\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>1414,column_high=>1428);
print "Node 28, xi3\n$sub_matrix\n";
$fun_8c_28=$var_1->evaluate_derivative(independent=>[$var_2,$fun_1->nodal_values(node=>$heart->get_node(name=>28))]);
print "$fun_8c_28\n";
# Node 24
$sub_matrix=$fun_8c->sub_matrix(column_low=>148,column_high=>153);
print "Node 24, xi1\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>769,column_high=>774);
print "Node 24, xi2\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>1390,column_high=>1395);
print "Node 24, xi3\n$sub_matrix\n";
$fun_8c_24=$var_1->evaluate_derivative(independent=>[$var_2,$fun_1->nodal_values(node=>$heart->get_node(name=>24))]);
print "$fun_8c_24\n";
# Node 81
$sub_matrix=$fun_8c->sub_matrix(column_low=>508,column_high=>513);
print "Node 81, xi1\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>1129,column_high=>1134);
print "Node 81, xi2\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>1750,column_high=>1755);
print "Node 81, xi3\n$sub_matrix\n";
$fun_8c_81=$var_1->evaluate_derivative(independent=>[$var_2,$fun_1->nodal_values(node=>$heart->get_node(name=>81))]);
print "$fun_8c_81\n";
# Node 13
$sub_matrix=$fun_8c->sub_matrix(column_low=>73,column_high=>87);
print "Node 13, xi1\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>694,column_high=>708);
print "Node 13, xi2\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>1315,column_high=>1329);
print "Node 13, xi3\n$sub_matrix\n";
$fun_8c_13=$var_1->evaluate_derivative(independent=>[$var_2,$fun_1->nodal_values(node=>$heart->get_node(name=>13))]);
print "$fun_8c_13\n";
# Node 9
$sub_matrix=$fun_8c->sub_matrix(column_low=>49,column_high=>54);
print "Node 9, xi1\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>670,column_high=>675);
print "Node 9, xi2\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>1291,column_high=>1296);
print "Node 9, xi3\n$sub_matrix\n";
$fun_8c_9=$var_1->evaluate_derivative(independent=>[$var_2,$fun_1->nodal_values(node=>$heart->get_node(name=>9))]);
print "$fun_8c_9\n";
# Node 51
$sub_matrix=$fun_8c->sub_matrix(column_low=>328,column_high=>333);
print "Node 51, xi1\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>949,column_high=>954);
print "Node 51, xi2\n$sub_matrix\n";
$sub_matrix=$fun_8c->sub_matrix(column_low=>1570,column_high=>1575);
print "Node 51, xi3\n$sub_matrix\n";
$fun_8c_51=$var_1->evaluate_derivative(independent=>[$var_2,$fun_1->nodal_values(node=>$heart->get_node(name=>51))]);
print "$fun_8c_51\n";
print "\n";

$fun_8d=$var_1->evaluate_derivative(independent=>[$var_2b,$var_2]);
# Node 28
$sub_matrix=$fun_8d->sub_matrix(column_low=>514,column_high=>558);
print "Node 28\n$sub_matrix\n";
$fun_8d_28=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>28)),$var_2]);
print "$fun_8d_28\n";
# Node 24
$sub_matrix=$fun_8d->sub_matrix(column_low=>442,column_high=>459);
print "Node 24\n$sub_matrix\n";
$fun_8d_24=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>24)),$var_2]);
print "$fun_8d_24\n";
# Node 81
$sub_matrix=$fun_8d->sub_matrix(column_low=>1522,column_high=>1539);
print "Node 81\n$sub_matrix\n";
$fun_8d_81=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>81)),$var_2]);
print "$fun_8d_81\n";
# Node 13
$sub_matrix=$fun_8d->sub_matrix(column_low=>217,column_high=>261);
print "Node 13\n$sub_matrix\n";
$fun_8d_13=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>13)),$var_2]);
print "$fun_8d_13\n";
# Node 9
$sub_matrix=$fun_8d->sub_matrix(column_low=>145,column_high=>162);
print "Node 9\n$sub_matrix\n";
$fun_8d_9=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>9)),$var_2]);
print "$fun_8d_9\n";
# Node 51
$sub_matrix=$fun_8d->sub_matrix(column_low=>982,column_high=>999);
print "Node 51\n$sub_matrix\n";
$fun_8d_51=$var_1->evaluate_derivative(independent=>[$fun_1->nodal_values(node=>$heart->get_node(name=>51)),$var_2]);
print "$fun_8d_51\n";
print "\n";

# check evaluating fourth derivative
$fun_8e=$var_1->evaluate_derivative(independent=>[$var_2,$var_2,$var_2,$var_2]);
print "???DB.  Haven't checked the values for component 1 apart from \"diagonal\" -\n";
print "???  d^4/dxi1^4 (column 1), d^4/dxi2^4 (column 41), d^4/dxi3^4 (column 81) -\n";
print "???  which should be 0\n";
print "???DB.  If the entry is (i,j,k,l) where 1<=i,j,k,l<=3 then the column number\n";
print "???  for the entry is (((i-1)*3+(j-1))*3+(k-1))*3+l\n";
print "$fun_8e\n";
print "\n";

# check creating composite variable
$var_3=new Cmiss::Function_variable::Composite($var_2,$fun_3->xi(index=>2));
# check evaluating first derivative wrt composite
$fun_9=$var_1->evaluate_derivative(independent=>[$var_3]);
print "$fun_9\n";
$fun_10=$var_1->evaluate_derivative(independent=>[new Cmiss::Function_variable::Composite($fun_3->xi(index=>2),$fun_3->xi(index=>3),$var_2)]);
print "$fun_10\n";
print "\n";

# check getting and setting input values
$fun_11=($fun_1->xi())->evaluate();
print "$fun_11\n";
$fun_12=($fun_1->nodal_values(node=>$heart->get_node(name=>"13")))->evaluate();
print "$fun_12\n";
$return_code=($fun_1->nodal_values(node=>$heart->get_node(name=>"13")))->set_value(new Cmiss::Function::Matrix(n_columns=>1,values=>[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]));
$fun_13=($fun_1->nodal_values(node=>$heart->get_node(name=>"13")))->evaluate();
print "$return_code $fun_13\n";
#???DB.  Either need collapsing of Composites to Matrices or entry variables for
#  Composites.  Result should be
#1 [15](0.98448,0,0,0,-1,0.253073,0.593412,0.933751,1.27409,1.88932,2.50455,3.735,4.96546,5.58069,6.19592)
#$fun_12->set_value($fun_12->values(5),new Cmiss::Function::Vector(-1));
#$return_code=$fun_1->set_value($fun_1->nodal_values(node=>$heart->get_node(name=>"13")),$fun_12);
#$fun_14=($fun_1->nodal_values(node=>$heart->get_node(name=>"13")))->evaluate();
#print "$return_code $fun_14\n";
$return_code=($fun_1->nodal_values(node=>$heart->get_node(name=>"13"),type=>'value'))->set_value(new Cmiss::Function::Matrix(n_columns=>1,values=>[1,2,3,4,5,6,7,8,9,10,11,12]));
$fun_15=($fun_1->nodal_values(node=>$heart->get_node(name=>"13")))->evaluate();
print "$return_code $fun_15\n";
print "\n";

# check single nodal value
$fun_16=($fun_1->nodal_values(node=>$heart->get_node(name=>"5"),component_name=>"mu"))->evaluate();
print "$fun_16\n";
