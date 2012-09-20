if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Finite_element

use Cmiss::Region;
use Cmiss::Variable_new;
use Cmiss::Variable_new::Finite_element;
use Cmiss::Variable_new::Element_xi;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new::Matrix;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
# check creating finite element variable
$var_1=new Cmiss::Variable_new::Finite_element(region=>$heart,name=>'coordinates');
print "$var_1\n";
# check evaluating with no arguments
$var_2=$var_1->evaluate();
print "$var_1, $var_2\n";
# make another variable to be used as an input when evaluating
$var_3=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$var_4=$var_1->evaluate($var_1->input_element_xi(),$var_3);
print "$var_1($var_3)=$var_4\n";
$var_4a=$var_1->evaluate();
print "$var_1, $var_4a\n";
$return_code=$var_1->set_input_value($var_1->input_element_xi(),$var_3);
$var_4b=$var_1->evaluate();
print "$return_code $var_1, $var_4b\n";
$var_4c=$var_1->evaluate($var_1->input_xi(),new Cmiss::Variable_new::Vector(0,0,1));
print "$var_1, $var_4c\n";
print "\n";
# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type (Scalar, input_value is only defined for
#   Scalar)
$var_5a=new Cmiss::Variable_new::Finite_element(region=>$heart,name=>'coordinates');
$var_5=$var_1->evaluate($var_5a->input_xi(),new Cmiss::Variable_new::Vector(0,0,1));
print "$var_1, $var_5\n";
# check evaluating first derivative
$var_6=$var_1->evaluate_derivative(independent=>[$var_1->input_xi()]);
print "$var_1 derivative $var_6\n";
$var_6a=$var_1->evaluate_derivative(independent=>[$var_1->input_element_xi()]);
print "$var_6a\n";
$var_6b=$var_1->evaluate_derivative(independent=>[$var_1->input_xi(1,3)]);
print "$var_6b\n";
$var_6c=$var_1->evaluate_derivative(independent=>[$var_5a->input_xi()]);
print "$var_6c\n";
$var_7=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values()]);
print "$var_7\n";
print "\n";
$matrix=$var_7->matrix($var_1->input_nodal_values());
# Because nodes 28 and 13 are on the central axis the modify (decreasing in xi1)
#   for theta uses the values at the outside nodes (9, 24, 51 and 81) - quick
#   fix for when don't have versions/calculation on axis.  This means that the
#   values at 28 and 13 aren't contributing and you could say that the third
#   rows of the derivative matrices for 28 and 13 should be zero.  However this
#   is a special point/discontinuity and should be ignored?
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>172,column_high=>186);
print "Node 28\n$sub_matrix\n";
$var_7_28=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>28))]);
print "$var_7_28\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>148,column_high=>153);
print "Node 24\n$sub_matrix\n";
$var_7_24=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>24))]);
print "$var_7_24\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>508,column_high=>513);
print "Node 81\n$sub_matrix\n";
$var_7_81=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>81))]);
print "$var_7_81\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>73,column_high=>87);
print "Node 13\n$sub_matrix\n";
$var_7_13=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>13))]);
print "$var_7_13\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>49,column_high=>54);
print "Node 9\n$sub_matrix\n";
$var_7_9=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>9))]);
print "$var_7_9\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>328,column_high=>333);
print "Node 51\n$sub_matrix\n";
$var_7_51=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>51))]);
print "$var_7_51\n";
print "\n";
# check evaluating second derivative
$input_1=$var_1->input_xi();
$var_8=$var_1->evaluate_derivative(independent=>[$input_1,$input_1]);
print "$var_8\n";
$input_1a=$var_1->input_xi(1,3);
$var_8a=$var_1->evaluate_derivative(independent=>[$input_1,$input_1a]);
print "$var_8a\n";
$var_8b=$var_8a->matrix($input_1a);
print "$var_8b\n";
$input_1b=$var_1->input_nodal_values();
$var_8c=$var_1->evaluate_derivative(independent=>[$input_1b,$input_1]);
$matrix=$var_8c->matrix($input_1);
print "$matrix\n";
$matrix=$var_8c->matrix($input_1b);
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>172,column_high=>186);
print "Node 28\n$sub_matrix\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>148,column_high=>153);
print "Node 24\n$sub_matrix\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>508,column_high=>513);
print "Node 81\n$sub_matrix\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>73,column_high=>87);
print "Node 13\n$sub_matrix\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>49,column_high=>54);
print "Node 9\n$sub_matrix\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>328,column_high=>333);
print "Node 51\n$sub_matrix\n";
$matrix=$var_8c->matrix($input_1b,$input_1);
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>172,column_high=>186);
print "Node 28, xi1\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>793,column_high=>807);
print "Node 28, xi2\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1414,column_high=>1428);
print "Node 28, xi3\n$sub_matrix\n";
$var_8c_28=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>28)),$input_1]);
print "$var_8c_28\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>148,column_high=>153);
print "Node 24, xi1\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>769,column_high=>774);
print "Node 24, xi2\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1390,column_high=>1395);
print "Node 24, xi3\n$sub_matrix\n";
$var_8c_24=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>24)),$input_1]);
print "$var_8c_24\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>508,column_high=>513);
print "Node 81, xi1\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1129,column_high=>1134);
print "Node 81, xi2\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1750,column_high=>1755);
print "Node 81, xi3\n$sub_matrix\n";
$var_8c_81=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>81)),$input_1]);
print "$var_8c_81\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>73,column_high=>87);
print "Node 13, xi1\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>694,column_high=>708);
print "Node 13, xi2\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1315,column_high=>1329);
print "Node 13, xi3\n$sub_matrix\n";
$var_8c_13=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>13)),$input_1]);
print "$var_8c_13\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>49,column_high=>54);
print "Node 9, xi1\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>670,column_high=>675);
print "Node 9, xi2\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1291,column_high=>1296);
print "Node 9, xi3\n$sub_matrix\n";
$var_8c_9=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>9)),$input_1]);
print "$var_8c_9\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>328,column_high=>333);
print "Node 51, xi1\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>949,column_high=>954);
print "Node 51, xi2\n$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1570,column_high=>1575);
print "Node 51, xi3\n$sub_matrix\n";
$var_8c_51=$var_1->evaluate_derivative(independent=>[$var_1->input_nodal_values(node=>$heart->get_node(name=>51)),$input_1]);
print "$var_8c_51\n";
$var_8d=$var_1->evaluate_derivative(independent=>[$input_1,$input_1b]);
$matrix=$var_8d->matrix($input_1);
print "$matrix\n";
$matrix=$var_8d->matrix($input_1b);
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>172,column_high=>186);
print "Node 28\n$sub_matrix\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>148,column_high=>153);
print "Node 24\n$sub_matrix\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>508,column_high=>513);
print "Node 81\n$sub_matrix\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>73,column_high=>87);
print "Node 13\n$sub_matrix\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>49,column_high=>54);
print "Node 9\n$sub_matrix\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>328,column_high=>333);
print "Node 51\n$sub_matrix\n";
$matrix=$var_8d->matrix($input_1,$input_1b);
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>514,column_high=>558);
print "Node 28\n$sub_matrix\n";
$var_8d_28=$var_1->evaluate_derivative(independent=>[$input_1,$var_1->input_nodal_values(node=>$heart->get_node(name=>28))]);
print "$var_8d_28\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>442,column_high=>459);
print "Node 24\n$sub_matrix\n";
$var_8d_24=$var_1->evaluate_derivative(independent=>[$input_1,$var_1->input_nodal_values(node=>$heart->get_node(name=>24))]);
print "$var_8d_24\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>1522,column_high=>1539);
print "Node 81\n$sub_matrix\n";
$var_8d_81=$var_1->evaluate_derivative(independent=>[$input_1,$var_1->input_nodal_values(node=>$heart->get_node(name=>81))]);
print "$var_8d_81\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>217,column_high=>261);
print "Node 13\n$sub_matrix\n";
$var_8d_13=$var_1->evaluate_derivative(independent=>[$input_1,$var_1->input_nodal_values(node=>$heart->get_node(name=>13))]);
print "$var_8d_13\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>145,column_high=>162);
print "Node 9\n$sub_matrix\n";
$var_8d_9=$var_1->evaluate_derivative(independent=>[$input_1,$var_1->input_nodal_values(node=>$heart->get_node(name=>9))]);
print "$var_8d_9\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>982,column_high=>999);
print "Node 51\n$sub_matrix\n";
$var_8d_51=$var_1->evaluate_derivative(independent=>[$input_1,$var_1->input_nodal_values(node=>$heart->get_node(name=>51))]);
print "$var_8d_51\n";
print "\n";
# check evaluating fourth derivative
$var_8e=$var_1->evaluate_derivative(independent=>[$input_1,$input_1,$input_1,$input_1]);
print "???DB.  Haven't checked the values for component 1 apart from \"diagonal\" -\n";
print "???  d^4/dxi1^4 (column 1), d^4/dxi2^4 (column 41), d^4/dxi3^4 (column 81) -\n";
print "???  which should be 0\n";
print "???DB.  If the entry is (i,j,k,l) where 1<=i,j,k,l<=3 then the column number\n";
print "???  for the entry is (((i-1)*3+(j-1))*3+(k-1))*3+l\n";
print "$var_8e\n";
print "\n";
# check creating composite input
$input_2=new Cmiss::Variable_new_input::Composite($input_1,$var_3->input_xi(2));
# check evaluating first derivative wrt composite
$var_9=$var_1->evaluate_derivative(independent=>[$input_2]);
print "$var_9\n";
$var_10=$var_1->evaluate_derivative(independent=>[new Cmiss::Variable_new_input::Composite($var_3->input_xi(2,3),$input_1)]);
print "$var_10\n";
print "\n";
## check getting and setting input values
$var_11=$var_1->get_input_value($input_1);
print "$var_11\n";
$var_12=$var_1->get_input_value($var_1->input_nodal_values(node=>$heart->get_node(name=>"13")));
print "$var_12\n";
$return_code=$var_1->set_input_value($var_1->input_nodal_values(node=>$heart->get_node(name=>"13")),new Cmiss::Variable_new::Vector(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15));
$var_13=$var_1->get_input_value($var_1->input_nodal_values(node=>$heart->get_node(name=>"13")));
print "$return_code $var_13\n";
$var_12->set_input_value($var_12->input_values(5),new Cmiss::Variable_new::Vector(-1));
$return_code=$var_1->set_input_value($var_1->input_nodal_values(node=>$heart->get_node(name=>"13")),$var_12);
$var_14=$var_1->get_input_value($var_1->input_nodal_values(node=>$heart->get_node(name=>"13")));
print "$return_code $var_14\n";
$return_code=$var_1->set_input_value($var_1->input_nodal_values(node=>$heart->get_node(name=>"13"),type=>'value'),new Cmiss::Variable_new::Vector(1,2,3,4,5,6,7,8,9,10,11,12));
$var_15=$var_1->get_input_value($var_1->input_nodal_values(node=>$heart->get_node(name=>"13")));
print "$return_code $var_15\n";
