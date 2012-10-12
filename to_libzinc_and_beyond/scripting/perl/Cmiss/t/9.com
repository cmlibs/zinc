if (!defined $path)
{
  $path = ".";
}
use Cmiss::Value::Element_xi;
use Cmiss::Value::FE_value_vector;
use Cmiss::Value::Derivative_matrix;
use Cmiss::Variable::Derivative;
use Cmiss::Variable::Element_xi;
use Cmiss::Variable::Finite_element;
use Cmiss::Region;
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
$coordinates=new Cmiss::Variable::Finite_element(region=>$heart,name=>'coordinates');
$element_xi=new Cmiss::Value::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$element_xi_var=new Cmiss::Variable::Element_xi(name=>'element_xi_var');
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$element_xi_var,$element_xi_var,$element_xi_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
Cmiss::Value::Matrix->set_string_convert_max_columns(100);
print "???DB.  Haven't checked the values for component 1 apart from \"diagonal\" -\n";
print "???  d^4/dxi1^4 (column 1), d^4/dxi2^4 (column 41), d^4/dxi3^4 (column 81) -\n";
print "???  which should be 0\n";
print "???DB.  If the entry is (i,j,k,l) where 1<=i,j,k,l<=3 then the column number\n";
print "???  for the entry is (((i-1)*3+(j-1))*3+(k-1))*3+l\n";
print "$result\n";
$element_xi=0;
$coordinates=0;
$heart=0;
$root=0;
